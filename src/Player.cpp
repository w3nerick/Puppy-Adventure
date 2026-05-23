#include "Player.h"
#include "Level.h"
#include <cmath>

static constexpr float GRAVITY    = 1400.0f;
static constexpr float MOVE_SPEED = 280.0f;
static constexpr float JUMP_VEL   = -680.0f;   // salto mas alto
static constexpr float JUMP_CUT   = 0.45f;     // si sueltas espacio mientras subes, recortamos
static constexpr float MAX_FALL   = 750.0f;
static constexpr float IFRAMES    = 1.4f;
static constexpr float TRANSFORM  = 0.5f;
static constexpr float COYOTE_T   = 0.10f;     // grace para saltar tras caer del borde
static constexpr float JUMP_BUF   = 0.12f;     // buffer si presionas justo antes de tocar suelo

float Player::Width()  const { return (state == PowerState::Small) ? SMALL_W : BIG_W; }
float Player::Height() const { return (state == PowerState::Small) ? SMALL_H : BIG_H; }

Rectangle Player::Bounds() const {
    return { pos.x, pos.y, Width(), Height() };
}

void Player::Reset(Vector2 spawn) {
    pos = spawn;
    vel = {0, 0};
    onGround   = false;
    dead       = false;
    facing     = 1;
    animTime   = 0;
    animFrame  = 0;
    stepTime   = 0;
    state      = PowerState::Small;
    iframes    = 0;
    transformT = 0;
    wantShoot  = false;
    coyoteT    = 0;
    jumpBufT   = 0;
    jumping    = false;
}

void Player::Update(float dt, const Level& level) {
    if (dead) return;

    // Timers
    if (iframes    > 0) iframes    -= dt;
    if (transformT > 0) transformT -= dt;
    if (coyoteT    > 0) coyoteT    -= dt;
    if (jumpBufT   > 0) jumpBufT   -= dt;

    // Movimiento horizontal
    float moveInput = 0.0f;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) moveInput += 1.0f;
    if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) moveInput -= 1.0f;

    // Sprint con SHIFT (estilo SMB)
    float speed = MOVE_SPEED;
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) speed *= 1.55f;

    vel.x = moveInput * speed;
    if (moveInput != 0) facing = (moveInput > 0) ? 1 : -1;

    // Buffer de salto: si presionaste hace poco, queda guardado
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        jumpBufT = JUMP_BUF;
    }

    // Salto: dispara si tienes buffer Y (estas en suelo o en coyote time)
    bool canJump = onGround || coyoteT > 0;
    if (jumpBufT > 0 && canJump) {
        vel.y = JUMP_VEL;
        onGround = false;
        coyoteT  = 0;
        jumpBufT = 0;
        jumping  = true;
        justJumped = true;
    }

    // Variable jump height: si sueltas el boton mientras subes, recortamos vel.y
    bool jumpHeld = IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
    if (jumping && !jumpHeld && vel.y < 0) {
        vel.y *= JUMP_CUT;
        jumping = false;
    }
    if (vel.y >= 0) jumping = false;

    // Disparo (solo en Fire) - tecla X o J
    if (state == PowerState::Fire &&
        (IsKeyPressed(KEY_X) || IsKeyPressed(KEY_J))) {
        wantShoot = true;
    }

    // Gravedad
    vel.y += GRAVITY * dt;
    if (vel.y > MAX_FALL) vel.y = MAX_FALL;

    // Mover en X
    pos.x += vel.x * dt;
    if (level.RectIntersectsSolid(Bounds())) {
        pos.x -= vel.x * dt;
        vel.x = 0;
    }

    // Mover en Y
    bool wasOnGround = onGround;
    pos.y += vel.y * dt;
    if (level.RectIntersectsSolid(Bounds())) {
        pos.y -= vel.y * dt;
        if (vel.y > 0) onGround = true;
        vel.y = 0;
    } else {
        onGround = false;
    }
    // Si recien dejamos el suelo (no por saltar), activar coyote time
    if (wasOnGround && !onGround && !jumping) {
        coyoteT = COYOTE_T;
    }

    if (pos.x < 0) pos.x = 0;

    // Caer al vacio
    if (pos.y > level.PixelHeight() + 100) {
        dead = true;
    }

    // Animacion
    animTime += dt;
    if (vel.x != 0 && onGround) {
        stepTime += dt;
        float stepInterval = (fabsf(vel.x) > MOVE_SPEED) ? 0.08f : 0.13f;
        if (stepTime > stepInterval) {
            stepTime = 0;
            animFrame = 1 - animFrame;
        }
    } else {
        animFrame = 0;
    }
}

bool Player::ConsumeShootRequest() {
    if (wantShoot) { wantShoot = false; return true; }
    return false;
}

bool Player::ConsumeJustJumped() {
    if (justJumped) { justJumped = false; return true; }
    return false;
}

void Player::Draw() const {
    if (dead) return;

    // Parpadeo durante iframes
    if (iframes > 0 && (int)(iframes * 14) % 2 == 0) return;

    float x = pos.x;
    float y = pos.y;
    float w = Width();
    float h = Height();
    float bob = onGround ? sinf(animTime * 9.0f) * 1.2f * (float)(vel.x != 0) : 0.0f;

    // Paleta segun estado
    Color body, belly, earCol, leg;
    switch (state) {
        case PowerState::Big:
            body   = {235, 165,  90, 255};   // dorado mas brillante
            belly  = {250, 230, 195, 255};
            earCol = {175, 110,  55, 255};
            leg    = {195, 130,  65, 255};
            break;
        case PowerState::Fire:
            body   = {255, 230, 230, 255};   // blanco con tinte rojo
            belly  = {255, 250, 250, 255};
            earCol = {220,  60,  60, 255};   // orejas rojas como Fire Mario
            leg    = {235,  80,  80, 255};
            break;
        default: // Small
            body   = {205, 145,  80, 255};
            belly  = {245, 220, 180, 255};
            earCol = {150,  95,  45, 255};
            leg    = {165, 110,  55, 255};
            break;
    }
    Color tongueC = {255, 130, 150, 255};
    Color noseC   = { 35,  25,  25, 255};

    // Escala visual segun el estado (Big/Fire son ~30% mas grandes que Small)
    float s = (state == PowerState::Small) ? 1.0f : 1.3f;

    float cx = x + w / 2;
    float cy = y + h / 2 + bob;

    // Cuerpo
    DrawEllipse((int)cx, (int)cy, w / 2.1f, h / 2.4f, body);
    DrawEllipse((int)cx, (int)(cy + 4 * s), w / 3.5f, h / 4.5f, belly);

    // Cabeza
    float headOff = facing * 2.0f;
    float headX   = cx + headOff;
    float headY   = y + 6 * s + bob;
    float headR   = 11 * s;
    DrawCircle((int)headX, (int)headY, headR, body);
    DrawEllipse((int)(headX + facing * 3 * s), (int)(headY + 5 * s), 6 * s, 4 * s, belly);

    // Orejas caidas
    DrawEllipse((int)(headX - 9 * s), (int)(headY + 1 * s), 4 * s, 9 * s, earCol);
    DrawEllipse((int)(headX + 9 * s), (int)(headY + 1 * s), 4 * s, 9 * s, earCol);

    // Manchita en la cabeza
    DrawEllipse((int)(headX - facing * 4 * s), (int)(headY - 5 * s), 4 * s, 3 * s, earCol);

    // Ojos
    float eyeShift = facing * 1.5f;
    DrawCircle((int)(headX - 4 * s + eyeShift), (int)(headY - 1 * s), 3 * s, WHITE);
    DrawCircle((int)(headX + 4 * s + eyeShift), (int)(headY - 1 * s), 3 * s, WHITE);
    DrawCircle((int)(headX - 4 * s + eyeShift + facing), (int)(headY - 1 * s), 2 * s, BLACK);
    DrawCircle((int)(headX + 4 * s + eyeShift + facing), (int)(headY - 1 * s), 2 * s, BLACK);
    DrawCircle((int)(headX - 4 * s + eyeShift + facing - 0.5f), (int)(headY - 2 * s), 0.7f * s, WHITE);
    DrawCircle((int)(headX + 4 * s + eyeShift + facing - 0.5f), (int)(headY - 2 * s), 0.7f * s, WHITE);

    // Nariz
    DrawEllipse((int)(headX + facing * 6 * s), (int)(headY + 4 * s), 2.5f * s, 2 * s, noseC);

    // Lengua afuera animada
    float tongueWiggle = sinf(animTime * 7.0f) * 1.0f;
    DrawEllipse((int)(headX + facing * 5 * s + tongueWiggle), (int)(headY + 9 * s), 3 * s, 4 * s, tongueC);

    // Collar especial en Fire (rojo brillante con un piedra)
    if (state == PowerState::Fire) {
        DrawCircle((int)cx, (int)(cy - 8 * s), 14 * s, {200, 40, 40, 255});
        DrawCircle((int)cx, (int)(cy - 8 * s), 11 * s, body);
        // Piedrita brillante
        DrawCircle((int)cx, (int)(cy - 4 * s), 2 * s, {255, 220, 50, 255});
    }

    // Cola
    float tailPhase = sinf(animTime * 10.0f) * 6.0f * s;
    float tailX = (facing > 0) ? x - 2 * s : x + w + 2 * s;
    float tailDir = (facing > 0) ? -1.0f : 1.0f;
    Vector2 tailStart = {tailX, cy - 2 * s};
    Vector2 tailMid   = {tailX + tailDir * 7 * s, cy - 8 * s + tailPhase * 0.5f};
    Vector2 tailEnd   = {tailX + tailDir * 12 * s, cy - 14 * s + tailPhase};
    DrawLineEx(tailStart, tailMid, 4.0f * s, body);
    DrawLineEx(tailMid,   tailEnd, 3.5f * s, body);
    DrawCircle((int)tailEnd.x, (int)tailEnd.y, 3 * s, belly);

    // Patas
    float legSwing = (animFrame == 1 && vel.x != 0) ? 2.5f * s : 0.0f;
    float legY = y + h - 5 * s + bob;
    DrawRectangle((int)(x + 5 * s  - legSwing), (int)legY, 5 * s, 6 * s, leg);
    DrawRectangle((int)(x + 12 * s + legSwing), (int)legY, 5 * s, 6 * s, leg);
    DrawRectangle((int)(x + w - 17 * s - legSwing), (int)legY, 5 * s, 6 * s, leg);
    DrawRectangle((int)(x + w - 10 * s + legSwing), (int)legY, 5 * s, 6 * s, leg);

    // Animacion de transformacion (parpadeo dorado)
    if (transformT > 0) {
        unsigned char alpha = (unsigned char)((sinf(transformT * 30.0f) * 0.5f + 0.5f) * 100);
        DrawCircleGradient((int)cx, (int)cy, w * 0.9f, {255, 230, 100, alpha}, {255, 230, 100, 0});
    }
}

void Player::Bounce() {
    vel.y = JUMP_VEL * 0.6f;
}

void Player::TakeHit() {
    if (iframes > 0) return; // todavia invencible

    if (state == PowerState::Fire || state == PowerState::Big) {
        // Downgrade
        state = PowerState::Small;
        iframes = IFRAMES;
        transformT = TRANSFORM;
    } else {
        // Small -> muerte
        dead = true;
    }
}

void Player::ApplyPowerUp(PowerState newState) {
    // Solo se sube de nivel; un Hueso en Big o Fire da puntos pero no baja
    if ((int)newState > (int)state) {
        state = newState;
        transformT = TRANSFORM;
    }
}
