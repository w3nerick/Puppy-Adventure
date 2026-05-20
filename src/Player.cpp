#include "Player.h"
#include "Level.h"
#include <cmath>

static constexpr float GRAVITY    = 1200.0f;
static constexpr float MOVE_SPEED = 280.0f;
static constexpr float JUMP_VEL   = -520.0f;
static constexpr float MAX_FALL   = 700.0f;

void Player::Reset(Vector2 spawn) {
    pos = spawn;
    vel = {0, 0};
    onGround = false;
    dead = false;
    facing = 1;
    animTime = 0;
    animFrame = 0;
    stepTime = 0;
}

void Player::Update(float dt, const Level& level) {
    if (dead) return;

    // Movimiento horizontal
    float moveInput = 0.0f;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) moveInput += 1.0f;
    if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) moveInput -= 1.0f;

    vel.x = moveInput * MOVE_SPEED;
    if (moveInput != 0) facing = (moveInput > 0) ? 1 : -1;

    // Salto
    if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && onGround) {
        vel.y = JUMP_VEL;
        onGround = false;
    }

    // Gravedad
    vel.y += GRAVITY * dt;
    if (vel.y > MAX_FALL) vel.y = MAX_FALL;

    // Mover en X
    pos.x += vel.x * dt;
    Rectangle hBox = Bounds();
    if (level.RectIntersectsSolid(hBox)) {
        pos.x -= vel.x * dt;
        vel.x = 0;
    }

    // Mover en Y
    pos.y += vel.y * dt;
    Rectangle vBox = Bounds();
    if (level.RectIntersectsSolid(vBox)) {
        pos.y -= vel.y * dt;
        if (vel.y > 0) onGround = true;
        vel.y = 0;
    } else {
        onGround = false;
    }

    // Clamp al inicio del nivel
    if (pos.x < 0) pos.x = 0;

    // Caer al vacío
    if (pos.y > level.PixelHeight() + 100) {
        dead = true;
    }

    // Animación de caminar
    animTime += dt;
    if (vel.x != 0 && onGround) {
        stepTime += dt;
        if (stepTime > 0.15f) {
            stepTime = 0;
            animFrame = 1 - animFrame;
        }
    } else {
        animFrame = 0;
    }
}

void Player::Draw() const {
    if (dead) return;

    float x = pos.x;
    float y = pos.y;
    float bob = onGround ? sinf(animTime * 8.0f) * 1.5f * (float)(vel.x != 0) : 0.0f;

    // Cuerpo del gato (ovalado)
    Color bodyColor = {255, 165, 50, 255}; // naranja
    DrawEllipse((int)(x + WIDTH / 2), (int)(y + HEIGHT / 2 + bob), WIDTH / 2.2f, HEIGHT / 2.3f, bodyColor);

    // Cabeza
    float headY = y + 4 + bob;
    DrawCircle((int)(x + WIDTH / 2), (int)(headY + 6), 11, bodyColor);

    // Orejas
    float earX = x + WIDTH / 2;
    float earY = headY - 2;
    DrawTriangle(
        {earX - 9, earY + 4}, {earX - 4, earY - 7}, {earX - 1, earY + 4}, bodyColor);
    DrawTriangle(
        {earX + 1, earY + 4}, {earX + 4, earY - 7}, {earX + 9, earY + 4}, bodyColor);
    // Interior orejas
    DrawTriangle(
        {earX - 7, earY + 3}, {earX - 4, earY - 4}, {earX - 2, earY + 3}, PINK);
    DrawTriangle(
        {earX + 2, earY + 3}, {earX + 4, earY - 4}, {earX + 7, earY + 3}, PINK);

    // Ojos
    float eyeDir = facing * 2.0f;
    DrawCircle((int)(earX - 4 + eyeDir), (int)(headY + 7), 3, WHITE);
    DrawCircle((int)(earX + 4 + eyeDir), (int)(headY + 7), 3, WHITE);
    DrawCircle((int)(earX - 4 + eyeDir + facing), (int)(headY + 7), 1.5f, BLACK);
    DrawCircle((int)(earX + 4 + eyeDir + facing), (int)(headY + 7), 1.5f, BLACK);

    // Nariz
    DrawCircle((int)(earX + eyeDir * 0.5f), (int)(headY + 11), 2, PINK);

    // Bigotes
    float whiskerY = headY + 10;
    DrawLine((int)(earX - 3), (int)whiskerY, (int)(earX - 14), (int)(whiskerY - 2), DARKGRAY);
    DrawLine((int)(earX - 3), (int)whiskerY, (int)(earX - 14), (int)(whiskerY + 2), DARKGRAY);
    DrawLine((int)(earX + 3), (int)whiskerY, (int)(earX + 14), (int)(whiskerY - 2), DARKGRAY);
    DrawLine((int)(earX + 3), (int)whiskerY, (int)(earX + 14), (int)(whiskerY + 2), DARKGRAY);

    // Cola
    float tailPhase = sinf(animTime * 5.0f) * 8.0f;
    float tailX = (facing > 0) ? x - 2 : x + WIDTH + 2;
    float tailDir = (facing > 0) ? -1.0f : 1.0f;
    DrawLineEx(
        {tailX, y + HEIGHT / 2 + bob},
        {tailX + tailDir * 14, y + HEIGHT / 2 - 10 + tailPhase + bob},
        3.0f, bodyColor
    );

    // Patas (simples)
    float legOffset = (animFrame == 1) ? 3.0f : 0.0f;
    Color legColor = {200, 130, 40, 255};
    DrawRectangle((int)(x + 5 - legOffset), (int)(y + HEIGHT - 6 + bob), 6, 6, legColor);
    DrawRectangle((int)(x + WIDTH - 11 + legOffset), (int)(y + HEIGHT - 6 + bob), 6, 6, legColor);
}

void Player::Bounce() {
    vel.y = JUMP_VEL * 0.6f;
}

void Player::TakeHit() {
    dead = true;
}
