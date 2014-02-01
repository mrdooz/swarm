#include "world.hpp"

namespace swarm
{
  class Entity;

  class Server
  {
  public:
    Server();
    ~Server();

    bool Init();
    bool Close();

  private:
    void UpdateEntity(Entity& entity, float dt);
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
      Vector2f pos;
      float radius;
    };

    struct PlayerData
    {
      Vector2f pos;
    };

    World _world;
    vector<MonsterAttractor> _monsterAttractors;
    vector<TcpSocket *> _connectedClients;

    map<pair<u32, u16>, int> _addrToId;

    map<int, PlayerData> _playerData;

    thread* _serverThread;
  };
}