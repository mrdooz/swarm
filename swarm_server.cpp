#include "swarm_server.hpp"
#include "error.hpp"
#include "protocol.hpp"

using namespace swarm;

namespace
{
  //-----------------------------------------------------------------------------
  pair<u32, u16> KeyFromSocket(const TcpSocket* socket)
  {
    return make_pair(socket->getRemoteAddress().toInteger(), socket->getRemotePort());
  }
}

//-----------------------------------------------------------------------------
Server::Server()
  : _serverThread(nullptr)
  , _done(false)
  , _nextPlayerId(1)
  , _gameStarted(false)
{
}

//-----------------------------------------------------------------------------
Server::~Server()
{
  delete exch_null(_serverThread);
  SeqDelete(&_connectedClients);
}

//-----------------------------------------------------------------------------
void Server::HandleClientMessages()
{
  for (TcpSocket* socket : _connectedClients)
  {
    size_t receivedBytes = 0;
    Socket::Status status = socket->receive(_networkBuffer, sizeof(_networkBuffer), receivedBytes);
    if (status == Socket::Done)
    {
      game::PlayerMessage playerMsg;
      if (playerMsg.ParseFromArray(_networkBuffer, receivedBytes))
      {
        auto it = _addrToId.find(KeyFromSocket(socket));
        if (it == _addrToId.end())
        {
          LOG_WARN("Unknown client");
          continue;
        }

        int id = it->second;
        switch (playerMsg.type())
        {
        case game::PlayerMessage_Type_PLAYER_POS:
          _playerData[id].pos = Vector2f(playerMsg.pos().x(), playerMsg.pos().y());
          break;

        case game::PlayerMessage_Type_PLAYER_CLICK:
          {
            Vector2f pos(playerMsg.click().click_pos().x(), playerMsg.click().click_pos().y());
            _attractors.push_back(MonsterAttractor(pos, playerMsg.click().click_size()));
          }
          break;
        }
      }

    }
  }

}

//-----------------------------------------------------------------------------
void Server::PlayerAdded(TcpSocket* socket)
{
  int initialHealth = _config.initial_health();

  _connectedClients.push_back(socket);

  // save the address to id mapping
  auto key = KeyFromSocket(socket);
  auto it = _addrToId.find(key);
  bool newPlayer = it == _addrToId.end();
  u32 id = newPlayer ? _nextPlayerId++ : it->second;
  _addrToId[key] = id;

  LOG_INFO((newPlayer ? "New player connected" : "Existing player connected")
      << LogKeyValue("addr", socket->getRemoteAddress().toString())
      << LogKeyValue("port", socket->getRemotePort())
      << LogKeyValue("id", id));

  PlayerData& player = _playerData[id];
  player.id = id;
  player.pos = _level.GetPlayerPos();
  player.health = initialHealth;

  if (!_gameStarted && _connectedClients.size() < _config.min_players())
    return;

  _gameStarted = true;

  game::ServerMessage serverMsg;
  serverMsg.set_type(game::ServerMessage_Type_GAME_STARTED);
  game::GameStarted& msg = *serverMsg.mutable_game_started();
  msg.set_map_name(_config.map_name());

  // add initial player state
  game::PlayerState* playerState = msg.mutable_player_state();
  for (auto it = _playerData.begin(); it != _playerData.end(); ++it)
  {
    const PlayerData& data = it->second;
    game::Player* player = playerState->add_player();
    player->set_id(it->first);
    player->set_health(data.health);
    ToProtocol(player->mutable_pos(), data.pos);
  }

  // add initial swarm state
  game::SwarmState* swarmState = msg.mutable_swarm_state();

  for (size_t i = 0; i < _monsterData.size(); ++i)
  {
    MonsterData& data = _monsterData[i];
    MonsterState& state = data._state;

    game::Monster* m = swarmState->add_monster();
    ToProtocol(m->mutable_acc(), state._curState._acc);
    ToProtocol(m->mutable_vel(), state._curState._vel);
    ToProtocol(m->mutable_pos(), state._curState._pos);
    m->set_size(data._size);
  }

  // send game started to each player who hasn't already got it
  for (TcpSocket* socket : _connectedClients)
  {
    auto key = KeyFromSocket(socket);
    u32 id = _addrToId[key];
    PlayerData& player = _playerData[id];
    if (!player.sentStartGame)
    {
      player.sentStartGame = true;
      msg.set_player_id(id);
      msg.set_health(initialHealth);

      vector<char> buf;
      if (PackMessage(buf, serverMsg))
      {
        SendToClient(buf, socket);
      }
    }
  }
}

//-----------------------------------------------------------------------------
void Server::ThreadProc()
{
  TcpSocket *socket = new TcpSocket();
  socket->setBlocking(false);

  Clock clock;
  clock.restart();
  Time lastUpdate = clock.getElapsedTime();
  Time lastSend = lastUpdate;
  Time lastCollisionCheck = lastUpdate;
  double timestep = 1/50.0;
  double accumulator = 0;

  while (!_done)
  {
    // Check for connected players
    Socket::Status status = _listener.accept(*socket);
    if (status == Socket::Done)
    {
      PlayerAdded(socket);
      socket = new TcpSocket();
      socket->setBlocking(false);
    }

    if (_gameStarted)
    {
      Time end = clock.getElapsedTime();
      Time delta = end - lastUpdate;
      lastUpdate = end;

      for (MonsterData& data : _monsterData)
      {
        data._state._curState._acc = Vector2f(0, 0);
      }

      // apply attractors..
      if (!_attractors.empty())
      {
        for (const MonsterAttractor& a : _attractors)
        {
          ApplyAttractor(a.pos, a.radius);
        }
      }

      accumulator += delta.asMicroseconds() / 1e6;

      while (accumulator >= timestep)
      {
        for (MonsterData& data : _monsterData)
        {
          MonsterState& state = data._state;
          state._prevState = state._curState;
          UpdateState(state._curState, (float)timestep);
        }
        accumulator -= timestep;
        _attractors.clear();
      }

      Time collisionDelta = end - lastCollisionCheck;
      if (collisionDelta.asMilliseconds() > 50)
      {
        HandleCollisions();
        lastCollisionCheck = end;
      }

      // send state 10 times/sec
      Time sendDelta = end - lastSend;
      if (sendDelta.asMilliseconds() > 100)
      {
        float alpha = (float)(accumulator / timestep);
        SendMonsterState(alpha);
        SendPlayerState();
        lastSend = end;
      }

      HandleClientMessages();

    }

  }

  delete socket;
}

//-----------------------------------------------------------------------------
void Server::AddMonster(const Vector2f& pos, float size)
{
  _monsterData.push_back(MonsterData());
  MonsterData& data = _monsterData.back();
  MonsterState& state = data._state;
  state._curState._pos = pos;
  state._prevState._pos = pos;
  data._size = size;
}


//-----------------------------------------------------------------------------
bool Server::InitLevel()
{
  if (!_level.Load(_config.map_name().c_str()))
    return false;

  float scale = _level._scale;
  // create the swarms

  for (size_t i = 0; i < _config.num_swarms(); ++i)
  {
    // find swarm center pos
    int centerX, centerY;
    while (true)
    {
      centerX = rand() % _level._width;
      centerY = rand() % _level._height;
      u8 val = _level._background[centerY*_level._width + centerX];
      if (val == 0)
        break;
    }

    int swarmRadius = 20;

    // add the monsters around the swarm
    for (size_t j = 0; j < _config.monsters_per_swarm(); ++j)
    {
      float s = 3 + 3 * rand() / (float)RAND_MAX;
      while (true)
      {
        float x = centerX - swarmRadius / 2 + swarmRadius / 2.0f * (rand() / (float)RAND_MAX);
        float y = centerY - swarmRadius / 2 + swarmRadius / 2.0f * (rand() / (float)RAND_MAX);

        x = (int)Clamp<float>(x, 0, _level._width - 1);
        y = (int)Clamp<float>(y, 0, _level._height - 1);

        u8 val = _level._background[y*_level._width + x];
        if (val == 0)
        {
          AddMonster(scale * Vector2f(x, y), s);
          break;
        }
      }
    }


  }

  return true;
}

//-----------------------------------------------------------------------------
bool Server::Init(const char* configFile)
{
  if (!ProtobufFromFile(configFile, &_config))
  {
    LOG_WARN("Unable to load config file" << LogKeyValue("name", configFile));
    return false;
  }

  if (!InitLevel())
  {
    LOG_WARN("Error initialzing level");
    return false;
  }

  // Start listening on the first available port
  _listener.setBlocking(false);
  _port = 50000;
  Socket::Status status = _listener.listen(_port);
  while (status != Socket::Done)
  {
    (_port)++;
    status = _listener.listen(_port);
  }
  printf("Server listening on port: %d\n", _port);

  _serverThread = new thread(bind(&Server::ThreadProc, this));
  return true;
}

//-----------------------------------------------------------------------------
bool Server::Close()
{
  _done = true;

  if (_serverThread)
    _serverThread->join();

  return true;
}

//----------------------------------------------------------------------------------
void Server::UpdateState(PhysicsState& state, float dt)
{
  // Velocity Verlet integration
  float friction = 0.999f;
  float scale = _level._scale;
  Vector2f oldVel = state._vel;
  Vector2f p = state._pos;
  Vector2f v = friction * (state._vel + state._acc * dt);

  Vector2f newPos = state._pos + (oldVel + state._vel) * 0.5f * dt;

  u8 b;
  // check horizontal collisions
  if (!(_level.PosToBackground(1/scale * (p + dt * Vector2f(v.x, 0)), &b) && b == 0))
  {
    newPos.x = p.x;
    v.x = -v.x;
  }

  // check vertical
  if (!(_level.PosToBackground(1/scale * (p + dt * Vector2f(0, v.y)), &b) && b == 0))
  {
    newPos.y = p.y;
    v.y = -v.y;
  }

  state._vel = v;
  state._pos = newPos;
}

//----------------------------------------------------------------------------------
bool Server::SendToClient(const vector<char>& buf, TcpSocket* socket)
{
  Socket::Status status = socket->send(buf.data(), buf.size());
  if (status == Socket::Disconnected)
  {
    // unable to send, so remove the client
    auto idIt = _addrToId.find(
        make_pair(socket->getRemoteAddress().toInteger(), socket->getRemotePort()));
    if (idIt != _addrToId.end())
    {
      int id = idIt->second;
      _playerData.erase(id);
    }

    delete socket;
    auto it = find(_connectedClients.begin(), _connectedClients.end(), socket);
    if (it != _connectedClients.end())
      _connectedClients.erase(it);
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------------
void Server::SendToClients(const vector<char>& buf)
{
  for (auto it = _connectedClients.begin(); it != _connectedClients.end(); )
  {
    TcpSocket* socket = *it;
    Socket::Status status = socket->send(buf.data(), buf.size());
    if (status == Socket::Disconnected)
    {
      // unable to send, so remove the client
      auto idIt = _addrToId.find(
          make_pair(socket->getRemoteAddress().toInteger(), socket->getRemotePort()));
      if (idIt != _addrToId.end())
      {
        int id = idIt->second;
        _playerData.erase(id);
      }

      delete socket;
      it = _connectedClients.erase(it);
    }
    else
    {
      //LOG_INFO("Send to client" << LogKeyValue("num_bytes", buf.size()));
      ++it;
    }
  }
}

//----------------------------------------------------------------------------------
template <typename T>
bool Server::PackMessage(vector<char>& buf, const T& msg)
{
  buf.resize(msg.ByteSize() + sizeof(u32));
  *(u32*)buf.data() = htonl(msg.ByteSize());
  if (!msg.SerializeToArray(&buf[4], buf.size() - sizeof(u32)))
  {
    LOG_WARN("Unable to serialize state");
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------
template <typename T>
bool Server::SendMessageToClients(const T& msg)
{
  // Send state to all connected clients
  vector<char> buf;
  if (PackMessage(buf, msg))
  {
    SendToClients(buf);
    return true;
  }
  return false;
}


//----------------------------------------------------------------------------------
void Server::SendMonsterState(float alpha)
{
  game::ServerMessage msg;
  msg.set_type(game::ServerMessage_Type_SWARM_STATE);
  game::SwarmState& swarmState = *msg.mutable_swarm_state();

  for (size_t i = 0; i < _monsterData.size(); ++i)
  {
    MonsterData& data = _monsterData[i];
    MonsterState& state = data._state;

    // add monster to state
    game::Monster* m = swarmState.add_monster();
    Vector2f acc = lerp(state._prevState._acc, state._curState._acc, alpha);
    Vector2f vel = lerp(state._prevState._vel, state._curState._vel, alpha);
    Vector2f pos = lerp(state._prevState._pos, state._curState._pos, alpha);

    ToProtocol(m->mutable_acc(), acc);
    ToProtocol(m->mutable_vel(), vel);
    ToProtocol(m->mutable_pos(), pos);
    m->set_size(data._size);
  }

  SendMessageToClients(msg);
}


//----------------------------------------------------------------------------------
void Server::SendPlayerState()
{
  game::ServerMessage msg;
  msg.set_type(game::ServerMessage_Type_PLAYER_STATE);
  game::PlayerState* state = msg.mutable_player_state();

  for (auto it = _playerData.begin(); it != _playerData.end(); ++it)
  {
    const PlayerData& data = it->second;
    game::Player* player = state->add_player();
    player->set_id(it->first);
    player->set_health(data.health);
    ToProtocol(player->mutable_pos(), data.pos);
  }

  SendMessageToClients(msg);
}

//----------------------------------------------------------------------------------
void Server::ApplyAttractor(const Vector2f& pos, float radius)
{
  for (MonsterData& data : _monsterData)
  {
    MonsterState& state = data._state;
    // Set acceleration for any mobs inside the click radius
    if (radius > 0)
    {
      Vector2f dir = pos - state._curState._pos;
      float d = Length(dir);
      if (d < radius)
      {
        // f = m * a, a = f / m
        state._curState._acc += 1000.0f / data._size * Normalize(dir);
      }
    }
  }
}

//----------------------------------------------------------------------------------
void Server::SendPlayerDied(u32 id)
{
  game::ServerMessage msg;
  msg.set_type(game::ServerMessage_Type_PLAYER_DIED);
  game::PlayerDied* d = msg.mutable_player_died();
  d->set_player_id(id);
  SendMessageToClients(msg);
}


//----------------------------------------------------------------------------------
void Server::HandleCollisions()
{
  game::ServerMessage msg;
  msg.set_type(game::ServerMessage_Type_MONSTER_DIED);
  game::MonsterDied* m = msg.mutable_monster_died();

  for (auto it = _monsterData.begin(); it != _monsterData.end(); )
  {
    MonsterData& data = *it;
    MonsterState& state = data._state;
    const Vector2f& monsterPos = state._curState._pos;

    bool deleteMonster = false;
    for (auto& kv : _playerData)
    {
      PlayerData& player = kv.second;

      Vector2f dir = player.pos - monsterPos;
      float d = Length(dir);
      if (d < data._size)
      {
        // Check if collision leads to player death
        if (0 == --player.health)
        {
          SendPlayerDied(player.id);
          player.alive = false;
        }
        deleteMonster = true;
        ToProtocol(m->mutable_pos()->Add(), monsterPos);
      }
    }

    if (deleteMonster)
    {
      it = _monsterData.erase(it);
    }
    else
    {
      ++it;
    }
  }

  // Check for monster collisions
  if (m->pos_size())
  {
    SendMessageToClients(msg);
  }

  int numPlayersAlive = 0;
  u32 firstLiving = ~0;
  for (auto& kv : _playerData)
  {
    PlayerData& player = kv.second;
    if (player.alive)
    {
      numPlayersAlive++;
      firstLiving = firstLiving == ~0 ? player.id : firstLiving;
    }
  }

  if (_connectedClients.size() == 1 && numPlayersAlive == 0 ||
      _connectedClients.size() > 1 && numPlayersAlive == 1)
  {
    game::ServerMessage msg;
    msg.set_type(game::ServerMessage_Type_GAME_ENDED);
    game::GameEnded* e = msg.mutable_game_ended();
    e->set_winner_id(firstLiving);
    SendMessageToClients(msg);
  }

}

//----------------------------------------------------------------------------------
void Server::ResetGame()
{
  // todo: disconnect everyone, reset state
}

