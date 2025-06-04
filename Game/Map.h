#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct Tile {
    sf::RectangleShape shape;
    bool passable;
    bool isStairs;
    int x, y;

    Tile(int x, int y, float size, bool passable = true, bool stairs = false);
    sf::FloatRect getBounds() const;
};

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
