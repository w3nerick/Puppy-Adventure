#pragma once

#include "raylib.h"
#include "Level.h"
#include "Player.h"
#include "Enemy.h"

#include <vector>

class Game {
public:
    static constexpr int   SCREEN_W = 960;
    static constexpr int   SCREEN_H = 540;

    Game();
    ~Game();

    void Run();
    void Frame();   // un solo frame: usado por el loop nativo y por Emscripten

private:
    enum class State { Title, Playing, Win, GameOver };

    void Update(float dt);
    void Draw();

    void ResetLevel();
    void DrawHUD();
    void DrawTitle();
    void DrawWin();
    void DrawGameOver();
    void DrawBackground();

    Level              level;
    Player             player;
    std::vector<Enemy> enemies;
    Camera2D           camera{};

    State  state    = State::Title;
    int    score    = 0;
    int    lives    = 3;
    int    coinsGot = 0;
    float  time     = 0.0f;
    float  bgTime   = 0.0f;
};
