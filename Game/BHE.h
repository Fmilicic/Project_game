#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

/// <summary>
/// Base unit of enemy attacks. Based on sf::circleShape
/// </summary>
class Bullet : public sf::Drawable {
public:
    Bullet(const sf::Vector2f& pos, const sf::Vector2f& vel, float radius = 5.f);
    void update(float dt);
    bool isOffscreen(const sf::RenderWindow& window) const;
    sf::FloatRect getBounds() const;
    ~Bullet() = default;
    bool getLand();
    void setLand();

private:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    bool landed = false;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

class BulletHellEngine : public sf::Drawable {
public:
    void spawnBullet(const sf::Vector2f& pos, const sf::Vector2f& vel);
    void update(float dt, const sf::RenderWindow& window);
    void clear();
    const std::vector<std::unique_ptr<Bullet>>& getBullets() const;
private:
    std::vector<std::unique_ptr<Bullet>> bullets;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

