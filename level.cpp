#include "level.hpp"
#include "utils.hpp"
#include "error.hpp"
#include "player.hpp"
#include "monster.hpp"

using namespace swarm;

//----------------------------------------------------------------------------------
bool Level::Load(const char *filename)
{
  Image img;
  if (!img.loadFromFile(filename))
  {
    // logwarn
    return false;
  }

  _width = img.getSize().x;
  _height = img.getSize().y;

  const u32* data = (const u32*)img.getPixelsPtr();
  _background.resize(_width*_height);

  for (int i = 0; i < _width * _height; ++i)
  {
    u32 cur = data[i];
    _background[i] = cur & 0xff;
  }

  _scale = 2;
  _texture.loadFromImage(img);
  _sprite.setScale(_scale, _scale);
  _sprite.setTexture(_texture);

  return true;
}

//----------------------------------------------------------------------------------
bool Level::PosToBackground(const Vector2f& p, u8* out)
{
  if (p.x < 0 || p.x >= _width)
    return false;

  if (p.y < 0 || p.y >= _height)
    return false;

  *out = _background[(int)p.y*_width + p.x];

  return true;
}

//-----------------------------------------------------------------------------
void Level::AddMonsters(vector<Monster* >* monsters)
{
  float scale = _scale;
  for (size_t i = 0; i < 10; ++i)
  {
    while (true)
    {
      int x = rand() % _width;
      int y = rand() % _height;
      float s = 5 + rand() % 5;
      if (_background[y*_width+x] == 0)
      {
        monsters->push_back(new Monster(scale * Vector2f(x, y), s));
        break;
      }
    }
  }
}

//-----------------------------------------------------------------------------
Vector2f Level::GetPlayerPos()
{
  Vector2f pos;
  float scale = _scale;
  // find a free spot to put the player in
  while (true)
  {
    int x = rand() % _width/4;
    int y = rand() % _height/4;
    if (_background[y*_width+x] == 0)
    {
      pos = scale * Vector2f(x, y);
      break;
    }
  }

  return pos;
}

//-----------------------------------------------------------------------------
Player* Level::AddPlayer()
{
  Player* player(new Player);

  float scale = _scale;
  // find a free spot to put the player in
  while (true)
  {
    int x = rand() % _width/4;
    int y = rand() % _height/4;
    if (_background[y*_width+x] == 0)
    {
      player->_pos = scale * Vector2f(x, y);
      return player;
    }
  }
}
