#pragma once
#include "protocol/game.pb.h"

namespace swarm
{
  void ToProtocol(game::Vector2* lhs, const Vector2f& rhs);
  void FromProtocol(Vector2f* lhs, const game::Vector2& rhs);
}