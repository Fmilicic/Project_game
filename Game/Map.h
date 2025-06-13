#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>

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
    static constexpr int width = 50;
    static constexpr int height = 40;
    static constexpr float tileSize = 30.f;

    void regenerate();
    std::vector<sf::Vector2i> findLargestConnectedArea();

    Map();
    Tile* getTile(int x, int y);
    void setStairs(int x, int y);

private:
    std::vector<Tile> tiles;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void generateMazeSkeleton();
    void applyCellularAutomata(int iterations);
    void removeSmallRegions();
    int countWallNeighbors(int x, int y);
};