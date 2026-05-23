#include "Level.h"
#include <cmath>
#include <cstring>

// =====================================================================
// Layouts ASCII de los niveles.
//   '#' = Ground       'B' = Brick       'S' = Spike
//   '?' = MysteryBlock (suelta hueso/pimiento alternando)
//   '$' = CoinBlock    'U' = UsedBlock (ya golpeado, no aparece en layouts)
//   'P' = Player spawn 'E' = Enemy       'C' = Coin estatica   'G' = Goal
//   '.' = Empty
// =====================================================================

// Nivel 1: el parque (introductorio, presenta los ? blocks)
static const char* LEVEL_1[] = {
    "............................................................................................................................",
    "............................................................................................................................",
    "..............................................................................................................................",
    "..............................CCCC................................?...?.?.?.................................................",
    ".............................BB?BBB............................BBBBBBB.....................................................",
    "............................................................................................................................",
    "..............CCC.....................................................$......................................................G..",
    ".............BB?BB......................BBB.............BBB............BBB..............BBB.........................GGG......",
    "............................................................................................................................",
    "............................................................................................................................",
    "...P...E..........E...........CCC..........E..........CCC..........E..........CCC..........E........................G......",
    "################..######..############..############..############..############..############..############################",
    "################..######..############..############..############..############..############..############################",
};

// Nivel 2: el bosque (mas saltos, spikes, mas enemigos, ? blocks ocultos)
static const char* LEVEL_2[] = {
    "............................................................................................................................",
    "............................................................................................................................",
    "...........CCC............................................CCCCC.................................................CCC.........",
    "..........BB?BB..............BBB............BBB...........BBB?BBB.............BB.............BBB.............BB?BBB..........",
    "............................................................................................................................",
    "...........................CCC............CCC.............................CCCCC...........CCC.................................",
    ".......................BB?BBB.........BBBBBB............................BB?BBBBB.......BBBBBB...........................G....",
    "............................................................................................................................",
    "...P..C.....E.....E...........CCC.................E.....E....................E.......................E.....CCC.........G....",
    "............................................................................................................................",
    "########..######....##########..########..############..######..########..######..############..############..############",
    "########..######SSSS##########..########..############..######SS########..######SS############..############..############",
    "########..####################..########..############..################..################..############..############",
};

// Nivel 3: la torre (mas vertical, ? blocks con power-ups potentes)
static const char* LEVEL_3[] = {
    ".............................................................................................................................",
    "..............................................................................................CCCCCC.........................",
    "..............................................................................................BB?BBB.........................",
    "..............CCC...........CCC...........CCC...........CCC...........CCC.................................................G..",
    "............BB?BBB.........BBBBBB.........BB?BBB.........BBBBBB.........BB?BBB.....................................GGGGGG.....",
    "..............................................................................................................................",
    "...........E.................E...............E................E..............E...............................................",
    "....BBBB...........BBBB.............BB?B............BBBB.............BBBB.............BBBB................BBBBBBBBBBBBBBB......",
    "..............................................................................................................................",
    "..C..............................CC$CC.................................CC?CC................................................",
    "...P...E.................E.....BBBBBBB.........E.................E.....BBBBBBB.........E.....E....E....E....E....E............",
    "########SS##########SS##########..############SS##########SS##############..############..############..############..########",
    "##########################################################################################################################",
};

struct LevelDef {
    const char* const* data;
    int rows;
    Color skyTop;
    Color skyBottom;
};

static const LevelDef LEVELS[Level::TOTAL_LEVELS] = {
    { LEVEL_1, (int)(sizeof(LEVEL_1) / sizeof(LEVEL_1[0])), {135, 206, 250, 255}, {200, 230, 255, 255} },
    { LEVEL_2, (int)(sizeof(LEVEL_2) / sizeof(LEVEL_2[0])), {255, 180, 130, 255}, {255, 230, 200, 255} },
    { LEVEL_3, (int)(sizeof(LEVEL_3) / sizeof(LEVEL_3[0])), { 70,  60, 110, 255}, {140, 110, 180, 255} },
};

Level::Level() {}

void Level::Build(int levelNumber) {
    if (levelNumber < 1) levelNumber = 1;
    if (levelNumber > TOTAL_LEVELS) levelNumber = TOTAL_LEVELS;
    currentLevel = levelNumber;

    const LevelDef& def = LEVELS[currentLevel - 1];
    rows = def.rows;
    cols = (int)strlen(def.data[0]);
    tiles.assign(cols * rows, Tile::Empty);
    coins.clear();
    enemySpawns.clear();

    for (int y = 0; y < rows; y++) {
        const char* row = def.data[y];
        int rowLen = (int)strlen(row);
        for (int x = 0; x < cols && x < rowLen; x++) {
            char c = row[x];
            switch (c) {
                case '#': At(x, y) = Tile::Ground;       break;
                case 'B': At(x, y) = Tile::Brick;        break;
                case 'S': At(x, y) = Tile::Spike;        break;
                case '?': At(x, y) = Tile::MysteryBlock; break;
                case '$': At(x, y) = Tile::CoinBlock;    break;
                case 'U': At(x, y) = Tile::UsedBlock;    break;
                case 'G':
                    At(x, y) = Tile::Goal;
                    goalPos = { (float)(x * TILE + TILE / 2), (float)(y * TILE + TILE / 2) };
                    break;
                case 'P':
                    playerSpawn = { (float)(x * TILE), (float)(y * TILE - 32) };
                    break;
                case 'E':
                    enemySpawns.push_back({ (float)(x * TILE), (float)(y * TILE - 26) });
                    break;
                case 'C':
                    coins.push_back({
                        { (float)(x * TILE + TILE / 2), (float)(y * TILE + TILE / 2) },
                        false, 0.0f
                    });
                    break;
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
    float left = camera.target.x - camera.offset.x / camera.zoom;
    float right = left + (float)GetScreenWidth() / camera.zoom;
    int x0 = std::max(0, (int)(left / TILE) - 1);
    int x1 = std::min(cols - 1, (int)(right / TILE) + 1);

    Color groundTop, groundBottom, brickColor, brickLine;
    switch (currentLevel) {
        case 2:
            groundTop    = {180, 110,  60, 255};
            groundBottom = {110,  70,  40, 255};
            brickColor   = {200, 130,  70, 255};
            brickLine    = { 90,  50,  20, 255};
            break;
        case 3:
            groundTop    = { 90,  90, 120, 255};
            groundBottom = { 50,  50,  80, 255};
            brickColor   = {130, 100, 160, 255};
            brickLine    = { 60,  40,  90, 255};
            break;
        default:
            groundTop    = DARKGREEN;
            groundBottom = BROWN;
            brickColor   = ORANGE;
            brickLine    = DARKBROWN;
            break;
    }

    // Pulso para mystery blocks
    float pulse = 0.5f + 0.5f * sinf((float)GetTime() * 3.0f);

    for (int y = 0; y < rows; y++) {
        for (int x = x0; x <= x1; x++) {
            Tile t = At(x, y);
            float px = (float)(x * TILE);
            float py = (float)(y * TILE);

            if (t == Tile::Ground) {
                DrawRectangle((int)px, (int)py, TILE, TILE / 3, groundTop);
                DrawRectangle((int)px, (int)py + TILE / 3, TILE, TILE - TILE / 3, groundBottom);
                DrawRectangleLines((int)px, (int)py, TILE, TILE, {20, 80, 20, 100});
            } else if (t == Tile::Brick) {
                DrawRectangle((int)px, (int)py, TILE, TILE, brickColor);
                DrawLine((int)px, (int)py + TILE / 2, (int)px + TILE, (int)py + TILE / 2, brickLine);
                DrawLine((int)px + TILE / 2, (int)py, (int)px + TILE / 2, (int)py + TILE, brickLine);
                DrawRectangleLines((int)px, (int)py, TILE, TILE, brickLine);
            } else if (t == Tile::MysteryBlock) {
                // Bloque amarillo brillante con un signo de interrogacion
                Color glow = {(unsigned char)(220 + pulse * 35),
                              (unsigned char)(170 + pulse * 50),
                              50, 255};
                DrawRectangle((int)px, (int)py, TILE, TILE, glow);
                DrawRectangleLines((int)px, (int)py, TILE, TILE, {120, 80, 0, 255});
                // Dibujar un "?"
                int fs = 22;
                int tw = MeasureText("?", fs);
                DrawText("?", (int)(px + (TILE - tw) / 2), (int)(py + (TILE - fs) / 2), fs, BLACK);
                // Brillito esquinas
                DrawRectangle((int)px + 3, (int)py + 3, 4, 4, {255, 255, 200, 200});
            } else if (t == Tile::CoinBlock) {
                // Bloque dorado con un "$"
                DrawRectangle((int)px, (int)py, TILE, TILE, {230, 180, 60, 255});
                DrawRectangleLines((int)px, (int)py, TILE, TILE, {130, 90, 20, 255});
                int fs = 22;
                int tw = MeasureText("$", fs);
                DrawText("$", (int)(px + (TILE - tw) / 2), (int)(py + (TILE - fs) / 2), fs, BLACK);
            } else if (t == Tile::UsedBlock) {
                // Bloque marron oscuro (vacio)
                DrawRectangle((int)px, (int)py, TILE, TILE, {110, 80, 50, 255});
                DrawRectangleLines((int)px, (int)py, TILE, TILE, {60, 40, 20, 255});
            } else if (t == Tile::Spike) {
                int spikes = 4;
                float w = (float)TILE / spikes;
                for (int i = 0; i < spikes; i++) {
                    float sx = px + i * w;
                    DrawTriangle(
                        { sx,         py + TILE },
                        { sx + w,     py + TILE },
                        { sx + w / 2, py + 4    },
                        DARKGRAY
                    );
                    DrawTriangleLines(
                        { sx,         py + TILE },
                        { sx + w,     py + TILE },
                        { sx + w / 2, py + 4    },
                        BLACK
                    );
                }
            } else if (t == Tile::Goal) {
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
        float scale = 0.7f + 0.3f * fabsf(cosf(coin.animTime * 3.0f));
        DrawEllipse((int)coin.pos.x, (int)(coin.pos.y + bob), 8.0f * scale, 8.0f, GOLD);
        DrawEllipse((int)coin.pos.x, (int)(coin.pos.y + bob), 5.0f * scale, 5.0f, YELLOW);
    }
}

bool Level::IsSolid(int tx, int ty) const {
    if (tx < 0 || tx >= cols || ty < 0 || ty >= rows) return false;
    Tile t = At(tx, ty);
    return t == Tile::Ground || t == Tile::Brick ||
           t == Tile::MysteryBlock || t == Tile::CoinBlock ||
           t == Tile::UsedBlock;
}

bool Level::RectIntersectsSolid(Rectangle r) const {
    int x0 = (int)(r.x / TILE);
    int y0 = (int)(r.y / TILE);
    int x1 = (int)((r.x + r.width  - 1) / TILE);
    int y1 = (int)((r.y + r.height - 1) / TILE);
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++)
            if (IsSolid(x, y)) return true;
    return false;
}

bool Level::RectIntersectsSpikes(Rectangle r) const {
    int x0 = (int)(r.x / TILE);
    int y0 = (int)(r.y / TILE);
    int x1 = (int)((r.x + r.width  - 1) / TILE);
    int y1 = (int)((r.y + r.height - 1) / TILE);
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            if (x < 0 || x >= cols || y < 0 || y >= rows) continue;
            if (At(x, y) == Tile::Spike) return true;
        }
    }
    return false;
}

BlockDrop Level::HitFromBelow(int tx, int ty) {
    if (tx < 0 || tx >= cols || ty < 0 || ty >= rows) return BlockDrop::None;
    Tile t = At(tx, ty);
    if (t == Tile::MysteryBlock) {
        At(tx, ty) = Tile::UsedBlock;
        // Alternamos: si la columna es par -> Mushroom, impar -> FireFlower
        // Asi siempre hay variedad sin un sistema RNG.
        return (tx % 2 == 0) ? BlockDrop::Mushroom : BlockDrop::FireFlower;
    }
    if (t == Tile::CoinBlock) {
        At(tx, ty) = Tile::UsedBlock;
        return BlockDrop::Coin;
    }
    return BlockDrop::None;
}

void Level::SpawnCoin(Vector2 pos) {
    coins.push_back({ pos, false, 0.0f });
}

Rectangle Level::GoalBounds() const {
    return { goalPos.x - TILE / 2.0f, goalPos.y - TILE / 2.0f, (float)TILE, (float)TILE };
}
