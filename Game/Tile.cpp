//#include "Tile.h"
//#include "Map.h"
//#include <SFML/Graphics.hpp>
//
//Tile::Tile(int x, int y, float size, bool passable, bool stairs)
//    : passable(passable), isStairs(stairs), x(x), y(y) {
//    shape.setSize({ size, size });
//    shape.setPosition({ x * size, y * size });
//    if (stairs)
//        shape.setFillColor(sf::Color::Yellow);
//    else if (passable)
//        shape.setFillColor(sf::Color(200, 200, 200));
//    else
//        shape.setFillColor(sf::Color(100, 100, 100));
//}
//
//sf::FloatRect Tile::getBounds() const {
//    return sf::FloatRect{ shape.getPosition(), shape.getSize() };
//}
