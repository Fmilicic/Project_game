#include "MapObject.h"

// Constructor implementation now matches the header.
MapObject::MapObject(int gridX, int gridY, float tileSize, BuffType type)
    : gridX(gridX), gridY(gridY), type(type)
{
    shape.setSize({ tileSize, tileSize });
    // Position is set safely with constructor arguments.
    shape.setPosition(sf::Vector2f(static_cast<float>(gridX) * tileSize, static_cast<float>(gridY) * tileSize));

    switch (type) {
    case BuffType::Health:  shape.setFillColor(sf::Color(200, 50, 50)); break;
    case BuffType::Attack:  shape.setFillColor(sf::Color(50, 200, 50)); break;
    case BuffType::Defense: shape.setFillColor(sf::Color(50, 50, 200)); break;
    }
}

sf::FloatRect MapObject::getBounds() const {
    return shape.getGlobalBounds();
}

void MapObject::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (used) {
        sf::RectangleShape fadedShape = shape;
        sf::Color c = shape.getFillColor();
        fadedShape.setFillColor(sf::Color(c.r, c.g, c.b, 100));
        target.draw(fadedShape, states);
    }
    else {
        target.draw(shape, states);
    }
}
