#pragma once

#include "raylib.h"

class Level;

// Estado del gatito protagonista.
class Player {
public:
    static constexpr float WIDTH  = 30.0f;
    static constexpr float HEIGHT = 32.0f;

    Player() = default;

    void Reset(Vector2 spawn);
    void Update(float dt, const Level& level);
    void Draw() const;

    // Salta sobre un enemigo (rebote).
    void Bounce();

    // Marca daño / muerte.
    void TakeHit();

    Rectangle Bounds() const { return { pos.x, pos.y, WIDTH, HEIGHT }; }
    Vector2   Position() const { return pos; }
    Vector2   Velocity() const { return vel; }
    bool      IsDead()   const { return dead; }
    bool      OnGround() const { return onGround; }
    int       FacingDir()const { return facing; }

private:
    Vector2 pos{};
    Vector2 vel{};
    bool    onGround = false;
    bool    dead     = false;
    int     facing   = 1;        // 1 derecha, -1 izquierda
    float   animTime = 0.0f;     // para bobble de cola/cuerpo
    int     animFrame= 0;        // 0/1 al caminar
    float   stepTime = 0.0f;
};
