#pragma once
#include "entity.hpp"

namespace swarm
{
  class Monster : public Entity
  {
  public:
    Monster(const Vector2f& pos, float size);
    Vector2f _target;
    float _size;
    int _swarmId;
  };
}
