#pragma once

#include "raylib.h"

class Level;

// Proyectil que dispara el perrito en estado Fire.
// Es una pequena bola con texto "BARK!" que rebota en el suelo y
// se desplaza horizontalmente. Mata enemigos de un toque.
class Bark {
public:
    static constexpr float SIZE = 14.0f;

    Bark() = default;
    Bark(Vector2 origin, int dir);

    void Update(float dt, const Level& level);
    void Draw() const;

    bool Alive() const { return alive; }
    void Kill()        { alive = false; }

    Rectangle Bounds() const {
        return { pos.x - SIZE / 2, pos.y - SIZE / 2, SIZE, SIZE };
    }

private:
    Vector2 pos{};
    Vector2 vel{};
    bool    alive    = true;
    float   animTime = 0.0f;
    float   lifetime = 0.0f;
};
