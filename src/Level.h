#pragma once

#include "raylib.h"
#include <vector>
#include <string>

// Tipos de tile que componen el mapa.
enum class Tile : unsigned char {
    Empty,
    Ground,   // bloque de tierra/césped
    Brick,    // bloque flotante (ladrillo)
    Goal      // bloque/poste de meta
};

// Moneda recolectable.
struct Coin {
    Vector2 pos;       // centro de la moneda
    bool    collected;
    float   animTime;
};

// Mapa basado en tiles. Carga el nivel desde un layout ASCII.
class Level {
public:
    static constexpr int TILE = 40;

    Level();

    // Construye el nivel a partir del layout interno.
    void Build();

    // Anima monedas, etc.
    void Update(float dt);

    // Dibuja sólo los tiles visibles (cull con la cámara).
    void Draw(const Camera2D& camera) const;

    // Acceso al grid.
    bool IsSolid(int tx, int ty) const;
    bool RectIntersectsSolid(Rectangle r) const;

    int Cols()        const { return cols; }
    int Rows()        const { return rows; }
    int PixelWidth()  const { return cols * TILE; }
    int PixelHeight() const { return rows * TILE; }

    Vector2 PlayerSpawn() const { return playerSpawn; }
    const std::vector<Vector2>& EnemySpawns() const { return enemySpawns; }

    std::vector<Coin>&       Coins()       { return coins; }
    const std::vector<Coin>& Coins() const { return coins; }

    Rectangle GoalBounds() const;

private:
    int cols = 0;
    int rows = 0;
    std::vector<Tile> tiles;

    Vector2 playerSpawn{};
    Vector2 goalPos{};
    std::vector<Vector2> enemySpawns;
    std::vector<Coin>    coins;

    Tile  At(int x, int y) const { return tiles[y * cols + x]; }
    Tile& At(int x, int y)       { return tiles[y * cols + x]; }
};
