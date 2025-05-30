#include "BHE.h"
#include "Entity.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <iostream>

// -------- Bullet Implementation --------

Bullet::Bullet(const sf::Vector2f& position, const sf::Vector2f& velocity, float radius)
    : velocity(velocity) {
    shape.setRadius(radius);
    shape.setOrigin(sf::Vector2f{ radius, radius });
    shape.setFillColor(sf::Color::Red);
    shape.setPosition(position);
}

void Bullet::update(float dt) {
    shape.move(velocity * dt);
}

void Bullet::draw(sf::RenderTarget& target) const {
    target.draw(shape);
}

bool Bullet::isOffscreen(const sf::Vector2u& windowSize) const {
    const auto& pos = shape.getPosition();
    float r = shape.getRadius();
    return (pos.x + r < 0 || pos.y + r < 0 || pos.x - r > windowSize.x || pos.y - r > windowSize.y);
}

bool Bullet::intersectsCircle(const sf::Vector2f& center, float radius) const {
    sf::Vector2f bulletCenter = shape.getPosition();
    float dx = bulletCenter.x - center.x;
    float dy = bulletCenter.y - center.y;
    float distSq = dx * dx + dy * dy;
    float radSum = shape.getRadius() + radius;
    return distSq <= radSum * radSum;
}

// -------- BulletHellEngine Implementation --------

BulletHellEngine::BulletHellEngine() = default;

void BulletHellEngine::start(const sf::Vector2f& /*playerCenter*/, float /*playerRadius*/, Enemy* enemyRef) {
    enemy = enemyRef;
    bullets.clear();
    spawnTimer = 0.f;
    noBulletTimer = 0.f;
}

void BulletHellEngine::update(
    float dt,
    const sf::Vector2u& windowSize,
    const sf::Vector2f& playerCenter,
    float playerRadius,
    int& playerHp,
    int playerDef,
    int& playerShields
) {
    if (!enemy) return;

    spawnTimer += dt;
    if (spawnTimer >= bulletInterval) {
        spawnTimer = 0.f;
        spawnBullet();
    }

    // Update bullets
    for (auto& b : bullets) {
        b->update(dt);
    }

    // Remove bullets that hit the player or are offscreen
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [&](const std::unique_ptr<Bullet>& b) {
                if (b->intersectsCircle(playerCenter, playerRadius)) {
                    std::cout << "Hit!" << std::endl;
                    // Damage logic here (mimic Entity::takeDamage)
                    int damage = enemy->getAtk();
                    if (playerShields > 0) {
                        playerShields--;
                        std::cout << "Shield blocked the hit!\n";
                    }
                    else {
                        int finalDmg = std::max(1, damage - playerDef);
                        playerHp -= finalDmg;
                        if (playerHp < 0) playerHp = 0;
                        std::cout << "Took " << finalDmg << " damage, hp is now " << playerHp << "\n";
                        
                    }
                    return true; // Remove on hit
                }
                return b->isOffscreen(windowSize);
            }),
        bullets.end()
    );

    // Timer to track if screen is clear
    if (bullets.empty())
        noBulletTimer += dt;
    else
        noBulletTimer = 0.f;
}

void BulletHellEngine::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (const auto& b : bullets)
        b->draw(target);
}

bool BulletHellEngine::isBattleOver() const {
    return noBulletTimer >= clearThreshold;
}

void BulletHellEngine::spawnBullet() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(20.f, 780.f);
    std::uniform_real_distribution<float> vDist(150.f, 250.f);

    sf::Vector2f pos(xDist(gen), 0.f);
    sf::Vector2f velocity(0.f, vDist(gen));
    bullets.push_back(std::make_unique<Bullet>(pos, velocity, 8.f));
}
