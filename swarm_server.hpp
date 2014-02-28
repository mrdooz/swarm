#include "world.hpp"
#include "level.hpp"
#include "physics.hpp"
#include "shared.hpp"
#include "protocol/game.pb.h"

namespace swarm
{
  class Entity;

  class Server
  {
  public:
    Server();
    ~Server();

    bool Init(const char* configFile);
    bool Close();

    u16 GetPort() const { return _port; }

  private:

    bool InitLevel();
    void PlayerAdded(TcpSocket* socket);

    void UpdateState(PhysicsState& state, float dt);

    void SendPlayerState();
    void SendMonsterState(float alpha);
    void SendToClients(const vector<char>& buf);
    bool SendToClient(const vector<char>& buf, TcpSocket* socket);

    void HandleClientMessages();
    void ApplyAttractor(const Vector2f& pos, float radius);
    void SendPlayerDied(u32 id);

    void HandleCollisions();

    void ResetGame();

    void ThreadProc();

    template <typename T>
    bool PackMessage(vector<char>& buf, const T& msg);

    template <typename T>
    bool SendMessageToClients(const T& msg);

    void AddMonster(const Vector2f& pos, float size);

    struct MonsterAttractor
    {
      MonsterAttractor(const Vector2f& pos, float radius) : pos(pos), radius(radius) {}
      Vector2f pos;
      float radius;
    };

    struct PlayerData
    {
      PlayerData() : id(~0), sentStartGame(false), alive(true) {}
      u32 id;
      Vector2f pos;
      int health;
      bool sentStartGame;
      bool alive;
    };

    struct MonsterData
    {
      MonsterState _state;
      float _size;
      int _health;
    };

    vector<MonsterData> _monsterData;
    vector<MonsterAttractor> _attractors;

    Level _level;

    vector<TcpSocket *> _connectedClients;
    map<pair<u32, u16>, u32> _addrToId;

    typedef map<u32, PlayerData> PlayerDataById;
    PlayerDataById _playerData;

    thread* _serverThread;
    u8 _networkBuffer[32*1024];

    TcpListener _listener;
    u16 _port;
    atomic<bool> _done;
    u32 _nextPlayerId;
    game::Config _config;
    bool _gameStarted;
  };
}
