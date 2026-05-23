#pragma once

#include "raylib.h"

class Level;

// Estados de poder del perrito (estilo SMB):
//   Small: estado base, un golpe -> muerte
//   Big:   tras agarrar un Hueso, un golpe -> downgrade a Small
//   Fire:  tras agarrar un Pimiento, puede lanzar barks; un golpe -> downgrade
enum class PowerState : unsigned char {
    Small = 0,
    Big   = 1,
    Fire  = 2
};

// Estado del perrito protagonista.
class Player {
public:
    // Dimensiones por estado. Las usamos en runtime via Width() / Height().
    static constexpr float SMALL_W = 30.0f;
    static constexpr float SMALL_H = 32.0f;
    static constexpr float BIG_W   = 36.0f;
    static constexpr float BIG_H   = 46.0f;

    // Compatibilidad con codigo viejo (algunos sitios siguen leyendo
    // Player::WIDTH/HEIGHT como referencia para el spawn). Devuelven
    // las medidas del estado Small.
    static constexpr float WIDTH  = SMALL_W;
    static constexpr float HEIGHT = SMALL_H;

    Player() = default;

    void Reset(Vector2 spawn);
    void Update(float dt, const Level& level);
    void Draw() const;

    // Salta sobre un enemigo (rebote).
    void Bounce();

    // Recibir un golpe: en Fire o Big baja un nivel + da iframes; en Small mata.
    void TakeHit();

    // Aplicar un power-up.
    void ApplyPowerUp(PowerState newState);

    // Devuelve true si el jugador presiono el boton de bark este frame
    // y estamos en Fire (el sistema de Game crea el proyectil).
    bool ConsumeShootRequest();

    // Devuelve true (y se resetea) si el jugador acaba de saltar.
    bool ConsumeJustJumped();

    Rectangle Bounds() const;
    Vector2   Position()  const { return pos; }
    Vector2   Velocity()  const { return vel; }
    bool      IsDead()    const { return dead; }
    bool      OnGround()  const { return onGround; }
    int       FacingDir() const { return facing; }
    PowerState State()    const { return state; }
    bool      Invincible()const { return iframes > 0.0f; }

    float Width()  const;
    float Height() const;

private:
    Vector2    pos{};
    Vector2    vel{};
    bool       onGround   = false;
    bool       dead       = false;
    int        facing     = 1;
    float      animTime   = 0.0f;
    int        animFrame  = 0;
    float      stepTime   = 0.0f;
    PowerState state      = PowerState::Small;
    float      iframes    = 0.0f;   // tiempo de invencibilidad tras un golpe
    float      transformT = 0.0f;   // animacion al transformarse
    bool       wantShoot  = false;  // pendiente para que Game lo consuma
    float      coyoteT    = 0.0f;   // grace period para saltar tras dejar el suelo
    float      jumpBufT   = 0.0f;   // buffer para presion de salto anticipada
    bool       jumping    = false;  // true mientras esta subiendo en un salto
    bool       justJumped = false;  // un frame en true tras presionar salto
};
