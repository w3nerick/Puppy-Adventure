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
    enum class State {
        Title,
        Playing,
        LevelComplete,  // pantalla intermedia entre niveles
        Win,            // termino el ultimo nivel
        GameOver
    };

    void Update(float dt);
    void Draw();

    void StartGame();
    void ResetLevel();
    void LoadNextLevel();

    void DrawHUD();
    void DrawTitle();
    void DrawLevelComplete();
    void DrawWin();
    void DrawGameOver();
    void DrawBackground();
    void DrawPuppyPortrait(float cx, float cy, float scale, float bobOffset) const;

    Level              level;
    Player             player;
    std::vector<Enemy> enemies;
    Camera2D           camera{};

    State  state         = State::Title;
    int    score         = 0;
    int    lives         = 3;
    int    coinsGot      = 0;
    int    currentLevel  = 1;
    float  time          = 0.0f;
    float  bgTime        = 0.0f;
    float  transitionT   = 0.0f;  // timer para pantallas de transicion
};
