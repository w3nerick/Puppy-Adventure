#include "Bark.h"
#include "Level.h"
#include <cmath>

static constexpr float SPEED      = 420.0f;
static constexpr float GRAVITY    = 900.0f;
static constexpr float BOUNCE_VEL = -360.0f;
static constexpr float MAX_FALL   = 600.0f;
static constexpr float MAX_LIFE   = 2.5f;

Bark::Bark(Vector2 origin, int dir) {
    pos = origin;
    vel = { SPEED * (float)dir, 0 };
    alive = true;
    animTime = 0;
    lifetime = 0;
}

void Bark::Update(float dt, const Level& level) {
    if (!alive) return;

    animTime += dt;
    lifetime += dt;
    if (lifetime > MAX_LIFE) { alive = false; return; }

    // Gravedad
    vel.y += GRAVITY * dt;
    if (vel.y > MAX_FALL) vel.y = MAX_FALL;

    // Mover X
    pos.x += vel.x * dt;
    if (level.RectIntersectsSolid(Bounds())) {
        // Choco con muro -> desaparece
        alive = false;
        return;
    }

    // Mover Y
    pos.y += vel.y * dt;
    if (level.RectIntersectsSolid(Bounds())) {
        pos.y -= vel.y * dt;
        if (vel.y > 0) vel.y = BOUNCE_VEL; // rebota
        else            vel.y = 0;          // toco techo
    }

    // Caer al vacio
    if (pos.y > level.PixelHeight() + 100) {
        alive = false;
    }
}

void Bark::Draw() const {
    if (!alive) return;
    // Esfera con resplandor
    float pulse = 0.7f + 0.3f * sinf(animTime * 18.0f);
    DrawCircleGradient((int)pos.x, (int)pos.y, SIZE * 1.4f * pulse,
                       {255, 200, 100, 200}, {255, 200, 100, 0});
    DrawCircle((int)pos.x, (int)pos.y, SIZE / 2, {255, 240, 200, 255});
    DrawCircle((int)pos.x, (int)pos.y, SIZE / 3, {255, 120,  60, 255});
    DrawCircle((int)(pos.x - 2), (int)(pos.y - 2), 2, WHITE);
}
