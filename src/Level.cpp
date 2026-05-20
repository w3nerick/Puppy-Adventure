#include "Level.h"
#include <cmath>
#include <cstring>

// Layout del nivel en ASCII:
// '#' = Ground, 'B' = Brick, 'P' = Player spawn, 'E' = Enemy spawn,
// 'C' = Coin, 'G' = Goal, '.' = Empty
static const char* LEVEL_DATA[] = {
    "............................................................................................................................",
    "............................................................................................................................",
    "............................................................................................................................",
    "..............................CCCC..............................C..C..C.......................................................",
    ".............................BBBBBB............................BBBBBBB.......................................................",
    "............................................................................................................................",
    "..............CCC...................................................................................................G.......",
    ".............BBBBB.......................BBB.............BBB.............BBB..............BBB.........................GGG......",
    "............................................................................................................................",
    "............................................................................................................................",
    "...P...E..........E...........CCC...........E..........CCC..........E..........CCC..........E.........................G......",
    "################..######..############..############..############..############..############..############################",
    "################..######..############..############..############..############..############..############################",
};

Level::Level() {}

void Level::Build() {
    rows = 13;
    cols = (int)strlen(LEVEL_DATA[0]);
    tiles.resize(cols * rows, Tile::Empty);
    coins.clear();
    enemySpawns.clear();

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            char c = LEVEL_DATA[y][x];
            switch (c) {
                case '#': At(x, y) = Tile::Ground; break;
                case 'B': At(x, y) = Tile::Brick;  break;
                case 'G': At(x, y) = Tile::Goal;
                          goalPos = { (float)(x * TILE + TILE / 2), (float)(y * TILE + TILE / 2) };
                          break;
                case 'P': playerSpawn = { (float)(x * TILE), (float)(y * TILE - 32) }; break;
                case 'E': enemySpawns.push_back({ (float)(x * TILE), (float)(y * TILE - 26) }); break;
                case 'C': coins.push_back({ { (float)(x * TILE + TILE / 2), (float)(y * TILE + TILE / 2) }, false, 0.0f }); break;
                default: break;
            }
        }
    }
}

void Level::Update(float dt) {
    for (auto& coin : coins) {
        if (!coin.collected) {
            coin.animTime += dt;
        }
    }
}

void Level::Draw(const Camera2D& camera) const {
    // Calcular rango visible
    float left = camera.target.x - camera.offset.x / camera.zoom;
    float right = left + (float)GetScreenWidth() / camera.zoom;
    int x0 = std::max(0, (int)(left / TILE) - 1);
    int x1 = std::min(cols - 1, (int)(right / TILE) + 1);

    for (int y = 0; y < rows; y++) {
        for (int x = x0; x <= x1; x++) {
            Tile t = At(x, y);
            float px = (float)(x * TILE);
            float py = (float)(y * TILE);

            if (t == Tile::Ground) {
                // Verde arriba (cesped), marron abajo (tierra)
                DrawRectangle((int)px, (int)py, TILE, TILE / 3, DARKGREEN);
                DrawRectangle((int)px, (int)py + TILE / 3, TILE, TILE - TILE / 3, BROWN);
                DrawRectangleLines((int)px, (int)py, TILE, TILE, {20, 80, 20, 100});
            } else if (t == Tile::Brick) {
                DrawRectangle((int)px, (int)py, TILE, TILE, ORANGE);
                // lineas de ladrillo
                DrawLine((int)px, (int)py + TILE / 2, (int)px + TILE, (int)py + TILE / 2, DARKBROWN);
                DrawLine((int)px + TILE / 2, (int)py, (int)px + TILE / 2, (int)py + TILE, DARKBROWN);
                DrawRectangleLines((int)px, (int)py, TILE, TILE, DARKBROWN);
            } else if (t == Tile::Goal) {
                // Poste de meta con bandera
                DrawRectangle((int)px + TILE / 2 - 3, (int)py, 6, TILE, GRAY);
                DrawTriangle(
                    {px + TILE / 2 + 3, py + 2},
                    {px + TILE / 2 + 3, py + 16},
                    {px + TILE / 2 + 20, py + 9},
                    RED
                );
            }
        }
    }

    // Dibujar monedas
    for (const auto& coin : coins) {
        if (coin.collected) continue;
        float bob = sinf(coin.animTime * 4.0f) * 3.0f;
        float scale = 0.7f + 0.3f * fabsf(cosf(coin.animTime * 3.0f)); // giro
        DrawEllipse((int)coin.pos.x, (int)(coin.pos.y + bob), 8.0f * scale, 8.0f, GOLD);
        DrawEllipse((int)coin.pos.x, (int)(coin.pos.y + bob), 5.0f * scale, 5.0f, YELLOW);
    }
}

bool Level::IsSolid(int tx, int ty) const {
    if (tx < 0 || tx >= cols || ty < 0 || ty >= rows) return false;
    Tile t = At(tx, ty);
    return t == Tile::Ground || t == Tile::Brick;
}

bool Level::RectIntersectsSolid(Rectangle r) const {
    int x0 = (int)(r.x / TILE);
    int y0 = (int)(r.y / TILE);
    int x1 = (int)((r.x + r.width - 1) / TILE);
    int y1 = (int)((r.y + r.height - 1) / TILE);
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++)
            if (IsSolid(x, y)) return true;
    return false;
}

Rectangle Level::GoalBounds() const {
    return { goalPos.x - TILE / 2.0f, goalPos.y - TILE / 2.0f, (float)TILE, (float)TILE };
}
