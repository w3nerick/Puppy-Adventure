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

    // Caer al vacio
    if (pos.y > level.PixelHeight() + 100) {
        dead = true;
    }

    // Animacion de caminar
    animTime += dt;
    if (vel.x != 0 && onGround) {
        stepTime += dt;
        if (stepTime > 0.13f) {
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
    float bob = onGround ? sinf(animTime * 9.0f) * 1.2f * (float)(vel.x != 0) : 0.0f;

    // Paleta del perrito (golden / tan)
    Color body     = {205, 145,  80, 255};   // dorado
    Color belly    = {245, 220, 180, 255};   // crema (panza/hocico)
    Color earCol   = {150,  95,  45, 255};   // marron mas oscuro (orejas)
    Color leg      = {165, 110,  55, 255};   // patas
    Color tongueC  = {255, 130, 150, 255};
    Color noseC    = { 35,  25,  25, 255};

    float cx = x + WIDTH / 2;     // centro horizontal
    float cy = y + HEIGHT / 2 + bob; // centro vertical con bob

    // Cuerpo (ovalado)
    DrawEllipse((int)cx, (int)cy, WIDTH / 2.1f, HEIGHT / 2.4f, body);
    // Panza blanca
    DrawEllipse((int)cx, (int)(cy + 4), WIDTH / 3.5f, HEIGHT / 4.5f, belly);

    // Cabeza (un poco adelantada hacia donde mira)
    float headOff = facing * 2.0f;
    float headX = cx + headOff;
    float headY = y + 6 + bob;
    DrawCircle((int)headX, (int)headY, 11, body);
    // Hocico color crema
    DrawEllipse((int)(headX + facing * 3), (int)(headY + 5), 6, 4, belly);

    // Orejas caidas (ovalos verticales colgando a los lados)
    DrawEllipse((int)(headX - 9), (int)(headY + 1), 4, 9, earCol);
    DrawEllipse((int)(headX + 9), (int)(headY + 1), 4, 9, earCol);

    // Manchita en la cabeza para personalidad (estilo "perro de TikTok")
    DrawEllipse((int)(headX - facing * 4), (int)(headY - 5), 4, 3, earCol);

    // Ojos grandes y redondos
    float eyeShift = facing * 1.5f;
    DrawCircle((int)(headX - 4 + eyeShift), (int)(headY - 1), 3, WHITE);
    DrawCircle((int)(headX + 4 + eyeShift), (int)(headY - 1), 3, WHITE);
    DrawCircle((int)(headX - 4 + eyeShift + facing), (int)(headY - 1), 2, BLACK);
    DrawCircle((int)(headX + 4 + eyeShift + facing), (int)(headY - 1), 2, BLACK);
    // Brillito (vivo)
    DrawCircle((int)(headX - 4 + eyeShift + facing - 0.5f), (int)(headY - 2), 0.7f, WHITE);
    DrawCircle((int)(headX + 4 + eyeShift + facing - 0.5f), (int)(headY - 2), 0.7f, WHITE);

    // Nariz (al final del hocico)
    DrawEllipse((int)(headX + facing * 6), (int)(headY + 4), 2.5f, 2, noseC);

    // Lengua afuera (animada con un wiggle)
    float tongueWiggle = sinf(animTime * 7.0f) * 1.0f;
    DrawEllipse((int)(headX + facing * 5 + tongueWiggle), (int)(headY + 9), 3, 4, tongueC);

    // Cola enroscada y moviendose
    float tailPhase = sinf(animTime * 10.0f) * 6.0f;
    float tailX = (facing > 0) ? x - 2 : x + WIDTH + 2;
    float tailDir = (facing > 0) ? -1.0f : 1.0f;
    Vector2 tailStart = {tailX, cy - 2};
    Vector2 tailMid   = {tailX + tailDir * 7, cy - 8 + tailPhase * 0.5f};
    Vector2 tailEnd   = {tailX + tailDir * 12, cy - 14 + tailPhase};
    DrawLineEx(tailStart, tailMid, 4.0f, body);
    DrawLineEx(tailMid,   tailEnd, 3.5f, body);
    // Punta de la cola con manchita
    DrawCircle((int)tailEnd.x, (int)tailEnd.y, 3, belly);

    // Patas (4: dos delante, dos detras, animadas)
    float legSwing = (animFrame == 1 && vel.x != 0) ? 2.5f : 0.0f;
    float legY = y + HEIGHT - 5 + bob;
    leg.a = 255;
    // Delanteras
    DrawRectangle((int)(x + 5  - legSwing), (int)legY, 5, 6, leg);
    DrawRectangle((int)(x + 12 + legSwing), (int)legY, 5, 6, leg);
    // Traseras (un poco mas atras visualmente)
    DrawRectangle((int)(x + WIDTH - 17 - legSwing), (int)legY, 5, 6, leg);
    DrawRectangle((int)(x + WIDTH - 10 + legSwing), (int)legY, 5, 6, leg);
}

void Player::Bounce() {
    vel.y = JUMP_VEL * 0.6f;
}

void Player::TakeHit() {
    dead = true;
}
