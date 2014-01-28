#pragma once

namespace swarm
{
  class Entity
  {
    public:
    Entity();
    
    Vector2f _acc;
    Vector2f _vel;
    Vector2f _pos;
  };
}