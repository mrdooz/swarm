#pragma once

namespace swarm
{
  class Player
  {
    friend void intrusive_ptr_add_ref(Player*);
    friend void intrusive_ptr_release(Player*);

    public:
    Player();
    Vector2f _pos;
    Vector2f _oldPos;
    Vector2f _vel;
    Vector2f _acc;
    u32 _refCount;
  };

  typedef intrusive_ptr<Player> PlayerPtr;

  //-----------------------------------------------------------------------------
  inline void intrusive_ptr_add_ref(Player* p)
  {
    ++p->_refCount;
  }

  //-----------------------------------------------------------------------------
  inline void intrusive_ptr_release(Player* p)
  {
    if (--p->_refCount == 0)
    {
      delete p;
    }
  }

}