#pragma once

namespace swarm
{
  struct PhysicsState
  {
    PhysicsState() : _acc(0,0), _vel(0,0), _pos(0,0) {}
    Vector2f _acc;
    Vector2f _vel;
    Vector2f _pos;
  };
}
