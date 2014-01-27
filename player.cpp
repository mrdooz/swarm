#include "player.hpp"

using namespace swarm;

Player::Player()
  : _pos(0,0)
  , _oldPos(0,0)
  , _vel(0,0)
  , _acc(0,0)
  , _refCount(0)
{
}

