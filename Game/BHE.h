#ifndef BULLETHELLENGINE_H
#define BULLETHELLENGINE_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class Player;
class Enemy;

// --- Bullet: always a circle ---
class Bullet {
public:
    Bullet(const sf::Vector2f& position, const sf::Vector2f& velocity, float radius);

    void update(float dt);
    void draw(sf::RenderTarget& target) const;

    bool isOffscreen(const sf::Vector2u& windowSize) const;
    bool intersectsCircle(const sf::Vector2f& center, float radius) const;

    sf::CircleShape shape;
    sf::Vector2f velocity;
};

// --- Bullet Hell Engine ---
class BulletHellEngine : public sf::Drawable {
public:
    BulletHellEngine();

    void start(const sf::Vector2f& playerCenter, float playerRadius, Enemy* enemyRef);
    void update(float dt, const sf::Vector2u& windowSize, const sf::Vector2f& playerCenter, float playerRadius, int& playerHp, int playerDef, int& playerShields);
    bool isBattleOver() const;

    const std::vector<std::unique_ptr<Bullet>>& getBullets() const { return bullets; }

private:
    void spawnBullet();
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::vector<std::unique_ptr<Bullet>> bullets;
    Enemy* enemy = nullptr;
    float spawnTimer = 0.f;
    float noBulletTimer = 0.f;
    const float bulletInterval = 0.3f;
    const float clearThreshold = 4.f;
};

#endif // BULLETHELLENGINE_H
