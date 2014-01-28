#include "monster.hpp"

using namespace swarm;

Monster::Monster(const Vector2f& pos, float size)
  : _size(size)
{
  _pos = pos;
}

