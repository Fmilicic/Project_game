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
    // NOTE: Dimensions adjusted for a balance of size and visibility.
    static constexpr int width = 50;
    static constexpr int height = 40;
    static constexpr float tileSize = 24.f;

    void regenerate();
    std::vector<sf::Vector2i> findLargestConnectedArea();

    Map();
    Tile* getTile(int x, int y);
    void setStairs(int x, int y);

private:
    std::vector<Tile> tiles;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    // Helper functions for map generation
    void generateMazeSkeleton();
    void applyCellularAutomata(int iterations);
    void removeSmallRegions();
    int countWallNeighbors(int x, int y);
};