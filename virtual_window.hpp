#pragma once

namespace swarm
{
  class VirtualWindowManager;

  namespace WindowFlags
  {
    enum Type
    {
      DrawBorder    =   1 << 1,
      DrawTitle     =   1 << 2,
      Movable       =   1 << 3,
      Resizable     =   1 << 4,

      DefaultFlags  =   DrawBorder | DrawTitle | Movable | Resizable
    };
  }

  class VirtualWindow
  {
    friend class VirtualWindowManager;
  public:

    virtual ~VirtualWindow() {}

    virtual bool Init();
    virtual void Draw() = 0;
    RenderTexture* GetTexture() { return &_texture; }

    bool PointInside(const Vector2f& pos, bool includeBorder);

    // TopBorderOuter is the outermost part of the top border (the part that is as thick
    // as the other side's borders).
    enum { LeftBorder = 1, RightBorder = 2, TopBorder = 4, TopBorderOuter = 8, BottomBorder = 16};
    u32 PointInsideBorder(const Vector2f& pos);

  protected:
    VirtualWindow(
        const string& title,
        const Vector2f& pos,
        const Vector2f& size,
        u32 flags = WindowFlags::DefaultFlags);

    void SetSize(const Vector2f& size);
    void SetPosition(const Vector2f& pos);
    void DrawBorder(RenderWindow* window);

    VirtualWindowManager* _windowManager;

    Sprite _sprite;
    RenderTexture _texture;

    Font _font;

    string _title;
    Vector2f _pos;
    Vector2f _size;
    int _depth;
    u32 _borderWidth;
    u32 _titleBarHeight;
    bool _focus;
    bool _moving;
    bool _resizing;
    u32 _flags;
  };
}