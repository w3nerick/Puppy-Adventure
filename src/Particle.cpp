#include "Particle.h"
#include <cmath>
#include <cstdio>
#include <algorithm>

static constexpr float P_GRAVITY = 1100.0f;

void Particles::SpawnBrickBreak(Vector2 c, Color brickColor) {
    // 4 pedazos volando en diagonal
    float spread = 220.0f;
    Vector2 dirs[4] = {
        {-spread, -spread},
        { spread, -spread},
        {-spread * 0.6f, -spread * 1.2f},
        { spread * 0.6f, -spread * 1.2f}
    };
    for (int i = 0; i < 4; i++) {
        Particle p;
        p.pos = c;
        p.vel = dirs[i];
        p.life = 0;
        p.maxLife = 1.2f;
        p.rot = 0;
        p.rotVel = (i % 2 == 0 ? 8.0f : -8.0f);
        p.size = 12.0f;
        p.color = brickColor;
        p.kind = ParticleKind::BrickPiece;
        items.push_back(p);
    }
}

void Particles::SpawnCoinSpark(Vector2 c) {
    for (int i = 0; i < 8; i++) {
        Particle p;
        p.pos = c;
        float angle = (float)i * 6.2831853f / 8.0f;
        float speed = 130.0f + (i % 3) * 20.0f;
        p.vel = { cosf(angle) * speed, sinf(angle) * speed - 60.0f };
        p.life = 0;
        p.maxLife = 0.6f;
        p.size = 4.0f;
        p.color = (i % 2 == 0) ? GOLD : YELLOW;
        p.kind = ParticleKind::CoinSpark;
        items.push_back(p);
    }
}

void Particles::SpawnStomp(Vector2 c) {
    for (int i = 0; i < 6; i++) {
        Particle p;
        p.pos = c;
        float angle = (float)i * 3.14159f / 6.0f - 3.14159f;
        p.vel = { cosf(angle) * 90.0f, sinf(angle) * 60.0f };
        p.life = 0;
        p.maxLife = 0.4f;
        p.size = 8.0f;
        p.color = {200, 200, 200, 255};
        p.kind = ParticleKind::StompPuff;
        items.push_back(p);
    }
}

void Particles::SpawnJumpDust(Vector2 feetPos, int facing) {
    for (int i = 0; i < 3; i++) {
        Particle p;
        p.pos = feetPos;
        p.vel = { -facing * (40.0f + i * 15.0f), -20.0f - i * 8.0f };
        p.life = 0;
        p.maxLife = 0.35f;
        p.size = 5.0f;
        p.color = {220, 210, 190, 220};
        p.kind = ParticleKind::JumpDust;
        items.push_back(p);
    }
}

void Particles::SpawnScorePopup(Vector2 pos, int amount, Color col) {
    Particle p;
    p.pos = pos;
    p.vel = { 0, -55.0f };
    p.life = 0;
    p.maxLife = 1.1f;
    p.size = 18.0f;
    p.color = col;
    p.kind = ParticleKind::ScorePopup;
    char buf[16];
    snprintf(buf, sizeof(buf), "+%d", amount);
    p.text = buf;
    items.push_back(p);
}

void Particles::Update(float dt) {
    for (auto& p : items) {
        p.life += dt;
        // Movimiento
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        // Gravedad segun tipo
        switch (p.kind) {
            case ParticleKind::BrickPiece:
            case ParticleKind::CoinSpark:
                p.vel.y += P_GRAVITY * dt;
                break;
            case ParticleKind::StompPuff:
            case ParticleKind::JumpDust:
                p.vel.x *= 0.92f;
                p.vel.y *= 0.92f;
                break;
            case ParticleKind::ScorePopup:
                p.vel.y *= 0.97f;
                break;
        }
        p.rot += p.rotVel * dt;
    }
    // Eliminar muertos
    items.erase(
        std::remove_if(items.begin(), items.end(),
            [](const Particle& p){ return p.life >= p.maxLife; }),
        items.end()
    );
}

void Particles::Draw() const {
    for (const auto& p : items) {
        float t     = p.life / p.maxLife;     // 0..1
        float alpha = 1.0f - t;
        unsigned char a = (unsigned char)(alpha * 255);
        Color c = p.color;
        c.a = a;

        switch (p.kind) {
            case ParticleKind::BrickPiece: {
                // Cuadrado rotado
                Rectangle rec = { p.pos.x, p.pos.y, p.size, p.size };
                Vector2 origin = { p.size / 2, p.size / 2 };
                DrawRectanglePro(rec, origin, p.rot * 57.2958f, c);
                break;
            }
            case ParticleKind::CoinSpark: {
                float r = p.size * (1.0f - t * 0.5f);
                DrawCircle((int)p.pos.x, (int)p.pos.y, r, c);
                // Brillo blanco al inicio
                if (t < 0.3f) {
                    Color w = {255, 255, 255, (unsigned char)(a * 0.8f)};
                    DrawCircle((int)p.pos.x, (int)p.pos.y, r * 0.5f, w);
                }
                break;
            }
            case ParticleKind::StompPuff: {
                float r = p.size + t * 10.0f;
                DrawCircle((int)p.pos.x, (int)p.pos.y, r, c);
                break;
            }
            case ParticleKind::JumpDust: {
                float r = p.size + t * 4.0f;
                DrawCircle((int)p.pos.x, (int)p.pos.y, r, c);
                break;
            }
            case ParticleKind::ScorePopup: {
                int fs = (int)p.size;
                int tw = MeasureText(p.text.c_str(), fs);
                // Sombra
                Color shadow = {0, 0, 0, (unsigned char)(a * 0.5f)};
                DrawText(p.text.c_str(), (int)p.pos.x - tw / 2 + 1,
                         (int)p.pos.y + 1, fs, shadow);
                DrawText(p.text.c_str(), (int)p.pos.x - tw / 2,
                         (int)p.pos.y, fs, c);
                break;
            }
        }
    }
}
