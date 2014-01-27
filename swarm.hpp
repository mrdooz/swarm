#pragma once
#include "virtual_window.hpp"
#include "rolling_average.hpp"

namespace swarm
{
  class World;
  class Renderer;
  class WindowEventManager;
  class Game;

  class MainWindow : public VirtualWindow
  {
  public:
    MainWindow(const string& title, const Vector2f& pos, const Vector2f& size, Game* game);
    virtual void Draw();
    Game* _game;
  };

  class DebugWindow : public VirtualWindow
  {
  public:
    DebugWindow(const string& title, const Vector2f& pos, const Vector2f& size, Game* game);
    virtual void Draw();
    Game* _game;
  };

  class Game
  {
    friend class MainWindow;
    friend class DebugWindow;
    
    public:
    Game();
    
    bool Init();
    void Run();

    private:
    void FindAppRoot();
    void Update(const time_duration& delta);

    bool OnKeyPressed(const Event& event);
    bool OnKeyReleased(const Event& event);

    void UpdatePlayers();

    bool _done;
    string _appRoot;
    unique_ptr<RenderWindow> _renderWindow;
    unique_ptr<WindowEventManager> _eventManager;
    unique_ptr<World> _world;
    unique_ptr<VirtualWindowManager> _windowManager;
    RollingAverage<s64> _frameTime;
    Font _font;
  };
}