#pragma once

#include "raylib.h"
#include <vector>
#include <string>

// Tipos de tile que componen el mapa.
enum class Tile : unsigned char {
    Empty,
    Ground,        // bloque de tierra/cesped
    Brick,         // bloque flotante (ladrillo)
    Goal,          // bloque/poste de meta
    Spike,         // pinchos: matan al jugador al tocarlos
    MysteryBlock,  // '?' bloque sorpresa, suelta power-up al golpear desde abajo
    CoinBlock,     // '$' bloque que suelta una moneda al golpear
    UsedBlock      // bloque ya golpeado (vacio, sigue siendo solido)
};

// Que suelta un bloque al ser golpeado desde abajo.
enum class BlockDrop : unsigned char {
    None,
    Coin,
    Mushroom,    // hueso magico (Big)
    FireFlower,  // pimiento de fuego (Fire)
    BrickBroken  // ladrillo destruido por jugador grande
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
    static constexpr int TILE         = 40;
    static constexpr int TOTAL_LEVELS = 3;

    Level();

    void Build(int levelNumber);

    void Update(float dt);

    void Draw(const Camera2D& camera) const;

    // Acceso al grid.
    bool IsSolid(int tx, int ty) const;
    bool RectIntersectsSolid(Rectangle r) const;
    bool RectIntersectsSpikes(Rectangle r) const;

    // Golpea el tile (x,y) desde abajo. Si era un MysteryBlock o CoinBlock,
    // lo convierte en UsedBlock y devuelve que dropea. Si es un Brick y
    // canBreakBricks=true, lo rompe (Empty) y devuelve BrickBroken.
    BlockDrop HitFromBelow(int tx, int ty, bool canBreakBricks);

    // Anade una moneda dinamica (la que sale del CoinBlock).
    void SpawnCoin(Vector2 pos);

    // Posicion en pixeles del centro del tile (tx,ty).
    Vector2 TileCenter(int tx, int ty) const {
        return { (float)(tx * TILE + TILE / 2), (float)(ty * TILE + TILE / 2) };
    }

    int Cols()        const { return cols; }
    int Rows()        const { return rows; }
    int PixelWidth()  const { return cols * TILE; }
    int PixelHeight() const { return rows * TILE; }

    int CurrentLevel() const { return currentLevel; }

    Vector2 PlayerSpawn() const { return playerSpawn; }
    const std::vector<Vector2>& EnemySpawns() const { return enemySpawns; }

    std::vector<Coin>&       Coins()       { return coins; }
    const std::vector<Coin>& Coins() const { return coins; }

    Rectangle GoalBounds() const;

private:
    int cols = 0;
    int rows = 0;
    int currentLevel = 1;
    std::vector<Tile> tiles;

    Vector2 playerSpawn{};
    Vector2 goalPos{};
    std::vector<Vector2> enemySpawns;
    std::vector<Coin>    coins;

    Tile  At(int x, int y) const { return tiles[y * cols + x]; }
    Tile& At(int x, int y)       { return tiles[y * cols + x]; }
};
