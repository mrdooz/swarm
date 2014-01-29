#include "swarm_server.hpp"
#include "utils.hpp"
#include "entity.hpp"

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
      socket = new TcpSocket();
    }

    Time end = clock.getElapsedTime();
    Time delta = end - lastUpdate;
    lastUpdate = end;
    Update(milliseconds(delta.asMilliseconds()));
  }

  delete socket;

}

//-----------------------------------------------------------------------------
bool Server::Init()
{
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

  float dt = delta.total_milliseconds() / 1000.0f;
  for (auto monster : _world._monsters)
  {
    UpdateEntity(*monster, dt);
  }
}
