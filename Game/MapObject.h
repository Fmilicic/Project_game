#ifndef MAPOBJECT_H
#define MAPOBJECT_H

#include <SFML/Graphics.hpp>

class MapObject : public sf::Drawable {
public:
    MapObject(int gridX, int gridY, float tileSize);

    int getGridX() const { return gridX; }
    int getGridY() const { return gridY; }
    sf::Vector2i getGridPos() const { return { gridX, gridY }; }
    bool isPassable() const { return false; }
    sf::FloatRect getBounds() const;

private:
    int gridX, gridY;
    sf::RectangleShape shape;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

#endif // MAPOBJECT_H
