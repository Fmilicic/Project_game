#include "Map.h"

Map::Map()
{
    tiles.reserve(width * height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            tiles.emplace_back(x, y, tileSize, true, false);
        }
    }
    for (int y = 5; y < 10; ++y)
        setPassable(10, y, false);
}

Tile* Map::getTile(int x, int y)
{
    if (x < 0 || y < 0 || x >= width || y >= height)
        return nullptr;
    return &tiles[y * width + x];
}

Tile* Map::getTileAtPixel(float px, float py)
{
    int x = static_cast<int>(px / tileSize);
    int y = static_cast<int>(py / tileSize);
    return getTile(x, y);
}

void Map::setStairs(int x, int y)
{
    if (auto* t = getTile(x, y)) {
        t->isStairs = true;
        t->shape.setFillColor(sf::Color::Yellow);
    }
}

void Map::setPassable(int x, int y, bool pass)
{
    if (auto* t = getTile(x, y)) {
        t->passable = pass;
        t->shape.setFillColor(pass
            ? sf::Color(200, 200, 200)
            : sf::Color(100, 100, 100));
    }
}

void Map::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    for (const auto& t : tiles)
        target.draw(t.shape, states);
}
