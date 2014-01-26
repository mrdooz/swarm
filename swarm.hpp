#pragma once

namespace swarm
{
  class World;
  class Renderer;
  class WindowEventManager;

  class Game
  {
    public:
    Game();
    
    bool Init();
    void Run();

    private:
    void FindAppRoot();
    void Update(const time_duration& delta);

    bool OnKeyPressed(const Event& event);
    bool OnKeyReleased(const Event& event);

    bool _done;
    string _appRoot;
    unique_ptr<RenderWindow> _renderWindow;
    unique_ptr<WindowEventManager> _eventManager;
    unique_ptr<World> _world;
    unique_ptr<Renderer> _renderer;
    Font _font;
  };
}