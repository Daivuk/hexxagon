#include <onut.h>

static const auto hexOffsetX = 0.5f;
static const auto hexOffsetY = 0.86602540378443864676372317075294f;
static const auto hexSize = 32.f;

enum class Player
{
    None,
    Player1,
    Player2
};

enum class TileType
{
    None,
    Hexagon
};

struct Tile
{
    TileType tileType;
    Player player;
};

static const std::vector<Tile> startingLayout =
{
    { TileType::Hexagon, Player::Player2 },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::Player1 },
    { TileType::None, Player::None },
    { TileType::None, Player::None },
    { TileType::None, Player::None },
    { TileType::None, Player::None },

    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::None, Player::None },
    { TileType::None, Player::None },
    { TileType::None, Player::None },

    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::None, Player::None },
    { TileType::None, Player::None },

    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::None, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::None, Player::None },

    { TileType::Hexagon, Player::Player1 },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::None, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::Player2 },

    { TileType::None, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::None, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },

    { TileType::None, Player::None },
    { TileType::None, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },

    { TileType::None, Player::None },
    { TileType::None, Player::None },
    { TileType::None, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },

    { TileType::None, Player::None },
    { TileType::None, Player::None },
    { TileType::None, Player::None },
    { TileType::None, Player::None },
    { TileType::Hexagon, Player::Player2 },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::None },
    { TileType::Hexagon, Player::Player1 }
};

struct Board
{
    Board(const std::vector<Tile>& layout) : tiles(layout) {}
    Board(const Board& other) : tiles(other.tiles) {}

    std::vector<Tile> tiles;
};

struct PossiblePlay
{
    POINT from;
    std::vector<POINT> directs;
    std::vector<POINT> jumps;
};

void init();
void render();
void update();

struct ThinkResult
{
    POINT from;
    POINT to;
    bool found;
    bool direct;
} thinkResult;
std::atomic<bool> cpuThinking = false;
Player turn = Player::Player1;
std::vector<Board> history;
Board board(startingLayout);
static const POINT invalidTile{ -1, -1 };
POINT mouseHover = invalidTile;
POINT selected = invalidTile;
bool operator==(const POINT& a, const POINT& b)
{
    return a.x == b.x && a.y == b.y;
}
bool operator!=(const POINT& a, const POINT& b)
{
    return !(a == b);
}
PossiblePlay possiblePlay;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	ORun(init, update, render);
	return 0;
}

void init()
{
}

PossiblePlay getPossiblePlay(const Board& inBoard, const POINT& tilePos)
{
    auto addTile = [](const Board& inBoard, std::vector<POINT>& outVec, POINT tilePos)
    {
        if (tilePos.x < 0 || tilePos.x >= 9 ||
            tilePos.y < 0 || tilePos.y >= 9) return;
        auto& tile = board.tiles[tilePos.y * 9 + tilePos.x];
        if (tile.tileType == TileType::None) return;
        if (tile.player != Player::None) return;
        outVec.push_back(tilePos);
    };

    PossiblePlay ret;
    ret.from = tilePos;

    if (tilePos == invalidTile)
    {
        return std::move(ret);
    }

    addTile(inBoard, ret.directs, { tilePos.x - 1, tilePos.y });
    addTile(inBoard, ret.directs, { tilePos.x - 1, tilePos.y - 1 });
    addTile(inBoard, ret.directs, { tilePos.x, tilePos.y - 1 });
    addTile(inBoard, ret.directs, { tilePos.x + 1, tilePos.y });
    addTile(inBoard, ret.directs, { tilePos.x + 1, tilePos.y + 1 });
    addTile(inBoard, ret.directs, { tilePos.x, tilePos.y + 1 });

    addTile(inBoard, ret.jumps, { tilePos.x - 2, tilePos.y });
    addTile(inBoard, ret.jumps, { tilePos.x - 2, tilePos.y - 1 });
    addTile(inBoard, ret.jumps, { tilePos.x - 2, tilePos.y - 2 });
    addTile(inBoard, ret.jumps, { tilePos.x - 1, tilePos.y - 2 });
    addTile(inBoard, ret.jumps, { tilePos.x, tilePos.y - 2 });
    addTile(inBoard, ret.jumps, { tilePos.x + 1, tilePos.y - 1 });
    addTile(inBoard, ret.jumps, { tilePos.x + 2, tilePos.y });
    addTile(inBoard, ret.jumps, { tilePos.x + 2, tilePos.y + 1 });
    addTile(inBoard, ret.jumps, { tilePos.x + 2, tilePos.y + 2 });
    addTile(inBoard, ret.jumps, { tilePos.x + 1, tilePos.y + 2 });
    addTile(inBoard, ret.jumps, { tilePos.x, tilePos.y + 2 });
    addTile(inBoard, ret.jumps, { tilePos.x - 1, tilePos.y + 1 });

    return std::move(ret);
}

std::vector<PossiblePlay> getPossiblePlays(const Board& inBoard, Player player)
{
    std::vector<PossiblePlay> possiblePlays;
    int k = 0;
    for (auto& tile : inBoard.tiles)
    {
        if (tile.player == player && tile.tileType != TileType::None)
        {
            possiblePlays.push_back(getPossiblePlay(inBoard, { k % 9, k / 9 }));
        }
        ++k;
    }
    return std::move(possiblePlays);
}

Tile& getTile(Board& inBoard, const POINT& pos)
{
    auto k = pos.y * 9 + pos.x;
    return inBoard.tiles[k];
}

void transformNeighbors(Board& inBoard, const POINT& tilePos, Player player)
{
    auto transformTile = [](Board& inBoard, const POINT& tilePos, Player player)
    {
        if (tilePos.x < 0 || tilePos.x >= 9 ||
            tilePos.y < 0 || tilePos.y >= 9) return;
        auto& tile = inBoard.tiles[tilePos.y * 9 + tilePos.x];
        if (tile.tileType == TileType::None) return;
        if (tile.player == Player::None) return;
        tile.player = player;
    };

    transformTile(inBoard, { tilePos.x - 1, tilePos.y }, player);
    transformTile(inBoard, { tilePos.x - 1, tilePos.y - 1 }, player);
    transformTile(inBoard, { tilePos.x, tilePos.y - 1 }, player);
    transformTile(inBoard, { tilePos.x + 1, tilePos.y }, player);
    transformTile(inBoard, { tilePos.x + 1, tilePos.y + 1 }, player);
    transformTile(inBoard, { tilePos.x, tilePos.y + 1 }, player);
}

int thinkMove(Board inBoard, POINT from, POINT to, bool direct)
{
    // Do the move
    auto& tileFrom = getTile(inBoard, from);
    auto& tileTo = getTile(inBoard, to);
    if (!direct) tileFrom.player = Player::None;
    tileTo.player = Player::Player2;
    transformNeighbors(inBoard, to, Player::Player2);

    // Now do all possible moves by player1 and determine his best score
    auto possiblePlays = getPossiblePlays(inBoard, Player::Player1);
    int bestOponentScore = -10000;
    for (auto& possiblePlay : possiblePlays)
    {
        for (auto& direct : possiblePlay.directs)
        {
            auto boardCopy = inBoard;
            auto& tileFrom = getTile(boardCopy, possiblePlay.from);
            auto& tileTo = getTile(boardCopy, direct);
            int score = 0;
            for (auto& tile : boardCopy.tiles)
            {
                if (tile.player == Player::Player1) ++score;
                else if (tile.player == Player::Player2) --score;
            }
            bestOponentScore = std::max<>(bestOponentScore, score);
        }
        for (auto& jump : possiblePlay.jumps)
        {
            auto boardCopy = inBoard;
            auto& tileFrom = getTile(boardCopy, possiblePlay.from);
            auto& tileTo = getTile(boardCopy, jump);
            int score = 0;
            for (auto& tile : boardCopy.tiles)
            {
                if (tile.player == Player::Player1) ++score;
                else if (tile.player == Player::Player2) --score;
            }
            bestOponentScore = std::max<>(bestOponentScore, score);
        }
    }

    // Score is my tiles - his tiles
    int score = -bestOponentScore;
    for (auto& tile : inBoard.tiles)
    {
        if (tile.player == Player::Player2) ++score;
    }

    return score;
}

void think(Board inBoard)
{
    auto possiblePlays = getPossiblePlays(inBoard, Player::Player2);
    struct Play
    {
        POINT from;
        POINT to;
        bool direct;
        std::shared_future<int> score;
    };
    std::vector<Play> plays;
    for (auto& possiblePlay : possiblePlays)
    {
        for (auto& direct : possiblePlay.directs)
        {
            plays.push_back(Play());
            auto& play = plays.back();
            play.from = possiblePlay.from;
            play.to = direct;
            play.direct = true;
            play.score = std::async(std::launch::async, thinkMove, inBoard, possiblePlay.from, direct, true).share();
        }
        for (auto& jump : possiblePlay.jumps)
        {
            plays.push_back(Play());
            auto& play = plays.back();
            play.from = possiblePlay.from;
            play.to = jump;
            play.direct = false;
            play.score = std::async(std::launch::async, thinkMove, inBoard, possiblePlay.from, jump, false).share();
        }
    }
    int bestScore = -10000;
    for (auto& play : plays)
    {
        auto score = play.score.get();
        if (score > bestScore)
        {
            bestScore = score;
        }
    }
    for (auto it = plays.begin(); it != plays.end(); )
    {
        if (it->score.get() < bestScore)
        {
            it = plays.erase(it);
            continue;
        }
        ++it;
    }
    if (!plays.empty())
    {
        auto& play = onut::randv(plays);
        thinkResult.from = play.from;
        thinkResult.to = play.to;
        thinkResult.direct = play.direct;
        thinkResult.found = true;
    }
    else
    {
        thinkResult.found = false;
    }
}

void startThinking()
{
    thinkResult.found = false;
    cpuThinking = true;
    std::async(std::launch::async, []
    {
        think(board);
        cpuThinking = false;
    });
}

void update()
{
    mouseHover = invalidTile;

    Matrix screenFit = Matrix::Identity;
    screenFit *= Matrix::CreateTranslation(OScreenCenterf);
    float closest = 1000.f;

    auto isMouseHover = [&](const Vector2& center, float size) -> bool
    {
        auto transformed = Vector2::Transform(center, screenFit);
        float dis = Vector2::DistanceSquared(OInput->mousePosf, transformed);
        if (dis > closest) return false;
        if (dis > size * size) return false;
        closest = dis;
        return true;
    };

    if (turn == Player::Player1)
    {
        for (int j = 0; j < 9; ++j)
        {
            for (int i = 0; i < 9; ++i)
            {
                auto k = j * 9 + i;
                auto& tile = board.tiles[k];
                if (tile.tileType == TileType::None) continue;
                Vector2 pos(
                    (float)((i - 4) - (j - 4)) * (hexOffsetX * 3.f * hexSize),
                    (float)((i - 4) + (j - 4)) * (hexOffsetY * hexSize));
                if (isMouseHover(pos, hexSize))
                {
                    mouseHover = { i, j };
                }
            }
        }

        if (OJustPressed(OINPUT_MOUSEB1))
        {
            if (mouseHover != invalidTile)
            {
                auto& tile = getTile(board, mouseHover);
                if (tile.player == Player::Player1)
                {
                    selected = mouseHover;
                    possiblePlay = getPossiblePlay(board, mouseHover);
                }
                else
                {
                    for (auto& direct : possiblePlay.directs)
                    {
                        if (mouseHover == direct)
                        {
                            // Duplicate!
                            history.push_back(board);
                            tile.player = Player::Player1;
                            selected = invalidTile;
                            possiblePlay = getPossiblePlay(board, selected);
                            transformNeighbors(board, mouseHover, Player::Player1);
                            turn = Player::Player2;
                            startThinking();
                            break;
                        }
                    }
                    if (selected != invalidTile)
                    {
                        for (auto& direct : possiblePlay.jumps)
                        {
                            if (mouseHover == direct)
                            {
                                // Duplicate!
                                history.push_back(board);
                                auto& selectedTile = getTile(board, selected);
                                tile.player = Player::Player1;
                                selectedTile.player = Player::None;
                                selected = invalidTile;
                                possiblePlay = getPossiblePlay(board, selected);
                                transformNeighbors(board, mouseHover, Player::Player1);
                                turn = Player::Player2;
                                startThinking();
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                selected = mouseHover;
                possiblePlay = getPossiblePlay(board, mouseHover);
            }
        }
    }
    else
    {
        if (!cpuThinking)
        {
            assert(thinkResult.found);
            if (thinkResult.found)
            {
                history.push_back(board);
                auto& tileFrom = getTile(board, thinkResult.from);
                auto& tileTo = getTile(board, thinkResult.to);
                if (!thinkResult.direct) tileFrom.player = Player::None;
                tileTo.player = Player::Player2;
                transformNeighbors(board, thinkResult.to, Player::Player2);
            }
            turn = Player::Player1;
        }
    }
}

void render()
{
	ORenderer->clear(Color::Black);

	Matrix screenFit = Matrix::Identity;
	screenFit *= Matrix::CreateTranslation(OScreenCenterf);
    auto drawHex = [](const Vector2& center, float size, const Color& color)
    {
        OPB->draw(Vector2(center.x - size, center.y), color);
        OPB->draw(Vector2(center.x - size * hexOffsetX, center.y - size * hexOffsetY), color);
        OPB->draw(center, color);

        OPB->draw(Vector2(center.x - size * hexOffsetX, center.y - size * hexOffsetY), color);
        OPB->draw(Vector2(center.x + size * hexOffsetX, center.y - size * hexOffsetY), color);
        OPB->draw(center, color);

        OPB->draw(Vector2(center.x + size * hexOffsetX, center.y - size * hexOffsetY), color);
        OPB->draw(Vector2(center.x + size, center.y), color);
        OPB->draw(center, color);

        OPB->draw(Vector2(center.x + size, center.y), color);
        OPB->draw(Vector2(center.x + size * hexOffsetX, center.y + size * hexOffsetY), color);
        OPB->draw(center, color);

        OPB->draw(Vector2(center.x + size * hexOffsetX, center.y + size * hexOffsetY), color);
        OPB->draw(Vector2(center.x - size * hexOffsetX, center.y + size * hexOffsetY), color);
        OPB->draw(center, color);

        OPB->draw(Vector2(center.x - size * hexOffsetX, center.y + size * hexOffsetY), color);
        OPB->draw(Vector2(center.x - size, center.y), color);
        OPB->draw(center, color);
    };

    auto drawHexLines = [](const Vector2& center, float size, const Color& color)
    {
        OPB->draw(Vector2(center.x - size, center.y), color);
        OPB->draw(Vector2(center.x - size * hexOffsetX, center.y - size * hexOffsetY), color);

        OPB->draw(Vector2(center.x - size * hexOffsetX, center.y - size * hexOffsetY), color);
        OPB->draw(Vector2(center.x + size * hexOffsetX, center.y - size * hexOffsetY), color);

        OPB->draw(Vector2(center.x + size * hexOffsetX, center.y - size * hexOffsetY), color);
        OPB->draw(Vector2(center.x + size, center.y), color);

        OPB->draw(Vector2(center.x + size, center.y), color);
        OPB->draw(Vector2(center.x + size * hexOffsetX, center.y + size * hexOffsetY), color);

        OPB->draw(Vector2(center.x + size * hexOffsetX, center.y + size * hexOffsetY), color);
        OPB->draw(Vector2(center.x - size * hexOffsetX, center.y + size * hexOffsetY), color);

        OPB->draw(Vector2(center.x - size * hexOffsetX, center.y + size * hexOffsetY), color);
        OPB->draw(Vector2(center.x - size, center.y), color);
    };

    OPB->begin(onut::ePrimitiveType::TRIANGLES, nullptr, screenFit);
    for (int j = 0; j < 9; ++j)
    {
        for (int i = 0; i < 9; ++i)
        {
            auto k = j * 9 + i;
            auto& tile = board.tiles[k];
            if (tile.tileType == TileType::None) continue;
            Vector2 pos(
                (float)((i - 4) - (j - 4)) * (hexOffsetX * 3.f * hexSize),
                (float)((i - 4) + (j - 4)) * (hexOffsetY * hexSize));
            drawHex(pos, hexSize, Color(.15f, .15f, .15f, 1));
            if (turn == Player::Player1)
            {
                if (selected == POINT{ i, j })
                {
                    drawHex(pos, hexSize * .95f, Color(0, 1, 0, 1));
                }
                if (mouseHover == POINT{ i, j })
                {
                    drawHex(pos, hexSize * .9f, Color(.35f, .35f, .35f, 1) * 1.25f);
                }
                else
                {
                    drawHex(pos, hexSize * .9f, Color(.35f, .35f, .35f, 1));
                }
            }
            else
            {
                drawHex(pos, hexSize * .9f, Color(.35f, .35f, .35f, 1));
            }
            if (tile.player == Player::Player1)
            {
                drawHex(pos, hexSize * .6f, Color(1, 0, 0, 1));
            }
            else if (tile.player == Player::Player2)
            {
                drawHex(pos, hexSize * .6f, Color(0, 1, 1, 1));
            }
        }
    }
	OPB->end();

    if (turn == Player::Player1)
    {
        OPB->begin(onut::ePrimitiveType::LINES, nullptr, screenFit);
        for (auto& direct : possiblePlay.directs)
        {
            Vector2 pos(
                (float)((direct.x - 4) - (direct.y - 4)) * (hexOffsetX * 3.f * hexSize),
                (float)((direct.x - 4) + (direct.y - 4)) * (hexOffsetY * hexSize));
            drawHexLines(pos, hexSize * .6f, Color(0, 1, 0, 1));
        }
        for (auto& direct : possiblePlay.jumps)
        {
            Vector2 pos(
                (float)((direct.x - 4) - (direct.y - 4)) * (hexOffsetX * 3.f * hexSize),
                (float)((direct.x - 4) + (direct.y - 4)) * (hexOffsetY * hexSize));
            drawHexLines(pos, hexSize * .6f, Color(1, 1, 0, 1));
        }
        OPB->end();
    }

    if (cpuThinking)
    {
        OSB->begin();
        OSB->drawRect(nullptr, { 0, 0, 20, 20 }, Color(1, 1, 0, 1));
        OSB->end();
    }
}
