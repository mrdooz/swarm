#pragma once

namespace swarm
{
  class Player;
  class Monster;

  class Level
  {
  public:
    bool Load(const char* filename);
    bool PosToBackground(const Vector2f& p, u8* out);

    Player* AddPlayer();
    void AddMonsters(vector<Monster* >* monsters);

    float _scale;
    int _width, _height;
    vector<u8> _background;

    Texture _texture;
    Sprite _sprite;
    
  };
}
