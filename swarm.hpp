#pragma once
#include "virtual_window.hpp"
#include "rolling_average.hpp"
#include "swarm_server.hpp"
#include "world.hpp"

namespace swarm
{
  class World;
  class Renderer;
  class WindowEventManager;
  class Game;
  class Entity;

  namespace game
  {
    class PlayerLeft;
    class PlayerJoined;
    class SwarmState;
    class PlayerState;
    class GameStarted;
  }

  struct LocalPlayer
  {
    PhysicsState _state;
    PhysicsState _prevState;
    Vector2f _pos;
    int _id;
    int _health;
  };

  class MainWindow : public VirtualWindow
  {
  public:
    MainWindow(const string& title, const Vector2f& pos, const Vector2f& size, Game* game);
    bool OnMouseButtonPressed(const Event& event);
    bool OnMouseButtonReleased(const Event& event);
    virtual void Draw();

    Game* _game;
    Vector2f _clickPos;
    ptime _clickStart;
    CircleShape _playerCircle;
    CircleShape _monsterCircle;
    CircleShape _selectionCircle;

    vector<RectangleShape> _textureRects;

    ptime _lastUpdate;
  };

  class PlayerWindow : public VirtualWindow
  {
    public:
    PlayerWindow(const string& title, const Vector2f& pos, const Vector2f& size, Game* game);
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

  struct RenderPlayer
  {
    PhysicsState _prevState;
    PhysicsState _state;
    Vector2f _pos;
    int _id;
    int _health;
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
    friend class PlayerWindow;
    friend class DebugWindow;
    
    public:
    Game(u16 serverPort, const string& serverAddr);
    ~Game();
    
    bool Init();
    void Run();
    void Close();

    private:

    void ProcessNetworkPackets();
    bool OnKeyPressed(const Event& event);
    bool OnKeyReleased(const Event& event);
    bool OnMouseReleased(const Event& event);

    bool OnLostFocus(const Event& event);
    bool OnGainedFocus(const Event& event);

    void UpdatePlayers();
    void UpdateState(PhysicsState& state, float dt);

    void HandlePlayerJoined(const game::PlayerJoined& msg);
    void HandlePlayerLeft(const game::PlayerLeft& msg);
    void HandleSwarmState(const game::SwarmState& msg);
    void HandlePlayerState(const game::PlayerState& msg);
    bool HandleGameStarted(const game::GameStarted& msg);
    void HandleGameEnded(const game::GameEnded& msg);
    void HandleMonsterDied(const game::MonsterDied& msg);
    void HandlePlayerDied(const game::PlayerDied& msg);

    bool _gameStarted;
    bool _gameEnded;
    bool _done;
    u32 _winnerId;
    string _appRoot;

    unordered_map<u32, RenderPlayer> _remotePlayers;
    LocalPlayer _localPlayer;

    vector<RenderMonster> _renderMonsters;
    unique_ptr<RenderWindow> _renderWindow;
    unique_ptr<WindowEventManager> _eventManager;
    unique_ptr<VirtualWindowManager> _windowManager;
    RollingAverage<s64> _frameTime;
    Font _font;

    Vector2f _mousePos;

    vector<MonsterState> _monsterState;

    Level _level;
    MainWindow* _mainWindow;
    PlayerWindow* _playerWindow;
    DebugWindow* _debugWindow;

    TcpSocket _socket;
    Server _server;
    time_duration _clickDuration;
    bool _sendClick;

    u32 _playerId;
    u16 _serverPort;
    string _serverAddr;
    bool _focus;
  };
}