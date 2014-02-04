#include "world.hpp"
#include "level.hpp"

namespace swarm
{
  class Entity;

  struct PhysicsState
  {
    PhysicsState() : _acc(0,0), _vel(0,0), _pos(0,0) {}
    Vector2f _acc;
    Vector2f _vel;
    Vector2f _pos;
  };

  class Server
  {
  public:
    Server();
    ~Server();

    bool Init();
    bool Close();

  private:
    void UpdateState(PhysicsState& state, float dt);
    void Update(const time_duration& delta);

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
      Vector2f pos;
    };

    struct MonsterState
    {
      PhysicsState _curState;
      PhysicsState _prevState;
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
    map<pair<u32, u16>, int> _addrToId;

    map<int, PlayerData> _playerData;

    thread* _serverThread;
    u8 _networkBuffer[32*1024];

    atomic<bool> _done;
  };
}
