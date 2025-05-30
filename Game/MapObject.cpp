#include "MapObject.h"

MapObject::MapObject(int gridX, int gridY, float tileSize)
    : gridX(gridX), gridY(gridY) {
    shape.setSize({ tileSize, tileSize });
    shape.setPosition({ gridX * tileSize, gridY * tileSize });
    shape.setFillColor(sf::Color(100, 100, 255));
}

sf::FloatRect MapObject::getBounds() const {
    return sf::FloatRect{ shape.getPosition(), shape.getSize() };
}

void MapObject::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(shape, states);
}

