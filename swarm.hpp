#pragma once
#include "virtual_window.hpp"
#include "rolling_average.hpp"
#include "swarm_server.hpp"
#include "player.hpp"

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

  struct RenderPlayer
  {
    Vector2f _pos;
    float _size;
  };

  struct RenderMonster
  {
    Vector2f _pos;
    float _size;
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

    void UpdateEntity(Entity& entity, float dt);

    bool _done;
    string _appRoot;
    vector<RenderPlayer> _renderPlayers;
    vector<RenderMonster> _renderMonsters;
    unique_ptr<RenderWindow> _renderWindow;
    unique_ptr<WindowEventManager> _eventManager;
    unique_ptr<VirtualWindowManager> _windowManager;
    RollingAverage<s64> _frameTime;
    Font _font;

    Vector2f _mousePos;

    Player _player;
    Level* _level;
    MainWindow* _mainWindow;
    PlayerWindow* _playerWindow;
    DebugWindow* _debugWindow;

    Server _server;
  };
}