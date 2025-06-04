#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Entity.h"

class Bullet {
public:
    Bullet(const sf::Vector2f& position, const sf::Vector2f& velocity, float radius);
    void update(float dt);
    void draw(sf::RenderTarget& target) const;
    bool isOffscreen(const sf::Vector2u& windowSize) const;
    bool intersectsCircle(const sf::Vector2f& center, float radius) const;
private:
    sf::CircleShape shape;
    sf::Vector2f   velocity;
};

class BulletHellEngine : public sf::Drawable {
public:
    BulletHellEngine();
    void start(Player& playerRef, Enemy& enemyRef);
    void update(float dt, const sf::Vector2u& windowSize, Player& playerRef);
    bool isBattleOver() const;
private:
    void spawnBullet();
    void draw(sf::RenderTarget& t, sf::RenderStates s) const override;

    std::vector<std::unique_ptr<Bullet>> bullets;
    Enemy* enemy = nullptr;
    float spawnTimer = 0.f;
    float noBulletTimer = 0.f;
    const float bulletInterval = 0.3f;
    const float clearThreshold = 4.f;
};


