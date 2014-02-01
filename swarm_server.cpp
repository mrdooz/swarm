#include "swarm_server.hpp"
#include "utils.hpp"
#include "entity.hpp"
#include "protocol/game.pb.h"
#include "error.hpp"

using namespace swarm;

//-----------------------------------------------------------------------------
Server::Server()
  : _serverThread(nullptr)
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
  vector<u8> buf(16*1024);
  for (TcpSocket* s : _connectedClients)
  {
    size_t recievedSize = 0;
    Socket::Status status = s->receive(buf.data(), buf.size(), recievedSize);
    if (status == Socket::Done)
    {
      game::PlayerMessage playerMsg;
      if (playerMsg.ParseFromArray(buf.data(), recievedSize))
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
            ApplyAttractor(pos, playerMsg.click().click_size());
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
  while (true)
  {
    // Check for connected players
    Socket::Status status = listener.accept(*socket);
    if (status == Socket::Done)
    {
      LOG_INFO("Player connected"
        << LogKeyValue("addr", socket->getRemoteAddress().toString())
        << LogKeyValue("port", socket->getRemotePort()));
      _connectedClients.push_back(socket);
      u16 port = socket->getRemotePort();
      u32 addr = socket->getRemoteAddress().toInteger();

      // save the address to id mapping
      auto key = make_pair(addr, port);
      int id;
      auto it = _addrToId.find(key);
      if (it != _addrToId.end())
      {
        id = it->second;
      }
      else
      {
        id = _addrToId.size();
        _addrToId[key] = id;
      }

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

    // update 10 times/sec
    if (delta.asMilliseconds() > 100)
    {
      lastUpdate = end;
      Update(milliseconds(delta.asMilliseconds()));
      SendPlayerState();
    }

    HandleClientMessages();
  }

  delete socket;
}

//-----------------------------------------------------------------------------
bool Server::Init()
{
  if (!_world._level->Load("data/pacman.png"))
    return false;
  _world.AddMonsters();

  _serverThread = new thread(bind(&Server::ThreadProc, this));
  return true;
}

//-----------------------------------------------------------------------------
bool Server::Close()
{
  if (_serverThread)
    _serverThread->join();

  return true;
}

//----------------------------------------------------------------------------------
void Server::UpdateEntity(Entity& entity, float dt)
{
  // Velocity Verlet integration
  float friction = 0.99f;
  float scale = _world._level->_scale;
  Vector2f oldVel = entity._vel;
  Vector2f p = entity._pos;
  Vector2f v = friction * (entity._vel + entity._acc * dt);

  Vector2f newPos = entity._pos + (oldVel + entity._vel) * 0.5f * dt;

  u8 b;
  // check horizontal collisions
  if (!(_world._level->PosToBackground(1/scale * (p + dt * Vector2f(v.x, 0)), &b) && b == 0))
  {
    newPos.x = p.x;
    v.x = -v.x;
  }

  // check vertical
  if (!(_world._level->PosToBackground(1/scale * (p + dt * Vector2f(0, v.y)), &b) && b == 0))
  {
    newPos.y = p.y;
    v.y = -v.y;
  }

  entity._vel = v;
  entity._pos = newPos;
}

//----------------------------------------------------------------------------------
void Server::SendToClients(const string& str)
{
  for (auto it = _connectedClients.begin(); it != _connectedClients.end(); )
  {
    TcpSocket* socket = *it;
    Socket::Status status = socket->send(str.data(), str.size());
    if (status != Socket::Done)
    {
      // unable to send, so remove the client
      delete socket;
      it = _connectedClients.erase(it);
    }
    else
    {
      LOG_INFO("Send to client" << LogKeyValue("num_bytes", str.size()));
      ++it;
    }
  }
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
    game::Position* pos = player->mutable_pos();
    pos->set_x(data.pos.x);
    pos->set_y(data.pos.y);
  }

  string str;
  if (!msg.SerializeToString(&str))
  {
    LOG_WARN("Unable to serialize state");
    return;
  }

  SendToClients(str);
}

//----------------------------------------------------------------------------------
void Server::ApplyAttractor(const Vector2f& pos, float radius)
{
  for (auto* monster : _world._monsters)
  {
    // Set acceleration for any mobs inside the click radius
    if (radius > 0)
    {
      Vector2f dir = pos - monster->_pos;
      float d = Length(dir);
      if (d < radius)
      {
        monster->_acc -= 10.0f * Normalize(dir);
      }
    }
  }
}

//----------------------------------------------------------------------------------
void Server::Update(const time_duration& delta)
{
  game::ServerMessage msg;
  msg.set_type(game::ServerMessage_Type_SWARM_STATE);
  game::SwarmState& state = *msg.mutable_swarm_state();

  float dt = delta.total_milliseconds() / 1000.0f;
  for (Monster* monster : _world._monsters)
  {
    UpdateEntity(*monster, dt);

    // add monster to state
    game::Monster* m = state.add_monster();
    auto* p = m->mutable_pos();
    p->set_x(monster->_pos.x);
    p->set_y(monster->_pos.y);
    m->set_size(monster->_size);
  }

  // Send state to all connected clients
  string str;
  if (!msg.SerializeToString(&str))
  {
    LOG_WARN("Unable to serialize state");
    return;
  }

  SendToClients(str);
}
