#include "protocol.hpp"

namespace swarm
{
  //-----------------------------------------------------------------------------
  void ToProtocol(game::Vector2* lhs, const Vector2f& rhs)
  {
    lhs->set_x(rhs.x);
    lhs->set_y(rhs.y);
  }

  //-----------------------------------------------------------------------------
  void FromProtocol(Vector2f* lhs, const game::Vector2& rhs)
  {
    lhs->x = rhs.x();
    lhs->y = rhs.y();
  }
}
