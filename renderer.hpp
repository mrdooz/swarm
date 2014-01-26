#pragma once

namespace swarm
{
  class World;
  
  class Renderer
  {
    public:
    void DrawWorld(RenderWindow& target, const World& world);
  };
}
