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
    void SendToClients(const vector<char>& buf);

    void HandleClientMessages();
    void ApplyAttractor(const Vector2f& pos, float radius);

    void ThreadProc();

    template <typename T>
    bool PackMessage(vector<char>& buf, const T& msg);

    struct MonsterAttractor
    {
      MonsterAttractor(const Vector2f& pos, float radius) : pos(pos), radius(radius) {}
      Vector2f pos;
      float radius;
    };

    struct PlayerData
    {
      u32 id;
      Vector2f pos;
    };

    struct MonsterData
    {
      float _size;
      int _health;
    };

    vector<MonsterState> _monsterState;
    vector<MonsterData> _monsterData;
    vector<MonsterAttractor> _attractors;

    Level _level;

    vector<TcpSocket *> _connectedClients;
    map<pair<u32, u16>, u32> _addrToId;

    map<u32, PlayerData> _playerData;

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
