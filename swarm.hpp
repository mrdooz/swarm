#pragma once
#include "virtual_window.hpp"
#include "rolling_average.hpp"

namespace swarm
{
  class World;
  class Renderer;
  class WindowEventManager;
  class Game;
  class Entity;

  class MainWindow : public VirtualWindow
  {
  public:
    MainWindow(const string& title, const Vector2f& pos, const Vector2f& size, Game* game);
    bool OnMouseButtonPressed(const Event& event);
    bool OnMouseButtonReleased(const Event& event);
    virtual void Draw();

    Game* _game;
    Vector2f _clickPos;
    float _clickRadius;
    ptime _clickStart;
  };

  class PlayerWindow : public VirtualWindow
  {
    public:
    PlayerWindow(const string& title, const Vector2f& pos, const Vector2f& size, Game* game);

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
    ~Game();
    
    bool Init();
    void Run();

    private:
    void Update(const time_duration& delta);

    bool OnKeyPressed(const Event& event);
    bool OnKeyReleased(const Event& event);
    bool OnMouseReleased(const Event& event);

    void UpdatePlayers();
    void UpdateMonsters();

    void UpdateEntity(Entity& entity, float dt);

    bool _done;
    string _appRoot;
    unique_ptr<RenderWindow> _renderWindow;
    unique_ptr<WindowEventManager> _eventManager;
    unique_ptr<World> _world;
    unique_ptr<VirtualWindowManager> _windowManager;
    RollingAverage<s64> _frameTime;
    Font _font;

    Vector2f _mousePos;

    MainWindow* _mainWindow;
    PlayerWindow* _playerWindow;
    DebugWindow* _debugWindow;
  };
}