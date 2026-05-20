#include "Game.h"
#include <cmath>
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
static void EmscriptenMainLoop(void* arg) {
    static_cast<Game*>(arg)->Frame();
}
#endif

Game::Game() {
    InitWindow(SCREEN_W, SCREEN_H, "Cat Game - Aventura Gatuna");
    SetTargetFPS(60);
    level.Build();
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
    // En el navegador no podemos bloquear con un while; cedemos al event loop del browser.
    emscripten_set_main_loop_arg(EmscriptenMainLoop, this, 0, 1);
#else
    while (!WindowShouldClose()) {
        Frame();
    }
#endif
}

void Game::ResetLevel() {
    player.Reset(level.PlayerSpawn());
    enemies.clear();
    for (auto& sp : level.EnemySpawns()) {
        enemies.emplace_back(sp);
    }
    // Reset coins
    for (auto& c : level.Coins()) {
        c.collected = false;
        c.animTime = 0;
    }
    coinsGot = 0;
    time = 0;
}

void Game::Update(float dt) {
    bgTime += dt;

    switch (state) {
        case State::Title:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                state = State::Playing;
                score = 0;
                lives = 3;
                level.Build();
                ResetLevel();
            }
            break;

        case State::Playing: {
            time += dt;
            player.Update(dt, level);
            level.Update(dt);

            // Actualizar enemigos
            for (auto& e : enemies) {
                e.Update(dt, level);
            }

            // Colisión jugador-enemigo
            if (!player.IsDead()) {
                Rectangle pBounds = player.Bounds();
                for (auto& e : enemies) {
                    if (!e.Alive()) continue;
                    Rectangle eBounds = e.Bounds();
                    if (CheckCollisionRecs(pBounds, eBounds)) {
                        // Si viene cayendo desde arriba → stomp
                        if (player.Velocity().y > 0 && pBounds.y + pBounds.height - 10 < eBounds.y + eBounds.height / 2) {
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

            // Llegar a la meta
            if (!player.IsDead()) {
                Rectangle pBounds = player.Bounds();
                Rectangle goalR = level.GoalBounds();
                if (CheckCollisionRecs(pBounds, goalR)) {
                    state = State::Win;
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

            // Cámara sigue al jugador
            camera.target = { player.Position().x + Player::WIDTH / 2, (float)SCREEN_H / 2.0f };
            camera.offset = { SCREEN_W / 2.0f, SCREEN_H / 2.0f };
            camera.zoom = 1.0f;

            // Clamp cámara
            if (camera.target.x < SCREEN_W / 2.0f)
                camera.target.x = SCREEN_W / 2.0f;
            if (camera.target.x > level.PixelWidth() - SCREEN_W / 2.0f)
                camera.target.x = (float)(level.PixelWidth() - SCREEN_W / 2.0f);

            break;
        }

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
    ClearBackground({135, 206, 235, 255}); // cielo azul

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
    // Gradiente de cielo
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, {135, 206, 250, 255}, {200, 230, 255, 255});

    // Nubes simples moviéndose
    for (int i = 0; i < 5; i++) {
        float cx = fmodf(bgTime * 15.0f + i * 220.0f, SCREEN_W + 100) - 50;
        float cy = 40.0f + i * 50.0f;
        DrawEllipse((int)cx, (int)cy, 50, 20, {255, 255, 255, 180});
        DrawEllipse((int)(cx + 30), (int)(cy - 5), 40, 18, {255, 255, 255, 160});
        DrawEllipse((int)(cx - 20), (int)(cy + 3), 35, 15, {255, 255, 255, 150});
    }
}

void Game::DrawHUD() {
    DrawRectangle(0, 0, SCREEN_W, 36, {0, 0, 0, 120});

    char buf[64];
    snprintf(buf, sizeof(buf), "Score: %d", score);
    DrawText(buf, 10, 8, 20, WHITE);

    snprintf(buf, sizeof(buf), "Coins: %d", coinsGot);
    DrawText(buf, 200, 8, 20, GOLD);

    snprintf(buf, sizeof(buf), "Lives: %d", lives);
    DrawText(buf, 400, 8, 20, RED);

    snprintf(buf, sizeof(buf), "Time: %.0f", time);
    DrawText(buf, SCREEN_W - 120, 8, 20, WHITE);
}

void Game::DrawTitle() {
    DrawText("CAT GAME", SCREEN_W / 2 - MeasureText("CAT GAME", 60) / 2, 100, 60, ORANGE);
    DrawText("Aventura Gatuna", SCREEN_W / 2 - MeasureText("Aventura Gatuna", 30) / 2, 170, 30, DARKGRAY);

    // Dibujar un gatito en la pantalla de título
    float bx = SCREEN_W / 2.0f;
    float by = 280.0f;
    float bob = sinf(bgTime * 3.0f) * 5.0f;
    Color catCol = {255, 165, 50, 255};
    DrawEllipse((int)bx, (int)(by + bob), 30, 25, catCol);
    DrawCircle((int)bx, (int)(by - 20 + bob), 18, catCol);
    // Orejas
    DrawTriangle({bx - 14, by - 24 + bob}, {bx - 8, by - 42 + bob}, {bx - 2, by - 24 + bob}, catCol);
    DrawTriangle({bx + 2, by - 24 + bob}, {bx + 8, by - 42 + bob}, {bx + 14, by - 24 + bob}, catCol);
    DrawTriangle({bx - 12, by - 26 + bob}, {bx - 8, by - 38 + bob}, {bx - 4, by - 26 + bob}, PINK);
    DrawTriangle({bx + 4, by - 26 + bob}, {bx + 8, by - 38 + bob}, {bx + 12, by - 26 + bob}, PINK);
    // Ojos
    DrawCircle((int)(bx - 6), (int)(by - 18 + bob), 4, WHITE);
    DrawCircle((int)(bx + 6), (int)(by - 18 + bob), 4, WHITE);
    DrawCircle((int)(bx - 6), (int)(by - 18 + bob), 2, BLACK);
    DrawCircle((int)(bx + 6), (int)(by - 18 + bob), 2, BLACK);
    // Nariz
    DrawCircle((int)bx, (int)(by - 12 + bob), 3, PINK);

    const char* startText = "Presiona ENTER o ESPACIO para jugar";
    float pulse = 0.6f + 0.4f * sinf(bgTime * 4.0f);
    Color startCol = {255, 255, 255, (unsigned char)(pulse * 255)};
    DrawText(startText, SCREEN_W / 2 - MeasureText(startText, 22) / 2, 400, 22, startCol);

    DrawText("Flechas/WASD: Mover | Espacio/Arriba: Saltar", 
             SCREEN_W / 2 - MeasureText("Flechas/WASD: Mover | Espacio/Arriba: Saltar", 16) / 2, 450, 16, LIGHTGRAY);
    DrawText("Salta sobre los enemigos para eliminarlos!", 
             SCREEN_W / 2 - MeasureText("Salta sobre los enemigos para eliminarlos!", 16) / 2, 475, 16, LIGHTGRAY);
}

void Game::DrawWin() {
    DrawText("GANASTE!", SCREEN_W / 2 - MeasureText("GANASTE!", 60) / 2, 150, 60, GREEN);

    char buf[64];
    snprintf(buf, sizeof(buf), "Score Final: %d", score);
    DrawText(buf, SCREEN_W / 2 - MeasureText(buf, 30) / 2, 240, 30, WHITE);

    snprintf(buf, sizeof(buf), "Monedas: %d", coinsGot);
    DrawText(buf, SCREEN_W / 2 - MeasureText(buf, 24) / 2, 290, 24, GOLD);

    const char* t = "Presiona ENTER para volver al menu";
    DrawText(t, SCREEN_W / 2 - MeasureText(t, 20) / 2, 380, 20, LIGHTGRAY);
}

void Game::DrawGameOver() {
    DrawText("GAME OVER", SCREEN_W / 2 - MeasureText("GAME OVER", 60) / 2, 180, 60, RED);
    const char* t = "Presiona ENTER para reintentar";
    DrawText(t, SCREEN_W / 2 - MeasureText(t, 22) / 2, 320, 22, LIGHTGRAY);
}
