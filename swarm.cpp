#include "swarm.hpp"
#include "player.hpp"
#include "error.hpp"
#include "window_event_manager.hpp"
#include "virtual_window_manager.hpp"
#include "protocol/game.pb.h"
#include "protocol.hpp"

using namespace sf;
using namespace swarm;

void FontPrint(const char* msg)
{
  while (*msg)
  {
    u8 ch = (u8)*msg;
    ++msg;

    if (ch > 127)
      continue;

    char buf[8*9+1];
    memset(buf, 0, sizeof(buf));
    for (int y = 0; y < 8; ++y)
    {
      u8 b = vincent_data[ch][y];

      for (int x = 0; x < 8; ++x)
      {
        u8 a = (b >> (7 - x)) & 1;
        buf[y*9+x] = a ? 'X' : ' ';
      }

      buf[y*9+8] = '\n';

    }
    printf("%s", buf);
  }
}

//----------------------------------------------------------------------------------
void CreateTextRects(const char* str, const Vector2f& center, float scale, vector<RectangleShape>* out)
{
  out->clear();
  if (!str)
    return;

  Vector2f size(scale, scale);
  size_t numChars = strlen(str);
  vector<u32> rowWidth;

  // count rows and find max row width
  vector<string> splits;
  Split(str, "\n", &splits);
  int numRows = splits.size();
  float maxRowWidth = 0;
  for (const string& s : splits)
  {
    maxRowWidth = max<float>(maxRowWidth, s.size());
    rowWidth.push_back(s.size());
  }

  maxRowWidth *= scale * 8;

  Vector2f pos(center.x - maxRowWidth/2, center.y - scale * 8 * numRows/2);
  Vector2f orgPos(pos);

  float tmpY = pos.y;
  int curRow = 0;
  bool newRow = true;
  for (size_t i = 0; i < numChars; ++i)
  {
    u8 ch = (u8)*str;
    str++;
    if (ch > 127)
      continue;

    // get starting x pos, based on current cur width
    if (newRow)
    {
      pos.x = orgPos.x + (maxRowWidth - scale * 8 * rowWidth[curRow]) / 2;
      newRow = false;
    }

    if (ch == '\n')
    {
      tmpY += scale * 8;
      ++curRow;
      newRow = true;
      continue;
    }

    pos.y = tmpY;
    float tmpX = pos.x;
    for (int y = 0; y < 8; ++y)
    {
      u8 row = vincent_data[ch][y];

      pos.x = tmpX;
      for (int x = 0; x < 8; ++x)
      {
        u8 a = (row >> (7 - x)) & 1;
        // if the current bit is set, create a rectangle
        if (a)
        {
          RectangleShape rect;
          rect.setPosition(pos);
          rect.setSize(size);
          out->push_back(rect);
        }
        pos.x += scale;
      }
      pos.y += scale;
    }
  }
}

//-----------------------------------------------------------------------------
PlayerWindow::PlayerWindow(const string& title, const Vector2f& pos, const Vector2f& size, Game* game)
  : VirtualWindow(title, pos, size)
  , _game(game)
{
}
//-----------------------------------------------------------------------------
void PlayerWindow::Draw()
{
  _texture.clear();

  Text header("", _game->_font, 30);
  Text text("", _game->_font, 20);

  // draw local player first, followed by remote players

  Vector2f pos(10, 10);
  text.setColor(Color::Yellow);
  header.setColor(Color::Yellow);
  header.setString(toString("Player %d", _game->_localPlayer._id));
  header.setPosition(pos);
  _texture.draw(header);
  pos.y += 35;

  text.setPosition(pos);
  text.setString(toString("Health: %d", _game->_localPlayer._health));
  _texture.draw(text);
  pos.y += 20;

  text.setColor(Color::White);
  header.setColor(Color::White);

  for (const auto& kv : _game->_remotePlayers)
  {
    const RenderPlayer& player = kv.second;
    header.setString(toString("Player %d", player._id));
    header.setPosition(pos);
    _texture.draw(header);
    pos.y += 35;

    text.setPosition(pos);
    text.setString(toString("Health: %d", player._health));
    _texture.draw(text);
    pos.y += 20;
  }

  _texture.display();
}

//-----------------------------------------------------------------------------
MainWindow::MainWindow(const string& title, const Vector2f& pos, const Vector2f& size, Game* game)
  : VirtualWindow(title, pos, size)
  , _game(game)
  , _playerCircle(5, 100)
  , _monsterCircle(5, 50)
  , _lastUpdate(microsec_clock::local_time())
{
  _playerCircle.setFillColor(Color::Green);
  _monsterCircle.setFillColor(Color::Red);
  _selectionCircle.setFillColor(Color(0xcc, 0xcc, 0, 0x80));

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
  _game->_sendClick = true;
  _game->_clickDuration = microsec_clock::local_time() - _clickStart;
  _clickStart = ptime();
  return true;
}

//-----------------------------------------------------------------------------
void MainWindow::Draw()
{
  _texture.clear();

  if (_game->_gameStarted)
  {
    //View view = _texture.getView();
    //view.setCenter(_game->_world._players[0]->_pos);
    //_texture.setView(view);
    _texture.draw(_game->_level._sprite);
    // draw local player
    Vector2f p(_game->_localPlayer._pos);
    p.x -= 5;
    p.y -= 5;
    _playerCircle.setPosition(p);
    _texture.draw(_playerCircle);

    // draw remote players
    for (const auto& kv: _game->_remotePlayers)
    {
      const RenderPlayer& player = kv.second;
      Vector2f p(player._pos);
      p.x -= 5;
      p.y -= 5;
      _playerCircle.setPosition(p);
      _texture.draw(_playerCircle);
    }

    RenderMonster* monster = _game->_renderMonsters.data();
    MonsterState* state = _game->_monsterState.data();
    for (size_t i = 0; i < _game->_renderMonsters.size(); ++i)
    {
      Vector2f p(monster->_pos);
      p.x -= monster->_size;
      p.y -= monster->_size;
      _monsterCircle.setPosition(p);
      _monsterCircle.setRadius(monster->_size);
      _texture.draw(_monsterCircle);

      ++monster;
      ++state;
    }

    if (!_clickStart.is_not_a_date_time())
    {
      time_duration delta = microsec_clock::local_time() - _clickStart;
      float r = 0.25f * delta.total_microseconds() / 1000.0;
      _selectionCircle.setPosition(_clickPos - r * Vector2f(1,1));
      _selectionCircle.setRadius(r);
      _texture.draw(_selectionCircle);
    }
  }
  else
  {
    s64 ms = microsec_clock::local_time().time_of_day().total_milliseconds();
    vector<RectangleShape> rects;
    CreateTextRects("Waiting for\nplayers", 0.5f * VectorCast<float>(_texture.getSize()), 20 + 15 * sinf(ms / 1000.0), &rects);

    for (const RectangleShape& rect : rects)
    {
      _texture.draw(rect);
    }
  }

  if (_game->_gameEnded)
  {
    s64 ms = microsec_clock::local_time().time_of_day().total_milliseconds();
    vector<RectangleShape> rects;
    if (_game->_winnerId == _game->_playerId)
    {
      CreateTextRects("A winner is you!", 0.5f * VectorCast<float>(_texture.getSize()), 20 + 15 * sinf(ms / 1000.0), &rects);
    }
    else
    {
      CreateTextRects("Game over!", 0.5f * VectorCast<float>(_texture.getSize()), 20 + 15 * sinf(ms / 1000.0), &rects);
    }

    for (const RectangleShape& rect : rects)
    {
      _texture.draw(rect);
    }
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

  frameTime.setString(toString("%.2f ms", _game->_frameTime.GetAverage() / 1000.0f));
  frameTime.setPosition(20, 20);
  _texture.draw(frameTime);

  frameTime.setString(toString("%d, %d", (int)_game->_mousePos.x, (int)_game->_mousePos.y));
  frameTime.setPosition(20, 30);
  _texture.draw(frameTime);

  _texture.display();
}


//----------------------------------------------------------------------------------
Game::Game(u16 serverPort, const string& serverAddr)
  : _gameStarted(false)
  , _gameEnded(false)
  , _done(false)
  , _winnerId(~0)
  , _frameTime(100)
  , _mainWindow(nullptr)
  , _playerWindow(nullptr)
  , _debugWindow(nullptr)
  , _sendClick(false)
  , _playerId(0)
  , _serverPort(serverPort)
  , _serverAddr(serverAddr)
  , _focus(true)
{
}

//----------------------------------------------------------------------------------
Game::~Game()
{
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

  sf::ContextSettings settings;
  settings.antialiasingLevel = 8;
  _renderWindow.reset(new RenderWindow(sf::VideoMode(8 * width / 10, 8 * height / 10), "...", sf::Style::Default, settings));
  //_renderWindow->setFramerateLimit(60);
  _renderWindow->setVerticalSyncEnabled(true);
  _eventManager.reset(new WindowEventManager(_renderWindow.get()));
  _windowManager.reset(new VirtualWindowManager(_renderWindow.get(), _eventManager.get()));

//  if (!_level.Load("data/pacman.png"))
//    return false;

  if (!_font.loadFromFile("gfx/wscsnrg.ttf"))
    return false;

  _eventManager->RegisterHandler(Event::KeyPressed, bind(&Game::OnKeyPressed, this, _1));
  _eventManager->RegisterHandler(Event::KeyReleased, bind(&Game::OnKeyReleased, this, _1));
  _eventManager->RegisterHandler(Event::LostFocus, bind(&Game::OnLostFocus, this, _1));
  _eventManager->RegisterHandler(Event::GainedFocus, bind(&Game::OnGainedFocus, this, _1));

  _playerWindow = new PlayerWindow("PLAYER", Vector2f(0,0), Vector2f(200, 200), this);
  //_debugWindow = new DebugWindow("DEBUG", Vector2f(0,500), Vector2f(200, _renderWindow->getSize().y), this);
  _mainWindow = new MainWindow("MAIN", Vector2f(200,0), Vector2f(_renderWindow->getSize().x-200, _renderWindow->getSize().y), this);
  _windowManager->AddWindow(_playerWindow);
 // _windowManager->AddWindow(_debugWindow);
  _windowManager->AddWindow(_mainWindow);

  // Only run a local server if one wasn't specified
  if (_serverAddr.empty())
  {
    if (!_server.Init("config.pb"))
      return false;

    _serverPort = _server.GetPort();
  }

  _socket.connect(IpAddress(_serverAddr.empty() ? "localhost" : _serverAddr), _serverPort);
  _socket.setBlocking(false);

  return true;
}

//----------------------------------------------------------------------------------
bool Game::OnLostFocus(const Event& event)
{
  _focus = false;
  return true;
}

//----------------------------------------------------------------------------------
bool Game::OnGainedFocus(const Event& event)
{
  _focus = true;
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
  if (!_focus || _gameEnded)
    return;

  float s = 30;

  Vector2f& acc = _localPlayer._state._acc;
  acc = Vector2f(0,0);

  if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A))
    acc += Vector2f(-s, 0);

  if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D))
    acc += Vector2f(+s, 0);

  if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W))
    acc += Vector2f(0, -s);

  if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))
    acc += Vector2f(0, +s);
}

//----------------------------------------------------------------------------------
bool Game::HandleGameStarted(const game::GameStarted& msg)
{
  if (!_level.Load(msg.map_name()))
    return false;

  _gameStarted = true;
  _playerId = msg.player_id();
  _localPlayer._id = _playerId;
  _localPlayer._health = msg.health();

  // Initial player state
  const game::PlayerState& playerState = msg.player_state();

  for (int i = 0; i < playerState.player_size(); ++i)
  {
    const game::Player& player = playerState.player(i);
    u32 id = player.id();

    // copy the initial local player state
    if (player.id() == _playerId)
    {
      FromProtocol(&_localPlayer._state._acc, player.acc());
      FromProtocol(&_localPlayer._state._vel, player.vel());
      FromProtocol(&_localPlayer._state._pos, player.pos());
    }
    else
    {
      RenderPlayer& remotePlayer = _remotePlayers[id];

      remotePlayer._id = player.id();
      remotePlayer._health = player.health();

      FromProtocol(&remotePlayer._state._acc, player.acc());
      FromProtocol(&remotePlayer._state._vel, player.vel());
      FromProtocol(&remotePlayer._state._pos, player.pos());
    }
  }

  // Initial swarm state
  const game::SwarmState& swarmState = msg.swarm_state();
  if (_renderMonsters.size() != swarmState.monster_size())
  {
    _renderMonsters.resize(swarmState.monster_size());
    _monsterState.resize(swarmState.monster_size());
  }

  RenderMonster* monster = _renderMonsters.data();
  MonsterState* state = _monsterState.data();
  for (int i = 0; i < swarmState.monster_size(); ++i)
  {
    const game::Monster& m = swarmState.monster(i);
    FromProtocol(&monster->_pos, m.pos());
    monster->_size = m.size();

    FromProtocol(&state->_curState._acc, m.acc());
    FromProtocol(&state->_curState._vel, m.vel());
    FromProtocol(&state->_curState._pos, m.pos());
    state->_prevState = state->_curState;

    monster++;
    state++;
  }
  return true;
}

//----------------------------------------------------------------------------------
void Game::HandlePlayerJoined(const game::PlayerJoined& msg)
{

}

//----------------------------------------------------------------------------------
void Game::HandlePlayerLeft(const game::PlayerLeft& msg)
{
  _remotePlayers.erase(msg.id());
}

//----------------------------------------------------------------------------------
void Game::HandlePlayerState(const game::PlayerState& msg)
{
  //LOG_INFO(__FUNCTION__ << LogKeyValue("num_players", msg.player_size()));

  for (int i = 0; i < msg.player_size(); ++i)
  {
    const game::Player& player = msg.player(i);
    u32 id = player.id();

    // Don't update position of local player..
    if (player.id() != _playerId)
    {
      RenderPlayer& remotePlayer = _remotePlayers[id];

      remotePlayer._id = player.id();
      remotePlayer._health = player.health();

      FromProtocol(&remotePlayer._state._acc, player.acc());
      FromProtocol(&remotePlayer._state._vel, player.vel());
      FromProtocol(&remotePlayer._state._pos, player.pos());
    }
    else
    {
      _localPlayer._health = player.health();
    }
  }
}

//----------------------------------------------------------------------------------
void Game::HandleMonsterDied(const game::MonsterDied& msg)
{

}

//----------------------------------------------------------------------------------
void Game::HandlePlayerDied(const game::PlayerDied& msg)
{

}

//----------------------------------------------------------------------------------
void Game::HandleGameEnded(const game::GameEnded& msg)
{
  _gameEnded = true;
  _winnerId = msg.winner_id();
}

//----------------------------------------------------------------------------------
void Game::HandleSwarmState(const game::SwarmState& msg)
{
  if (_renderMonsters.size() != msg.monster_size())
  {
    _renderMonsters.resize(msg.monster_size());
    _monsterState.resize(msg.monster_size());
  }

  RenderMonster* monster = _renderMonsters.data();
  MonsterState* state = _monsterState.data();
  for (int i = 0; i < msg.monster_size(); ++i)
  {
    const game::Monster& m = msg.monster(i);
    FromProtocol(&monster->_pos, m.pos());
    monster->_size = m.size();

    FromProtocol(&state->_curState._acc, m.acc());
    FromProtocol(&state->_curState._vel, m.vel());
    FromProtocol(&state->_curState._pos, m.pos());
    state->_prevState = state->_curState;

    monster++;
    state++;
  }
}

//----------------------------------------------------------------------------------
void Game::UpdateState(PhysicsState& state, float dt)
{
  // Velocity Verlet integration
  float friction = 0.99f;
  float scale = _level._scale;
  Vector2f oldVel = state._vel;
  Vector2f p = state._pos;
  Vector2f v = friction * (state._vel + state._acc * dt);

  Vector2f newPos = state._pos + (oldVel + state._vel) * 0.5f * dt;

  u8 b;
  // check horizontal collisions
  if (!(_level.PosToBackground(1/scale * (p + dt * Vector2f(v.x, 0)), &b) && b == 0))
  {
    newPos.x = p.x;
    v.x = -v.x;
  }

  // check vertical
  if (!(_level.PosToBackground(1/scale * (p + dt * Vector2f(0, v.y)), &b) && b == 0))
  {
    newPos.y = p.y;
    v.y = -v.y;
  }

  state._vel = v;
  state._pos = newPos;
}

//----------------------------------------------------------------------------------
void Game::ProcessNetworkPackets()
{
  static char buf[32*1024];

  size_t bytesReceived = 0;
  Socket::Status status = _socket.receive(buf, sizeof(buf), bytesReceived);
  if (status == Socket::Done)
  {
    // LOG_INFO("received from server" << LogKeyValue("num_bytes", bytesReceived));
    char *ptr = buf;
    while (ptr < buf + bytesReceived)
    {
      u32 msgSize = ntohl(*(u32*)ptr);
      ptr += sizeof(u32);
      game::ServerMessage msg;
      if (msg.ParseFromArray(ptr, msgSize))
      {
        ptr += msgSize;
        switch (msg.type())
        {
          case game::ServerMessage_Type_GAME_STARTED:
            HandleGameStarted(msg.game_started());
            break;

          case game::ServerMessage_Type_PLAYER_JOINED:
            HandlePlayerJoined(msg.player_joined());
            break;

          case game::ServerMessage_Type_PLAYER_LEFT:
            HandlePlayerLeft(msg.player_left());
            break;

          case game::ServerMessage_Type_SWARM_STATE:
            HandleSwarmState(msg.swarm_state());
            break;

          case game::ServerMessage_Type_PLAYER_STATE:
            HandlePlayerState(msg.player_state());
            break;

          case game::ServerMessage_Type_GAME_ENDED:
            HandleGameEnded(msg.game_ended());
            break;

          case game::ServerMessage_Type_PLAYER_DIED:
            HandlePlayerDied(msg.player_died());
            break;

          case game::ServerMessage_Type_MONSTER_DIED:
            HandleMonsterDied(msg.monster_died());
            break;
        }
      }
      else
      {
        break;
      }
    }
  }
}

//----------------------------------------------------------------------------------
void Game::Run()
{
  Clock clock;
  clock.restart();

  vector<char> buf(16 * 1024);

  Time lastUpdate = clock.getElapsedTime();
  Time lastSend = clock.getElapsedTime();

  // rendering time adds to the accumulator, and the physics update
  // subtracts from it (using a fixed timestep)
  double timestep = 1/50.0;
  double accumulator = 0;

  while (_renderWindow->isOpen() && !_done)
  {
    Time start = clock.getElapsedTime();
    _renderWindow->clear();
    _eventManager->Poll();
    _windowManager->Update();

    ProcessNetworkPackets();

    if (_gameStarted)
    {
      // check if we should send click state
      if (_sendClick)
      {
        _sendClick = false;
        float r = 0.25f * _clickDuration.total_microseconds() / 1000.0;
        game::PlayerMessage msg;
        msg.set_type(game::PlayerMessage_Type_PLAYER_CLICK);
        game::PlayerClick* click = msg.mutable_click();
        game::Vector2* pos = click->mutable_click_pos();
        pos->set_x(_mainWindow->_clickPos.x);
        pos->set_y(_mainWindow->_clickPos.y);
        click->set_click_size(r);

        string str;
        if (msg.SerializePartialToString(&str))
        {
          _socket.send(str.data(), str.size());
        }
      }

      Time end = clock.getElapsedTime();
      Time computeTime = end - start;
      _frameTime.AddSample(computeTime.asMicroseconds());
      Time delta = end - lastUpdate;
      lastUpdate = end;

      UpdatePlayers();

      accumulator += delta.asMicroseconds() / 1e6;
      while (accumulator >= timestep)
      {
        // Update all the remote players
        for (auto& kv : _remotePlayers)
        {
          RenderPlayer& player = kv.second;
          player._prevState = player._state;
          UpdateState(player._state, (float)timestep);
        }

        // Update the local player
        _localPlayer._prevState = _localPlayer._state;
        UpdateState(_localPlayer._state, (float)timestep);

        for (MonsterState& state : _monsterState)
        {
          state._prevState = state._curState;
          UpdateState(state._curState, (float)timestep);
        }

        accumulator -= timestep;
      }

      // Do the lerp approximation step
      float alpha = (float)(accumulator / timestep);

      for (auto& kv : _remotePlayers)
      {
        RenderPlayer& player = kv.second;
        player._pos = lerp(player._prevState._pos, player._state._pos, alpha);
      }

      _localPlayer._pos = lerp(_localPlayer._prevState._pos, _localPlayer._state._pos, alpha);


      for (size_t i = 0; i < _monsterState.size(); ++i)
      {
        MonsterState& state = _monsterState[i];
        _renderMonsters[i]._pos = lerp(state._prevState._pos, state._curState._pos, alpha);
      }

      if ((end - lastSend).asMilliseconds() > 100)
      {
        // send player state
        game::PlayerMessage msg;
        msg.set_type(game::PlayerMessage_Type_PLAYER_POS);
        ToProtocol(msg.mutable_pos(), _localPlayer._state._pos);

        string str;
        if (msg.SerializePartialToString(&str))
        {
          _socket.send(str.data(), str.size());
        }
        lastSend = end;
      }
    }
    else
    {

    }

    _renderWindow->display();
  }
}

//----------------------------------------------------------------------------------
void Game::Close()
{
  _server.Close();
}

//----------------------------------------------------------------------------------
int main(int argc, char** argv)
{
  u16 serverPort = 50000;
  string serverAddr;

  for (int i = 1; i < argc; ++i)
  {
    if (strcmp(argv[i], "-p") == 0 && i < argc - 1)
      serverPort = atoi(argv[i+1]);

    if (strcmp(argv[i], "-s") == 0  && i < argc - 1)
      serverAddr = argv[i+1];
  }

  srand(1337);
  Game game(serverPort, serverAddr);

  if (!game.Init())
    return 1;

  game.Run();

  game.Close();

  return 0;
}
