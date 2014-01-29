#pragma once
#include "utils.hpp"
#include "level.hpp"
#include "monster.hpp"

namespace swarm
{
  class Player;
  class Level;
  class Monster;

  class World
  {
  public:
    World();
    ~World();

    void AddPlayer();
    void AddMonsters();

    vector<Monster* > _monsters;
    vector<Player* > _players;
    Level* _level;

  private:
    DISALLOW_COPY_AND_ASSIGN(World);
  };
}