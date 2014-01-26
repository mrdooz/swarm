#include "level.hpp"
#include "utils.hpp"
#include "error.hpp"

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

