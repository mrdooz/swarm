#include "swarm_server.hpp"
#include "utils.hpp"
#include "entity.hpp"
#include "protocol/game.pb.h"
#include "error.hpp"
#include "protocol.hpp"

using namespace swarm;

//-----------------------------------------------------------------------------
Server::Server()
  : _serverThread(nullptr)
  , _done(false)
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
  for (TcpSocket* s : _connectedClients)
  {
    size_t recievedSize = 0;
    Socket::Status status = s->receive(_networkBuffer, sizeof(_networkBuffer), recievedSize);
    if (status == Socket::Done)
    {
      game::PlayerMessage playerMsg;
      if (playerMsg.ParseFromArray(_networkBuffer, recievedSize))
      {
        auto it = _addrToId.find(make_pair(s->getRemoteAddress().toInteger(), s->getRemotePort()));
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
void Server::ThreadProc()
{
  TcpListener listener;
  listener.setBlocking(false);
  listener.listen(50000);

  TcpSocket *socket = new TcpSocket();
  socket->setBlocking(false);

  Clock clock;
  clock.restart();
  Time lastUpdate = clock.getElapsedTime();
  Time lastSend = lastUpdate;
  double timestep = 1/50.0;
  double accumulator = 0;

  while (!_done)
  {
    // Check for connected players
    Socket::Status status = listener.accept(*socket);
    if (status == Socket::Done)
    {
      _connectedClients.push_back(socket);
      u16 port = socket->getRemotePort();
      u32 addr = socket->getRemoteAddress().toInteger();

      // save the address to id mapping
      auto key = make_pair(addr, port);
      int id;
      auto it = _addrToId.find(key);
      bool newPlayer = false;
      if (it != _addrToId.end())
      {
        id = it->second;
      }
      else
      {
        newPlayer = true;
        id = _addrToId.size();
        _addrToId[key] = id;
      }

      LOG_INFO((newPlayer ? "New player connected" : "Existing player connected")
        << LogKeyValue("addr", socket->getRemoteAddress().toString())
        << LogKeyValue("port", socket->getRemotePort()));

      // send the ack to the client
      game::ServerMessage msg;
      msg.set_type(game::ServerMessage_Type_CONNECTION_ACK);
      game::ConnectionAck& ack = *msg.mutable_connection_ack();
      ack.set_player_id(id);
      string str;
      if (msg.SerializeToString(&str))
      {
        socket->send(str.data(), str.size());
      }

      socket = new TcpSocket();
      socket->setBlocking(false);
    }

    Time end = clock.getElapsedTime();
    Time delta = end - lastUpdate;
    lastUpdate = end;

    // apply attractors..
    for (MonsterState& state : _monsterState)
    {
      state._curState._acc = Vector2f(0,0);
    }

    for (const MonsterAttractor& a : _attractors)
    {
      ApplyAttractor(a.pos, a.radius);
    }

    accumulator += delta.asMicroseconds() / 1e6;

    if (accumulator >= timestep)
      _attractors.clear();

    while (accumulator >= timestep)
    {
      for (MonsterState& state : _monsterState)
      {
        state._prevState = state._curState;
        UpdateState(state._curState, (float)timestep);
      }
      accumulator -= timestep;
    }

    float alpha = (double)accumulator / timestep;

    // send state 10 times/sec
    Time sendDelta = end - lastSend;
    if (sendDelta.asMilliseconds() > 100)
    {
      game::ServerMessage msg;
      msg.set_type(game::ServerMessage_Type_SWARM_STATE);
      game::SwarmState& swarmState = *msg.mutable_swarm_state();

      for (size_t i = 0; i < _monsterState.size(); ++i)
      {
        MonsterState& state = _monsterState[i];
        MonsterData& data = _monsterData[i];

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

      // Send state to all connected clients
      vector<char> buf;
      if (PackMessage(buf, msg))
      {
        SendToClients(buf);
      }

      SendPlayerState();
      lastSend = end;
    }

    HandleClientMessages();
  }

  delete socket;
}

//-----------------------------------------------------------------------------
bool Server::Init()
{
  if (!_level.Load("data/pacman.png"))
    return false;

  float scale = _level._scale;
  for (size_t i = 0; i < 20; ++i)
  {
    while (true)
    {
      int x = rand() % _level._width;
      int y = rand() % _level._height;
      float s = 5 + rand() % 5;
      if (_level._background[y*_level._width+x] == 0)
      {
        Vector2f pos(scale * Vector2f(x, y));

        _monsterState.push_back(MonsterState());
        MonsterState& state = _monsterState.back();
        state._curState._pos = pos;
        state._prevState._pos = pos;

        _monsterData.push_back(MonsterData());
        MonsterData& data  = _monsterData.back();
        data._size = s;
        break;
      }
    }
  }

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
      LOG_INFO("Send to client" << LogKeyValue("num_bytes", buf.size()));
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
    ToProtocol(player->mutable_pos(), data.pos);
  }

  vector<char> buf;
  if (PackMessage(buf, msg))
  {
    SendToClients(buf);
  }
}

//----------------------------------------------------------------------------------
void Server::ApplyAttractor(const Vector2f& pos, float radius)
{
  for (MonsterState& state : _monsterState)
  {
    // Set acceleration for any mobs inside the click radius
    if (radius > 0)
    {
      Vector2f dir = pos - state._curState._pos;
      float d = Length(dir);
      if (d < radius)
      {
        state._curState._acc += 250.0f * Normalize(dir);
      }
    }
  }
}
