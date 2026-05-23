#include "Game.h"
#include <cmath>
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
static void EmscriptenMainLoop(void* arg) {
    static_cast<Game*>(arg)->Frame();
}
#endif

// Colores del cielo por nivel (deben coincidir con la tematica de Level.cpp)
static const Color SKY_TOP[]    = { {135, 206, 250, 255}, {255, 180, 130, 255}, { 70,  60, 110, 255} };
static const Color SKY_BOTTOM[] = { {200, 230, 255, 255}, {255, 230, 200, 255}, {140, 110, 180, 255} };

Game::Game() {
    InitWindow(SCREEN_W, SCREEN_H, "Puppy Game - Aventura Perruna");
    SetTargetFPS(60);
    level.Build(1);
    ResetLevel();
}

Game::~Game() {
    CloseWindow();
}

void Game::Frame() {
    float dt = GetFrameTime();
    if (dt > 0.05f) dt = 0.05f; // cap delta
    Update(dt);
    Draw();
}

void Game::Run() {
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(EmscriptenMainLoop, this, 0, 1);
#else
    while (!WindowShouldClose()) {
        Frame();
    }
#endif
}

void Game::StartGame() {
    score = 0;
    lives = 3;
    currentLevel = 1;
    level.Build(currentLevel);
    ResetLevel();
}

void Game::ResetLevel() {
    player.Reset(level.PlayerSpawn());
    enemies.clear();
    for (auto& sp : level.EnemySpawns()) {
        enemies.emplace_back(sp);
    }
    for (auto& c : level.Coins()) {
        c.collected = false;
        c.animTime = 0;
    }
    coinsGot = 0;
    time = 0;
}

void Game::LoadNextLevel() {
    currentLevel++;
    level.Build(currentLevel);
    ResetLevel();
}

void Game::Update(float dt) {
    bgTime += dt;

    switch (state) {
        case State::Title:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                StartGame();
                state = State::Playing;
            }
            break;

        case State::Playing: {
            time += dt;
            player.Update(dt, level);
            level.Update(dt);

            for (auto& e : enemies) {
                e.Update(dt, level);
            }

            // Colision con pinchos: muerte instantanea
            if (!player.IsDead() && level.RectIntersectsSpikes(player.Bounds())) {
                player.TakeHit();
            }

            // Colision jugador-enemigo
            if (!player.IsDead()) {
                Rectangle pBounds = player.Bounds();
                for (auto& e : enemies) {
                    if (!e.Alive()) continue;
                    Rectangle eBounds = e.Bounds();
                    if (CheckCollisionRecs(pBounds, eBounds)) {
                        // Si viene cayendo desde arriba -> stomp
                        if (player.Velocity().y > 0 &&
                            pBounds.y + pBounds.height - 10 < eBounds.y + eBounds.height / 2) {
                            e.Stomp();
                            player.Bounce();
                            score += 100;
                        } else {
                            player.TakeHit();
                        }
                    }
                }
            }

            // Recolectar monedas
            if (!player.IsDead()) {
                Rectangle pBounds = player.Bounds();
                for (auto& c : level.Coins()) {
                    if (c.collected) continue;
                    Rectangle cBounds = { c.pos.x - 8, c.pos.y - 8, 16, 16 };
                    if (CheckCollisionRecs(pBounds, cBounds)) {
                        c.collected = true;
                        coinsGot++;
                        score += 50;
                    }
                }
            }

            // Llegar a la meta -> avanzar de nivel o terminar el juego
            if (!player.IsDead()) {
                if (CheckCollisionRecs(player.Bounds(), level.GoalBounds())) {
                    score += 200; // bono por completar nivel
                    if (currentLevel >= Level::TOTAL_LEVELS) {
                        state = State::Win;
                    } else {
                        state = State::LevelComplete;
                        transitionT = 0;
                    }
                }
            }

            // Muerte
            if (player.IsDead()) {
                lives--;
                if (lives <= 0) {
                    state = State::GameOver;
                } else {
                    ResetLevel();
                }
            }

            // Camara sigue al jugador
            camera.target = { player.Position().x + Player::WIDTH / 2, (float)SCREEN_H / 2.0f };
            camera.offset = { SCREEN_W / 2.0f, SCREEN_H / 2.0f };
            camera.zoom = 1.0f;

            // Clamp horizontal
            if (camera.target.x < SCREEN_W / 2.0f)
                camera.target.x = SCREEN_W / 2.0f;
            if (camera.target.x > level.PixelWidth() - SCREEN_W / 2.0f)
                camera.target.x = (float)(level.PixelWidth() - SCREEN_W / 2.0f);

            // Clamp vertical (para niveles altos como el 3)
            float halfH = SCREEN_H / 2.0f;
            float minY  = halfH;
            float maxY  = level.PixelHeight() - halfH;
            // Sigue al jugador en Y solo si el nivel es mas alto que la pantalla
            if (level.PixelHeight() > SCREEN_H) {
                camera.target.y = player.Position().y + Player::HEIGHT / 2.0f;
                if (camera.target.y < minY) camera.target.y = minY;
                if (camera.target.y > maxY) camera.target.y = maxY;
            }
            break;
        }

        case State::LevelComplete:
            transitionT += dt;
            if (transitionT > 1.5f &&
                (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || transitionT > 4.0f)) {
                LoadNextLevel();
                state = State::Playing;
            }
            break;

        case State::Win:
        case State::GameOver:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                state = State::Title;
            }
            break;
    }
}

void Game::Draw() {
    BeginDrawing();

    switch (state) {
        case State::Title:
            DrawBackground();
            DrawTitle();
            break;
        case State::Playing:
            DrawBackground();
            BeginMode2D(camera);
                level.Draw(camera);
                for (auto& e : enemies) e.Draw();
                player.Draw();
            EndMode2D();
            DrawHUD();
            break;
        case State::LevelComplete:
            DrawBackground();
            BeginMode2D(camera);
                level.Draw(camera);
                player.Draw();
            EndMode2D();
            DrawLevelComplete();
            break;
        case State::Win:
            DrawBackground();
            DrawWin();
            break;
        case State::GameOver:
            DrawBackground();
            DrawGameOver();
            break;
    }

    EndDrawing();
}

void Game::DrawBackground() {
    int idx = currentLevel - 1;
    if (state == State::Title) idx = 0;
    if (idx < 0) idx = 0;
    if (idx >= Level::TOTAL_LEVELS) idx = Level::TOTAL_LEVELS - 1;

    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, SKY_TOP[idx], SKY_BOTTOM[idx]);

    // Estrellas en el nivel 3 (noche)
    if (idx == 2) {
        for (int i = 0; i < 60; i++) {
            float sx = fmodf(i * 137.5f, SCREEN_W);
            float sy = fmodf(i * 71.3f, SCREEN_H * 0.7f);
            float twinkle = 0.5f + 0.5f * sinf(bgTime * 2.0f + i);
            DrawCircle((int)sx, (int)sy, 1.0f + twinkle, {255, 255, 255, (unsigned char)(150 + twinkle * 80)});
        }
    } else {
        // Nubes simples moviendose
        for (int i = 0; i < 5; i++) {
            float cx = fmodf(bgTime * 15.0f + i * 220.0f, SCREEN_W + 100) - 50;
            float cy = 40.0f + i * 50.0f;
            unsigned char alpha = (idx == 1) ? 220 : 180;
            DrawEllipse((int)cx, (int)cy, 50, 20, {255, 255, 255, alpha});
            DrawEllipse((int)(cx + 30), (int)(cy - 5), 40, 18, {255, 255, 255, (unsigned char)(alpha - 20)});
            DrawEllipse((int)(cx - 20), (int)(cy + 3), 35, 15, {255, 255, 255, (unsigned char)(alpha - 30)});
        }
    }
}

void Game::DrawHUD() {
    DrawRectangle(0, 0, SCREEN_W, 36, {0, 0, 0, 140});

    char buf[64];
    snprintf(buf, sizeof(buf), "Score: %d", score);
    DrawText(buf, 10, 8, 20, WHITE);

    snprintf(buf, sizeof(buf), "Coins: %d", coinsGot);
    DrawText(buf, 170, 8, 20, GOLD);

    snprintf(buf, sizeof(buf), "Lives: %d", lives);
    DrawText(buf, 320, 8, 20, RED);

    snprintf(buf, sizeof(buf), "Level: %d/%d", currentLevel, Level::TOTAL_LEVELS);
    DrawText(buf, 460, 8, 20, SKYBLUE);

    snprintf(buf, sizeof(buf), "Time: %.0f", time);
    DrawText(buf, SCREEN_W - 120, 8, 20, WHITE);
}

// Dibuja un retrato del perrito (cabeza grande, oreja caida, lengua afuera)
// usado en el menu y pantallas de transicion.
void Game::DrawPuppyPortrait(float cx, float cy, float scale, float bobOffset) const {
    auto S = [&](float v) { return v * scale; };

    Color body  = {205, 145,  80, 255};   // golden / tan
    Color belly = {245, 220, 180, 255};   // crema
    Color earCol= {150,  95,  45, 255};   // marron mas oscuro
    Color noseColor = {35, 25, 25, 255};

    float bob = bobOffset;

    // Cuerpo
    DrawEllipse((int)cx, (int)(cy + bob), S(34), S(28), body);
    DrawEllipse((int)cx, (int)(cy + S(8) + bob), S(22), S(14), belly);

    // Cabeza
    float hy = cy - S(24) + bob;
    DrawCircle((int)cx, (int)hy, S(22), body);
    // Hocico
    DrawEllipse((int)cx, (int)(hy + S(8)), S(11), S(8), belly);

    // Orejas caidas (golden retriever style)
    DrawEllipse((int)(cx - S(20)), (int)(hy + S(2)), S(8), S(16), earCol);
    DrawEllipse((int)(cx + S(20)), (int)(hy + S(2)), S(8), S(16), earCol);

    // Ojos (grandes y redondos)
    DrawCircle((int)(cx - S(8)), (int)(hy - S(2)), S(4), WHITE);
    DrawCircle((int)(cx + S(8)), (int)(hy - S(2)), S(4), WHITE);
    DrawCircle((int)(cx - S(7)), (int)(hy - S(1)), S(2.5f), BLACK);
    DrawCircle((int)(cx + S(7)), (int)(hy - S(1)), S(2.5f), BLACK);
    // Brillito en el ojo
    DrawCircle((int)(cx - S(6)), (int)(hy - S(2)), S(1), WHITE);
    DrawCircle((int)(cx + S(8)), (int)(hy - S(2)), S(1), WHITE);

    // Nariz
    DrawEllipse((int)cx, (int)(hy + S(6)), S(4), S(3), noseColor);

    // Lengua afuera (animada)
    float tongueWiggle = sinf(bgTime * 6.0f) * S(1.5f);
    DrawEllipse((int)(cx + tongueWiggle), (int)(hy + S(13)), S(4), S(5), {255, 120, 140, 255});

    // Cola moviendose
    float tailPhase = sinf(bgTime * 8.0f) * S(8);
    DrawLineEx({cx + S(28), cy + bob}, {cx + S(38) + tailPhase, cy - S(8) + bob}, S(4), body);
    DrawCircle((int)(cx + S(38) + tailPhase), (int)(cy - S(8) + bob), S(5), body);
}

void Game::DrawTitle() {
    const char* title = "PUPPY GAME";
    DrawText(title, SCREEN_W / 2 - MeasureText(title, 60) / 2, 80, 60, {200, 110,  40, 255});
    const char* sub = "Aventura del Perrito";
    DrawText(sub, SCREEN_W / 2 - MeasureText(sub, 28) / 2, 150, 28, DARKGRAY);

    // Perrito grande en la pantalla de titulo
    float bob = sinf(bgTime * 3.0f) * 5.0f;
    DrawPuppyPortrait(SCREEN_W / 2.0f, 280.0f, 1.6f, bob);

    const char* startText = "Presiona ENTER o ESPACIO para jugar";
    float pulse = 0.6f + 0.4f * sinf(bgTime * 4.0f);
    Color startCol = {255, 255, 255, (unsigned char)(pulse * 255)};
    DrawText(startText, SCREEN_W / 2 - MeasureText(startText, 22) / 2, 410, 22, startCol);

    const char* l1 = "Flechas/WASD: Mover  |  Espacio: Saltar";
    DrawText(l1, SCREEN_W / 2 - MeasureText(l1, 16) / 2, 455, 16, LIGHTGRAY);
    const char* l2 = "Salta sobre los enemigos para vencerlos. Esquiva los pinchos!";
    DrawText(l2, SCREEN_W / 2 - MeasureText(l2, 16) / 2, 478, 16, LIGHTGRAY);
    const char* l3 = "3 niveles te esperan: Parque, Bosque y Torre Nocturna";
    DrawText(l3, SCREEN_W / 2 - MeasureText(l3, 14) / 2, 502, 14, {200, 200, 200, 255});
}

void Game::DrawLevelComplete() {
    // Capa oscura semitransparente
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, {0, 0, 0, 160});

    char buf[80];
    snprintf(buf, sizeof(buf), "NIVEL %d COMPLETO!", currentLevel);
    int titleW = MeasureText(buf, 56);
    DrawText(buf, SCREEN_W / 2 - titleW / 2, 130, 56, GREEN);

    snprintf(buf, sizeof(buf), "Score: %d  -  Monedas: %d", score, coinsGot);
    DrawText(buf, SCREEN_W / 2 - MeasureText(buf, 26) / 2, 220, 26, WHITE);

    // Perrito feliz saltando en el centro
    float bob = -fabsf(sinf(bgTime * 4.0f)) * 25.0f; // saltitos
    DrawPuppyPortrait(SCREEN_W / 2.0f, 330.0f, 1.2f, bob);

    snprintf(buf, sizeof(buf), "Siguiente: Nivel %d", currentLevel + 1);
    DrawText(buf, SCREEN_W / 2 - MeasureText(buf, 22) / 2, 420, 22, GOLD);

    if (transitionT > 1.5f) {
        const char* hint = "Presiona ENTER para continuar";
        float pulse = 0.5f + 0.5f * sinf(bgTime * 5.0f);
        Color c = {255, 255, 255, (unsigned char)(pulse * 255)};
        DrawText(hint, SCREEN_W / 2 - MeasureText(hint, 20) / 2, 470, 20, c);
    }
}

void Game::DrawWin() {
    const char* t1 = "JUEGO COMPLETO!";
    DrawText(t1, SCREEN_W / 2 - MeasureText(t1, 56) / 2, 80, 56, GREEN);
    const char* t2 = "El perrito es el mejor heroe del mundo!";
    DrawText(t2, SCREEN_W / 2 - MeasureText(t2, 22) / 2, 150, 22, DARKGRAY);

    float bob = sinf(bgTime * 3.0f) * 6.0f;
    DrawPuppyPortrait(SCREEN_W / 2.0f, 290.0f, 1.5f, bob);

    char buf[64];
    snprintf(buf, sizeof(buf), "Score Final: %d", score);
    DrawText(buf, SCREEN_W / 2 - MeasureText(buf, 30) / 2, 400, 30, WHITE);

    const char* t = "Presiona ENTER para volver al menu";
    DrawText(t, SCREEN_W / 2 - MeasureText(t, 20) / 2, 460, 20, LIGHTGRAY);
}

void Game::DrawGameOver() {
    DrawText("GAME OVER", SCREEN_W / 2 - MeasureText("GAME OVER", 60) / 2, 160, 60, RED);

    char buf[64];
    snprintf(buf, sizeof(buf), "Llegaste al Nivel %d", currentLevel);
    DrawText(buf, SCREEN_W / 2 - MeasureText(buf, 24) / 2, 240, 24, WHITE);

    snprintf(buf, sizeof(buf), "Score: %d", score);
    DrawText(buf, SCREEN_W / 2 - MeasureText(buf, 20) / 2, 280, 20, GOLD);

    const char* t = "Presiona ENTER para reintentar";
    DrawText(t, SCREEN_W / 2 - MeasureText(t, 22) / 2, 360, 22, LIGHTGRAY);
}
