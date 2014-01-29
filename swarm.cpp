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
  game->_windowManager->RegisterHandler(Event::MouseButtonReleased, this, bind(&MainWindow::OnMouseButtonReleased, this, _1));
  game->_windowManager->RegisterHandler(Event::MouseButtonPressed, this, bind(&MainWindow::OnMouseButtonPressed, this, _1));
}

//-----------------------------------------------------------------------------
bool MainWindow::OnMouseButtonPressed(const Event& event)
{
  _clickStart = microsec_clock::local_time();

  // Convert mouse pos to world space
  _clickPos = _texture.mapPixelToCoords(Vector2i(event.mouseButton.x, event.mouseButton.y));
  _game->_mousePos = _clickPos;

  return true;
}

//-----------------------------------------------------------------------------
bool MainWindow::OnMouseButtonReleased(const Event& event)
{
  _clickStart = ptime();
  return true;
}

//-----------------------------------------------------------------------------
void MainWindow::Draw()
{
  _texture.clear();
  //auto* world = _game->_world.get();
  
  View view = _texture.getView();
  view.setCenter(_game->_renderPlayers[0]._pos);
  _texture.setView(view);
  _texture.draw(_game->_level->_sprite);

  CircleShape circle;
  for (const RenderPlayer& player : _game->_renderPlayers)
  {
    circle.setFillColor(Color::Green);
    Vector2f p(VectorCast<float>(player._pos));
    p.x -= 5;
    p.y -= 5;
    circle.setPosition(p);
    circle.setRadius(5);
    _texture.draw(circle);
  }

  for (const RenderMonster& monster : _game->_renderMonsters)
  {
    circle.setFillColor(Color::Red);
    Vector2f p(VectorCast<float>(monster._pos));
    p.x -= monster._size;
    p.y -= monster._size;
    circle.setPosition(p);
    circle.setRadius(monster._size);
    _texture.draw(circle);
  }

  _clickRadius = -1.0f;
  if (!_clickStart.is_not_a_date_time())
  {
    time_duration delta = microsec_clock::local_time() - _clickStart;
    circle.setFillColor(Color(0xcc, 0xcc, 0, 0x80));
    float r = 0.25f * delta.total_microseconds() / 1000.0;
    circle.setPosition(_clickPos - r * Vector2f(1,1));
    circle.setRadius(r);
    _texture.draw(circle);
    _clickRadius = r;
  }

  _texture.display();
}

//-----------------------------------------------------------------------------
PlayerWindow::PlayerWindow(const string& title, const Vector2f& pos, const Vector2f& size, Game* game)
  : VirtualWindow(title, pos, size)
{
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

  frameTime.setString(toString("%d, %d", (int)_game->_mousePos.x, (int)_game->_mousePos.y));
  frameTime.setPosition(20, 30);
  _texture.draw(frameTime);

  _texture.display();
}

//----------------------------------------------------------------------------------
Game::Game()
  : _done(false)
  , _frameTime(100)
  , _level(nullptr)
  , _mainWindow(nullptr)
  , _playerWindow(nullptr)
  , _debugWindow(nullptr)
{
}

//----------------------------------------------------------------------------------
Game::~Game()
{
  delete exch_null(_level);
  delete exch_null(_mainWindow);
  delete exch_null(_playerWindow);
  delete exch_null(_debugWindow);
}

//----------------------------------------------------------------------------------
bool Game::Init()
{
  _appRoot = FindAppRoot();
  if (_appRoot.empty())
    return false;

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
  //_world.reset(World::Create());
  _level = new Level();
  if (!_level->Load("data/pacman.png"))
    return false;

  //_world->AddPlayer();
  //_world->AddMonsters();

  if (!_font.loadFromFile("gfx/wscsnrg.ttf"))
    return false;

  _eventManager->RegisterHandler(Event::KeyPressed, bind(&Game::OnKeyPressed, this, _1));
  _eventManager->RegisterHandler(Event::KeyReleased, bind(&Game::OnKeyReleased, this, _1));

  _debugWindow = new DebugWindow("DEBUG", Vector2f(0,0), Vector2f(200, _renderWindow->getSize().y), this);
  _mainWindow = new MainWindow("MAIN", Vector2f(200,0), Vector2f(_renderWindow->getSize().x-200, _renderWindow->getSize().y), this);
  _windowManager->AddWindow(_debugWindow);
  _windowManager->AddWindow(_mainWindow);

  if (!_server.Init())
    return false;

  TcpSocket socket;
  socket.connect(IpAddress("localhost"), 50000);
  socket.setBlocking(false);

  _renderPlayers.push_back(RenderPlayer());

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
bool Game::OnMouseReleased(const Event& event)
{

  return true;
}

//----------------------------------------------------------------------------------
void Game::UpdatePlayers()
{
  float s = 20;

  _player._acc = Vector2f(0,0);

  if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A))
    _player._acc += Vector2f(-s, 0);

  if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D))
    _player._acc += Vector2f(+s, 0);

  if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W))
    _player._acc += Vector2f(0, -s);

  if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))
    _player._acc += Vector2f(0, +s);

  _renderPlayers[0]._pos = _player._pos;
}

//----------------------------------------------------------------------------------
void Game::UpdateEntity(Entity& entity, float dt)
{
  // Velocity Verlet integration
  float friction = 0.99f;
  float scale = _level->_scale;
  Vector2f oldVel = entity._vel;
  Vector2f p = entity._pos;
  Vector2f v = friction * (entity._vel + entity._acc * dt);

  Vector2f newPos = entity._pos + (oldVel + entity._vel) * 0.5f * dt;

  u8 b;
  // check horizontal collisions
  if (!(_level->PosToBackground(1/scale * (p + dt * Vector2f(v.x, 0)), &b) && b == 0))
  {
    newPos.x = p.x;
    v.x = -v.x;
  }

  // check vertical
  if (!(_level->PosToBackground(1/scale * (p + dt * Vector2f(0, v.y)), &b) && b == 0))
  {
    newPos.y = p.y;
    v.y = -v.y;
  }

  entity._vel = v;
  entity._pos = newPos;
}

//----------------------------------------------------------------------------------
void Game::Update(const time_duration& delta)
{
  UpdatePlayers();

  float dt = delta.total_milliseconds() / 1000.0f;
  UpdateEntity(_player, dt);
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