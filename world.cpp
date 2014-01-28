#include "world.hpp"
#include "player.hpp"
#include "level.hpp"
#include "monster.hpp"

using namespace swarm;

//-----------------------------------------------------------------------------
World::World()
  : _level(new Level())
{
}

//-----------------------------------------------------------------------------
World::~World()
{
  delete exch_null(_level);
  SeqDelete(&_players);
  SeqDelete(&_monsters);
}

//-----------------------------------------------------------------------------
World* World::Create()
{
  return new World();
}

//-----------------------------------------------------------------------------
void World::AddMonsters()
{
  float scale = _level->_scale;
  for (size_t i = 0; i < 10; ++i)
  {
    while (true)
    {
      int x = rand() % _level->_width;
      int y = rand() % _level->_height;
      float s = 5 + rand() % 5;
      if (_level->_background[y*_level->_width+x] == 0)
      {
        _monsters.push_back(new Monster(scale * Vector2f(x, y), s));
        break;
      }
    }
  }
}

//-----------------------------------------------------------------------------
void World::AddPlayer()
{
  auto player(new Player);

  float scale = _level->_scale;
  // find a free spot to put the player in
  while (true)
  {
    int x = rand() % _level->_width/4;
    int y = rand() % _level->_height/4;
    if (_level->_background[y*_level->_width+x] == 0)
    {
      player->_pos = scale * Vector2f(x, y);
      break;
    }
  }
  _players.push_back(player);
}
