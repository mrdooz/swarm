#include "swarm.hpp"
#include "utils.hpp"
#include "world.hpp"
#include "player.hpp"
#include "rolling_average.hpp"
#include "window_event_manager.hpp"
#include "virtual_window_manager.hpp"

using namespace sf;
using namespace swarm;

//-----------------------------------------------------------------------------
MainWindow::MainWindow(const string& title, const Vector2f& pos, const Vector2f& size, Game* game)
  : VirtualWindow(title, pos, size)
  , _game(game)
{

}

//-----------------------------------------------------------------------------
void MainWindow::Draw()
{
  _texture.clear();
  auto* world = _game->_world.get();
  
  View view = _texture.getView();
  view.setCenter(world->_players[0]->_pos);
  _texture.setView(view);
  _texture.draw(world->_level->_sprite);

  for (const auto& player : world->_players)
  {
    CircleShape circle;
    circle.setFillColor(Color::Green);
    Vector2f p(VectorCast<float>(player->_pos));
    p.x -= 5;
    p.y -= 5;
    circle.setPosition(p);
    circle.setRadius(5);
    _texture.draw(circle);
  }
  _texture.display();
}

//-----------------------------------------------------------------------------
DebugWindow::DebugWindow(const string& title, const Vector2f& pos, const Vector2f& size, Game* game)
  : VirtualWindow(title, pos, size)
  , _game(game)
{
}

//-----------------------------------------------------------------------------
void DebugWindow::Draw()
{
  _texture.clear();
  Text frameTime("", _game->_font, 10);
  frameTime.setColor(Color::Blue);

  frameTime.setString(toString("%.2f ms", _game->_frameTime.GetAverage() / 1000.0));
  frameTime.setPosition(20, 20);
  _texture.draw(frameTime);

  _texture.display();
}

//-----------------------------------------------------------------------------
void Game::FindAppRoot()
{
#ifdef _WIN32
  char startingDir[MAX_PATH];
  if (!_getcwd(startingDir, MAX_PATH))
    return;

  // keep going up directory levels until we find "app.json", or we hit the bottom..
  char prevDir[MAX_PATH], curDir[MAX_PATH];
  ZeroMemory(prevDir, sizeof(prevDir));

  while (true) {
    if (!_getcwd(curDir, MAX_PATH))
      return;

    // check if we haven't moved
    if (!strncmp(curDir, prevDir, MAX_PATH))
      break;

    memcpy(prevDir, curDir, MAX_PATH);

    if (fileExists("settings.pb")) {
      _appRoot = curDir;
      return;
    }

    if (_chdir("..") == -1)
      break;
  }
  _appRoot = startingDir;
#else
  char startingDir[256];
  getcwd(startingDir, 256);

  // keep going up directory levels until we find "app.json", or we hit the bottom..
  char prevDir[256], curDir[256];
  memset(prevDir, 0, sizeof(prevDir));

  while (true) {
    getcwd(curDir, 256);
    // check if we haven't moved
    if (!strcmp(curDir, prevDir))
      break;

    memcpy(prevDir, curDir, 256);

    if (FileExists("settings.pb")) {
      _appRoot = curDir;
      return;
    }

    if (chdir("..") == -1)
      break;
  }
  _appRoot = startingDir;
  
#endif
}

//----------------------------------------------------------------------------------
Game::Game()
  : _done(false)
  , _frameTime(100)
{
}

//----------------------------------------------------------------------------------
bool Game::Init()
{
  FindAppRoot();

  size_t width, height;
#ifdef _WIN32
  width = GetSystemMetrics(SM_CXFULLSCREEN);
  height = GetSystemMetrics(SM_CYFULLSCREEN);
#else
  auto displayId = CGMainDisplayID();
  width = CGDisplayPixelsWide(displayId);
  height = CGDisplayPixelsHigh(displayId);
#endif

  _renderWindow.reset(new RenderWindow(sf::VideoMode(8 * width / 10, 8 * height / 10), "..."));
  _renderWindow->setFramerateLimit(60);
  _eventManager.reset(new WindowEventManager(_renderWindow.get()));
  _windowManager.reset(new VirtualWindowManager(_renderWindow.get(), _eventManager.get()));
  _world.reset(World::Create());
  if (!_world->_level->Load("data/pacman.png"))
    return false;

  _world->AddPlayer();

  if (!_font.loadFromFile("gfx/wscsnrg.ttf"))
    return false;

  _eventManager->RegisterHandler(Event::KeyPressed, bind(&Game::OnKeyPressed, this, _1));
  _eventManager->RegisterHandler(Event::KeyReleased, bind(&Game::OnKeyReleased  , this, _1));

  _windowManager->AddWindow(new DebugWindow("DEBUG", Vector2f(0,0), Vector2f(200, _renderWindow->getSize().y), this));
  _windowManager->AddWindow(new MainWindow("MAIN", Vector2f(200,0), Vector2f(_renderWindow->getSize().x-200, _renderWindow->getSize().y), this));

  return true;
}

//----------------------------------------------------------------------------------
bool Game::OnKeyPressed(const Event& event)
{
  Keyboard::Key key = event.key.code;

  switch (key)
  {
    case Keyboard::Escape:
      _done = true;
      break;
  }

  return true;
}

//----------------------------------------------------------------------------------
bool Game::OnKeyReleased(const Event& event)
{
  return true;
}

//----------------------------------------------------------------------------------
void Game::UpdatePlayers()
{
  float s = 20;

  Player& player = *_world->_players[0];

  player._acc = Vector2f(0,0);

  if (Keyboard::isKeyPressed(Keyboard::Left))
    player._acc += Vector2f(-s, 0);

  if (Keyboard::isKeyPressed(Keyboard::Right))
    player._acc += Vector2f(+s, 0);

  if (Keyboard::isKeyPressed(Keyboard::Up))
    player._acc += Vector2f(0, -s);

  if (Keyboard::isKeyPressed(Keyboard::Down))
    player._acc += Vector2f(0, +s);

}

//----------------------------------------------------------------------------------
void Game::Update(const time_duration& delta)
{
  UpdatePlayers();

  float dt = delta.total_milliseconds() / 1000.0f;
  float scale = _world->_level->_scale;
  for (auto& player : _world->_players)
  {
    // Velocity Verlet integration
    float friction = 0.99f;
    Vector2f oldVel = player->_vel;
    Vector2f p = player->_pos;
    Vector2f v = friction * (player->_vel + player->_acc * dt);

    Vector2f newPos = player->_pos + (oldVel + player->_vel) * 0.5f * dt;

    u8 b;
    if (!(_world->_level->PosToBackground(1/scale * (p + dt * Vector2f(v.x, 0)), &b) && b == 0))
    {
      newPos.x = p.x;
      v.x = -v.x;
    }

    if (!(_world->_level->PosToBackground(1/scale * (p + dt * Vector2f(0, v.y)), &b) && b == 0))
    {
      newPos.y = p.y;
      v.y = -v.y;
    }

    player->_vel = v;
    player->_pos = newPos;
  }
}

//----------------------------------------------------------------------------------
void Game::Run()
{
  Clock clock;
  clock.restart();

  RollingAverage<s64> avg(100);

  Time lastUpdate = clock.getElapsedTime();
  while (_renderWindow->isOpen() && !_done)
  {
    Time start = clock.getElapsedTime();
    _renderWindow->clear();
    _eventManager->Poll();
    _windowManager->Update();

    Time end = clock.getElapsedTime();
    Time computeTime = end - start;
    Time delta = end - lastUpdate;
    lastUpdate = end;
    Update(milliseconds(delta.asMilliseconds()));
    avg.AddSample(computeTime.asMicroseconds());
    _renderWindow->display();
  }
}

//----------------------------------------------------------------------------------
int main(int argc, char** argv)
{
  srand(1337);
  Game game;
  if (!game.Init())
    return 1;

  game.Run();

	return 0;
}