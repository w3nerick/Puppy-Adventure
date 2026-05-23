#pragma once

#include "raylib.h"
#include <vector>
#include <string>

enum class ParticleKind : unsigned char {
    BrickPiece,    // rota, gravedad, color marron-naranja
    CoinSpark,     // dorado, sube/cae rapido, fade
    StompPuff,     // gris, expande, fade
    JumpDust,      // gris claro, despega lateral
    ScorePopup     // texto que sube y se desvanece
};

struct Particle {
    Vector2      pos{};
    Vector2      vel{};
    float        life     = 0.0f;
    float        maxLife  = 1.0f;
    float        rot      = 0.0f;
    float        rotVel   = 0.0f;
    float        size     = 4.0f;
    Color        color    = WHITE;
    ParticleKind kind     = ParticleKind::CoinSpark;
    std::string  text;        // solo para ScorePopup
};

class Particles {
public:
    void SpawnBrickBreak(Vector2 center, Color brickColor);
    void SpawnCoinSpark(Vector2 center);
    void SpawnStomp(Vector2 center);
    void SpawnJumpDust(Vector2 feetPos, int facing);
    void SpawnScorePopup(Vector2 pos, int amount, Color col = YELLOW);

    void Update(float dt);
    void Draw() const;

    void Clear() { items.clear(); }

private:
    std::vector<Particle> items;
};
