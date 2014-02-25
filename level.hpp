#pragma once

namespace swarm
{
  class Player;
  class Monster;

  class Level
  {
  public:
    bool Load(const string& filename);
    bool PosToBackground(const Vector2f& p, u8* out);

    Player* AddPlayer();
    void AddMonsters(vector<Monster* >* monsters);
    Vector2f GetPlayerPos();

    float _scale;
    int _width, _height;
    vector<u8> _background;

    Texture _texture;
    Sprite _sprite;
    
  };
}
