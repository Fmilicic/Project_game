#pragma once
#include <SFML/Graphics.hpp>

class MapObject : public sf::Drawable {
public:
    enum class BuffType { Health, Attack, Defense };

    MapObject(int gridX, int gridY, float tileSize, BuffType type);

    int getGridX() const { return gridX; }
    int getGridY() const { return gridY; }
    sf::FloatRect getBounds() const;
    BuffType getBuffType() const { return type; }
    bool isUsed() const { return used; }
    void use() { used = true; }

private:
    int gridX, gridY;
    sf::RectangleShape shape;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    bool used = false;
    BuffType type;
};

