#include "renderer.hpp"
#include "world.hpp"
#include "utils.hpp"
#include "player.hpp"

using namespace swarm;

void Renderer::DrawWorld(RenderWindow& target, const World& world)
{
  View view = target.getView();
  view.setCenter(world._players[0]->_pos);
  target.setView(view);
  target.draw(world._level->_sprite);

  for (const auto& player : world._players)
  {
    CircleShape circle;
    circle.setFillColor(Color::Green);
    Vector2f p(VectorCast<float>(player->_pos));
    p.x -= 5;
    p.y -= 5;
    circle.setPosition(p);
    circle.setRadius(5);
    target.draw(circle);
  }
}
