#pragma once
#include "utils.hpp"
#include "level.hpp"

namespace swarm
{
  class Player;
  class Level;

  class World
  {
    public:
    void AddPlayer();

    static World* Create();

    vector<intrusive_ptr<Player>> _players;
    unique_ptr<Level> _level;

    private:
    World();
    
    DISALLOW_COPY_AND_ASSIGN(World);
    
  };
}