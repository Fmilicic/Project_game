#include "Map.h"
#include <random>
#include <stack>
#include <numeric>
#include <algorithm>
#include <queue>

// Unchanged from original file
Tile::Tile(int x, int y, float tileSize, bool isPassable, bool isStairsFlag)
    : x(x), y(y), passable(isPassable), isStairs(isStairsFlag) {
    shape.setSize({ tileSize, tileSize });
    shape.setPosition(sf::Vector2f(static_cast<float>(x) * tileSize, static_cast<float>(y) * tileSize));
}

// Unchanged from original file
Map::Map() {
    tiles.reserve(width * height);
    regenerate();
}

// Replaces the old, buggy global countAliveNeighbours function
int Map::countWallNeighbors(int x, int y) {
    int count = 0;
    for (int ny = y - 1; ny <= y + 1; ++ny) {
        for (int nx = x - 1; nx <= x + 1; ++nx) {
            if (nx == x && ny == y) continue;
            if (nx < 0 || nx >= width || ny < 0 || ny >= height || !tiles[ny * width + nx].passable) {
                count++;
            }
        }
    }
    return count;
}

void Map::generateMazeSkeleton() {
    tiles.clear();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            tiles.emplace_back(x, y, tileSize, false, false); // All walls
        }
    }

    std::vector<bool> visited(width * height, false);
    std::stack<sf::Vector2i> stack;
    std::mt19937 gen(std::random_device{}());

    sf::Vector2i startPos(1, 1);
    stack.push(startPos);
    getTile(startPos.x, startPos.y)->passable = true;
    visited[startPos.y * width + startPos.x] = true;

    int dx[] = { 0, 0, 2, -2 }, dy[] = { 2, -2, 0, 0 };

    while (!stack.empty()) {
        sf::Vector2i current = stack.top();
        std::vector<sf::Vector2i> neighbors;
        for (int i = 0; i < 4; ++i) {
            sf::Vector2i next(current.x + dx[i], current.y + dy[i]);
            if (next.x > 0 && next.x < width - 1 && next.y > 0 && next.y < height - 1 && !visited[next.y * width + next.x]) {
                neighbors.push_back(next);
            }
        }

        if (!neighbors.empty()) {
            std::uniform_int_distribution<> distrib(0, neighbors.size() - 1);
            sf::Vector2i chosen = neighbors[distrib(gen)];
            sf::Vector2i wallToRemove(current.x + (chosen.x - current.x) / 2, current.y + (chosen.y - current.y) / 2);
            getTile(wallToRemove.x, wallToRemove.y)->passable = true;
            getTile(chosen.x, chosen.y)->passable = true;
            visited[chosen.y * width + chosen.x] = true;
            stack.push(chosen);
        }
        else {
            stack.pop();
        }
    }
}

void Map::applyCellularAutomata(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        std::vector<Tile> newTiles = tiles;
        for (int y = 1; y < height - 1; ++y) {
            for (int x = 1; x < width - 1; ++x) {
                int wallNeighbors = countWallNeighbors(x, y);
                int index = y * width + x;
                if (!tiles[index].passable && wallNeighbors <= 3) {
                    newTiles[index].passable = true;
                }
                else if (tiles[index].passable && wallNeighbors >= 5) {
                    newTiles[index].passable = false;
                }
            }
        }
        tiles = newTiles;
    }
}

void Map::removeSmallRegions() {
    std::vector<bool> visited(width * height, false);
    std::vector<std::vector<int>> regions;

    for (int i = 0; i < width * height; ++i) {
        if (tiles[i].passable && !visited[i]) {
            std::vector<int> currentRegion;
            std::stack<int> stack;
            stack.push(i);
            visited[i] = true;
            while (!stack.empty()) {
                int currentIndex = stack.top();
                stack.pop();
                currentRegion.push_back(currentIndex);
                int x = currentIndex % width, y = currentIndex / width;
                int dx[] = { 0, 0, 1, -1 }, dy[] = { 1, -1, 0, 0 };
                for (int j = 0; j < 4; ++j) {
                    int nx = x + dx[j], ny = y + dy[j];
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height && tiles[ny * width + nx].passable && !visited[ny * width + nx]) {
                        visited[ny * width + nx] = true;
                        stack.push(ny * width + nx);
                    }
                }
            }
            regions.push_back(currentRegion);
        }
    }

    if (regions.size() <= 1) return;

    auto largestRegionIt = std::max_element(regions.begin(), regions.end(), [](const auto& a, const auto& b) {
        return a.size() < b.size();
        });

    for (const auto& region : regions) {
        if (&region != &(*largestRegionIt)) {
            for (int tileIndex : region) {
                tiles[tileIndex].passable = false;
            }
        }
    }
}


// The new map generation function
void Map::regenerate() {
    generateMazeSkeleton();
    applyCellularAutomata(4);
    removeSmallRegions();

    for (auto& tile : tiles) {
        if (tile.isStairs) {
            tile.shape.setFillColor(sf::Color::Yellow);
        }
        else if (!tile.passable) {
            tile.shape.setFillColor(sf::Color(100, 100, 100));
        }
        else {
            tile.shape.setFillColor(sf::Color(200, 200, 200));
        }
    }
}

// --- All other functions from the original file remain unchanged ---

std::vector<sf::Vector2i> Map::findLargestConnectedArea() {
    std::vector<bool> visited(width * height, false);
    std::vector<sf::Vector2i> largestArea;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (tiles[y * width + x].passable && !visited[y * width + x]) {
                std::vector<sf::Vector2i> currentArea;
                std::queue<sf::Vector2i> q;
                q.push({ x, y });
                visited[y * width + x] = true;
                while (!q.empty()) {
                    sf::Vector2i tile = q.front();
                    q.pop();
                    currentArea.push_back(tile);
                    int dx[] = { 0, 0, 1, -1 }, dy[] = { 1, -1, 0, 0 };
                    for (int i = 0; i < 4; ++i) {
                        int nx = tile.x + dx[i], ny = tile.y + dy[i];
                        if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
                            tiles[ny * width + nx].passable && !visited[ny * width + nx]) {
                            visited[ny * width + nx] = true;
                            q.push({ nx, ny });
                        }
                    }
                }
                if (currentArea.size() > largestArea.size()) {
                    largestArea = currentArea;
                }
            }
        }
    }
    return largestArea;
}

Tile* Map::getTile(int x, int y) {
    if (x < 0 || y < 0 || x >= width || y >= height) return nullptr;
    return &tiles[y * width + x];
}

void Map::setStairs(int x, int y) {
    if (auto* t = getTile(x, y)) {
        t->isStairs = true;
        t->shape.setFillColor(sf::Color::Yellow);
    }
}

void Map::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    // Get the current view from the render target
    sf::View view = target.getView();
    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();

    // Calculate the boundaries of the view in world coordinates
    float left = center.x - size.x / 2.f;
    float right = center.x + size.x / 2.f;
    float top = center.y - size.y / 2.f;
    float bottom = center.y + size.y / 2.f;

    // Convert world coordinates to tile indices, with a small buffer
    int startX = std::max(0, static_cast<int>(left / tileSize) - 1);
    int endX = std::min(width, static_cast<int>(right / tileSize) + 1);
    int startY = std::max(0, static_cast<int>(top / tileSize) - 1);
    int endY = std::min(height, static_cast<int>(bottom / tileSize) + 1);

    // Draw only the tiles that are visible within the calculated range
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            target.draw(tiles[y * width + x].shape, states);
        }
    }
}

// Tile::getBounds() from original file
sf::FloatRect Tile::getBounds() const {
    return shape.getGlobalBounds();
}

