#pragma once

#include "raylib.h"

class Level;

// Enemigo tipo "perro" que patrulla horizontalmente sobre las plataformas.
class Enemy {
public:
    static constexpr float WIDTH  = 34.0f;
    static constexpr float HEIGHT = 26.0f;

    Enemy() = default;
    explicit Enemy(Vector2 spawn);

    void Update(float dt, const Level& level);
    void Draw() const;

    void Stomp();              // aplastado por el jugador

    Rectangle Bounds() const { return { pos.x, pos.y, WIDTH, HEIGHT }; }
    bool      Alive()  const { return state == State::Alive; }
    bool      Dead()   const { return state == State::Gone; }

private:
    enum class State { Alive, Squished, Gone };

    Vector2 pos{};
    Vector2 vel{};
    int     dir   = -1;          // -1 izquierda, +1 derecha
    State   state = State::Alive;
    float   timer = 0.0f;
    float   animTime = 0.0f;
};
