#include "Enemy.h"
#include "Level.h"
#include <cmath>

static constexpr float SPEED   = 80.0f;
static constexpr float GRAVITY = 800.0f;

Enemy::Enemy(Vector2 spawn) : pos(spawn), dir(-1), state(State::Alive) {
    vel = { SPEED * (float)dir, 0 };
}

void Enemy::Update(float dt, const Level& level) {
    if (state == State::Gone) return;

    if (state == State::Squished) {
        timer -= dt;
        if (timer <= 0) state = State::Gone;
        return;
    }

    // Gravedad
    vel.y += GRAVITY * dt;
    if (vel.y > 600.0f) vel.y = 600.0f;

    // Movimiento horizontal
    vel.x = SPEED * (float)dir;
    pos.x += vel.x * dt;

    // Colisión horizontal
    Rectangle hBox = Bounds();
    if (level.RectIntersectsSolid(hBox)) {
        pos.x -= vel.x * dt;
        dir = -dir;
        vel.x = SPEED * (float)dir;
    }

    // Movimiento vertical
    pos.y += vel.y * dt;
    Rectangle vBox = Bounds();
    if (level.RectIntersectsSolid(vBox)) {
        pos.y -= vel.y * dt;
        vel.y = 0;
    }

    // Detectar borde de plataforma → cambiar dirección
    int checkX = (dir < 0) ? (int)(pos.x / Level::TILE) : (int)((pos.x + WIDTH) / Level::TILE);
    int checkY = (int)((pos.y + HEIGHT + 4) / Level::TILE);
    if (!level.IsSolid(checkX, checkY)) {
        dir = -dir;
    }

    animTime += dt;
}

void Enemy::Draw() const {
    if (state == State::Gone) return;

    float x = pos.x;
    float y = pos.y;

    if (state == State::Squished) {
        // Aplastado: solo queda un óvalo plano
        DrawEllipse((int)(x + WIDTH / 2), (int)(y + HEIGHT - 4), WIDTH / 2, 5, DARKGRAY);
        return;
    }

    // Cuerpo del perro enemigo (gris oscuro)
    Color dogColor = {100, 100, 110, 255};
    float bob = sinf(animTime * 6.0f) * 2.0f;

    // Cuerpo
    DrawEllipse((int)(x + WIDTH / 2), (int)(y + HEIGHT / 2 + bob), WIDTH / 2.2f, HEIGHT / 2.3f, dogColor);

    // Cabeza
    DrawCircle((int)(x + WIDTH / 2 + dir * 5), (int)(y + 8 + bob), 9, dogColor);

    // Ojos rojos malvados
    float eyeX = x + WIDTH / 2 + dir * 5;
    float eyeY = y + 7 + bob;
    DrawCircle((int)(eyeX - 3), (int)eyeY, 2.5f, RED);
    DrawCircle((int)(eyeX + 3), (int)eyeY, 2.5f, RED);
    DrawCircle((int)(eyeX - 3), (int)eyeY, 1.0f, BLACK);
    DrawCircle((int)(eyeX + 3), (int)eyeY, 1.0f, BLACK);

    // Cejas enojadas
    DrawLine((int)(eyeX - 5), (int)(eyeY - 4), (int)(eyeX - 2), (int)(eyeY - 3), BLACK);
    DrawLine((int)(eyeX + 2), (int)(eyeY - 3), (int)(eyeX + 5), (int)(eyeY - 4), BLACK);

    // Patas
    float legOff = sinf(animTime * 8.0f) * 3.0f;
    DrawRectangle((int)(x + 5 + legOff), (int)(y + HEIGHT - 6 + bob), 5, 6, DARKGRAY);
    DrawRectangle((int)(x + WIDTH - 10 - legOff), (int)(y + HEIGHT - 6 + bob), 5, 6, DARKGRAY);
}

void Enemy::Stomp() {
    state = State::Squished;
    timer = 0.5f;
    vel = {0, 0};
}
