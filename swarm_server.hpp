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
    void UpdateMonsters();
    void UpdateEntity(Entity& entity, float dt);
    void Update(const time_duration& delta);

    void ThreadProc();

    struct MonsterAttractor
    {
      Vector2f pos;
      float radius;
    };

    World _world;
    vector<MonsterAttractor> _monsterAttractors;
    vector<Socket *> _connectedClients;
    thread* _serverThread;
  };
}