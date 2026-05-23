#pragma once

#include "raylib.h"

class Level;

// Tipo de power-up.
enum class PowerUpKind : unsigned char {
    Mushroom,   // hueso magico -> sube a Big
    FireFlower  // pimiento de fuego -> sube a Fire
};

// Item flotante que sale de un MysteryBlock al golpearlo.
// Pasa por una fase de "spawn" subiendo, luego camina como enemigo
// (mushroom) o se queda quieto (fire flower).
class PowerUp {
public:
    static constexpr float WIDTH  = 28.0f;
    static constexpr float HEIGHT = 28.0f;

    PowerUp() = default;
    PowerUp(Vector2 spawnTileCenter, PowerUpKind kind);

    void Update(float dt, const Level& level);
    void Draw() const;

    bool Alive() const { return state != State::Gone; }
    void Collect()     { state = State::Gone; }

    Rectangle  Bounds() const { return { pos.x, pos.y, WIDTH, HEIGHT }; }
    PowerUpKind Kind() const { return kind; }

private:
    enum class State { Spawning, Active, Gone };

    Vector2 pos{};
    Vector2 vel{};
    PowerUpKind kind = PowerUpKind::Mushroom;
    State   state    = State::Spawning;
    float   spawnT   = 0.0f;
    float   originY  = 0.0f;
    float   animTime = 0.0f;
    int     dir      = 1;
};
