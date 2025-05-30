#ifndef MAP_H
#define MAP_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "Tile.h"

class Map : public sf::Drawable {
public:
    static constexpr int width = 20;
    static constexpr int height = 15;
    static constexpr float tileSize = 40.f;

    Map();

    Tile* getTile(int x, int y);
    Tile* getTileAtPixel(float px, float py);
    void setStairs(int x, int y);
    void setPassable(int x, int y, bool pass);

private:
    std::vector<Tile> tiles;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

#endif // MAP_H
