#include "world.hpp"
#include "player.hpp"
#include "level.hpp"

using namespace swarm;

//-----------------------------------------------------------------------------
void World::AddPlayer()
{
  auto player(PlayerPtr(new Player));

  float scale = _level->_scale;
  // find a free spot to put the player in
  while (true)
  {
    int x = rand() % _level->_width/4;
    int y = rand() % _level->_height/4;
    if (_level->_background[y*_level->_width+x] == 0)
    {
      player->_pos = player->_oldPos = Vector2f(x*scale, y*scale);
      break;
    }
  }
  _players.push_back(player);
}

//-----------------------------------------------------------------------------
World::World()
  : _level(new Level())
{
}

//-----------------------------------------------------------------------------
World* World::Create()
{
  return new World();
}