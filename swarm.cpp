#include "swarm.hpp"
#include "utils.hpp"
#include "world.hpp"
#include "player.hpp"
#include "renderer.hpp"
#include "rolling_average.hpp"
#include "window_event_manager.hpp"

using namespace sf;
using namespace swarm;

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
  _renderer.reset(new Renderer);
  _eventManager.reset(new WindowEventManager(_renderWindow.get()));
  _world.reset(World::Create());
  if (!_world->_level->Load("data/pacman.png"))
    return false;

  _world->AddPlayer();

  if (!_font.loadFromFile("gfx/wscsnrg.ttf"))
    return false;

  _eventManager->RegisterHandler(Event::KeyPressed, bind(&Game::OnKeyPressed, this, _1));
  _eventManager->RegisterHandler(Event::KeyReleased, bind(&Game::OnKeyReleased  , this, _1));

  return true;
}

//----------------------------------------------------------------------------------
bool Game::OnKeyPressed(const Event& event)
{
  Keyboard::Key key = event.key.code;

  float s = 20;

  switch (key)
  {
    case Keyboard::Escape:
      _done = true;
      break;

    case Keyboard::Left:
      _world->_players[0]->_speed = Vector2f(-s, 0);
      break;

    case Keyboard::Right:
      _world->_players[0]->_speed = Vector2f(+s, 0);
      break;

    case Keyboard::Up:
      _world->_players[0]->_speed = Vector2f(0, -s);
      break;

    case Keyboard::Down:
      _world->_players[0]->_speed = Vector2f(0, +s);
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
void Game::Update(const time_duration& delta)
{
  float scale = _world->_level->_scale;
  for (auto& player : _world->_players)
  {
    Vector2f newPos = player->_pos + delta.total_milliseconds() / 1000.0f * player->_speed;

    u8 b;
    // Check that the new position is inside the world, and doesn't collide with the background
    Vector2f t(newPos.x / scale, newPos.y / scale);
    if (_world->_level->PosToBackground(t, &b) && b == 0)
    {
      player->_pos = newPos;
    }
    else
    {
      player->_speed = Vector2f(0,0);
    }

  }
}

//----------------------------------------------------------------------------------
void Game::Run()
{
  Clock clock;
  clock.restart();

  RollingAverage<s64> avg(100);

  Text frameTime("", _font, 10);
  frameTime.setColor(Color::Blue);

  Time lastUpdate = clock.getElapsedTime();
  while (_renderWindow->isOpen() && !_done)
  {
    Time start = clock.getElapsedTime();
    _renderWindow->clear();
    _eventManager->Poll();
    _renderer->DrawWorld(*_renderWindow, *_world);

    Time end = clock.getElapsedTime();
    Time computeTime = end - start;
    Time delta = end - lastUpdate;
    lastUpdate = end;
    Update(milliseconds(delta.asMilliseconds()));
    avg.AddSample(computeTime.asMicroseconds());
    frameTime.setString(toString("%.2f ms", avg.GetAverage() / 1000.0));
    frameTime.setPosition(20, 20);
    _renderWindow->draw(frameTime);
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