#pragma once

// Shared between client and server

namespace swarm
{
  struct MonsterState
  {
    PhysicsState _curState;
    PhysicsState _prevState;
  };

  struct PlayerState
  {
    PhysicsState _curState;
    PhysicsState _prevState;
  };

}