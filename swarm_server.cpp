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
      _connectedClients.push_back(socket);
      u16 port = socket->getLocalPort();
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
    }

    vector<u8> buf(16*1024);
    for (TcpSocket* s : _connectedClients)
    {
      size_t recievedSize = 0;
      status = s->receive(buf.data(), buf.size(), recievedSize);
      if (status == Socket::Done)
      {
        game::PlayerMessage playerMsg;
        if (playerMsg.ParseFromArray(buf.data(), recievedSize))
        {
          switch (playerMsg.type())
          {
          case game::PlayerMessage_Type_PLAYER_POS:
            break;

          case game::PlayerMessage_Type_PLAYER_CLICK:
            break;
          }
        }

      }
    }
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
void Server::UpdateMonsters()
{
  for (auto* monster : _world._monsters)
  {
    for (const MonsterAttractor& a : _monsterAttractors)
    {
      float r = a.radius;
      Vector2f pos = a.pos;
      // Set acceleration for any mobs inside the click radius
      if (r > 0)
      {
        Vector2f dir = pos - monster->_pos;
        float d = Length(dir);
        if (d < r)
        {
          monster->_acc -= 10.0f * Normalize(dir);
        }
      }

    }
  }
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
void Server::Update(const time_duration& delta)
{
  UpdateMonsters();

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
      ++it;
    }
  }
}
