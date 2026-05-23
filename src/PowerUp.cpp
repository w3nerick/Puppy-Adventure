#include "PowerUp.h"
#include "Level.h"
#include <cmath>

static constexpr float SPAWN_TIME    = 0.45f;  // tiempo subiendo desde el bloque
static constexpr float SPAWN_DIST    = 38.0f;  // pixeles que sube al spawn
static constexpr float WALK_SPEED    = 60.0f;
static constexpr float GRAVITY       = 800.0f;

PowerUp::PowerUp(Vector2 spawnTileCenter, PowerUpKind k) {
    kind  = k;
    pos.x = spawnTileCenter.x - WIDTH / 2;
    pos.y = spawnTileCenter.y - HEIGHT / 2; // dentro del bloque
    originY = pos.y;
    vel = {0, 0};
    state = State::Spawning;
    spawnT = 0;
    dir = 1;
}

void PowerUp::Update(float dt, const Level& level) {
    if (state == State::Gone) return;
    animTime += dt;

    if (state == State::Spawning) {
        spawnT += dt;
        float t = spawnT / SPAWN_TIME;
        if (t >= 1.0f) {
            t = 1.0f;
            state = State::Active;
            vel.x = (kind == PowerUpKind::Mushroom) ? WALK_SPEED * dir : 0;
        }
        // Suave subida con easing
        pos.y = originY - SPAWN_DIST * t;
        return;
    }

    // Active: gravedad + (mushroom camina, fire flower se queda)
    vel.y += GRAVITY * dt;
    if (vel.y > 500) vel.y = 500;

    if (kind == PowerUpKind::Mushroom) {
        pos.x += vel.x * dt;
        if (level.RectIntersectsSolid(Bounds())) {
            pos.x -= vel.x * dt;
            dir = -dir;
            vel.x = WALK_SPEED * dir;
        }
    }

    pos.y += vel.y * dt;
    if (level.RectIntersectsSolid(Bounds())) {
        pos.y -= vel.y * dt;
        vel.y = 0;
    }

    // Caer al vacio -> desaparece
    if (pos.y > level.PixelHeight() + 100) {
        state = State::Gone;
    }
}

void PowerUp::Draw() const {
    if (state == State::Gone) return;

    float cx = pos.x + WIDTH / 2;
    float cy = pos.y + HEIGHT / 2;

    if (kind == PowerUpKind::Mushroom) {
        // Hueso magico: estilo hueso clasico con brillo
        Color boneCol = {245, 235, 200, 255};
        Color shadow  = {200, 185, 145, 255};
        // Cuerpo central
        DrawRectangle((int)(cx - 8), (int)(cy - 4), 16, 9, boneCol);
        // Bolas en los extremos (4 esquinas)
        DrawCircle((int)(cx - 9), (int)(cy - 7), 5, boneCol);
        DrawCircle((int)(cx - 9), (int)(cy + 6), 5, boneCol);
        DrawCircle((int)(cx + 9), (int)(cy - 7), 5, boneCol);
        DrawCircle((int)(cx + 9), (int)(cy + 6), 5, boneCol);
        // Sombra
        DrawCircle((int)(cx - 8), (int)(cy - 6), 1.5f, shadow);
        DrawCircle((int)(cx + 8), (int)(cy + 5), 1.5f, shadow);
        // Halo dorado para que se note
        unsigned char glow = (unsigned char)(80 + 40 * sinf(animTime * 5.0f));
        DrawCircleGradient((int)cx, (int)cy, 18, {255, 220, 100, glow}, {255, 220, 100, 0});
    } else {
        // Pimiento de fuego: rojo con tallo verde, animado
        float wobble = sinf(animTime * 6.0f) * 1.5f;
        // Tallo verde
        DrawRectangle((int)(cx - 2), (int)(cy - 12), 4, 4, {60, 140, 60, 255});
        DrawCircle((int)(cx - 4), (int)(cy - 10), 3, {80, 160, 80, 255});
        DrawCircle((int)(cx + 4), (int)(cy - 10), 3, {80, 160, 80, 255});
        // Cuerpo del pimiento (forma de gota invertida)
        Color red    = {220, 40, 40, 255};
        Color redDk  = {160, 20, 20, 255};
        DrawCircle((int)cx, (int)(cy + wobble), 11, red);
        DrawTriangle(
            {cx - 9, cy - 4},
            {cx + 9, cy - 4},
            {cx,     cy + 12 + wobble},
            red
        );
        // Brillito
        DrawCircle((int)(cx - 4), (int)(cy - 2), 2, {255, 220, 220, 200});
        // Halo de fuego
        unsigned char glow = (unsigned char)(100 + 60 * sinf(animTime * 8.0f));
        DrawCircleGradient((int)cx, (int)cy, 22, {255, 100, 50, glow}, {255, 100, 50, 0});
        (void)redDk;
    }
}
