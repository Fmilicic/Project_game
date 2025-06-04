#include "BHE.h"
#include "Entity.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <iostream>

// -------- Bullet Implementation --------

Bullet::Bullet(const sf::Vector2f& position, const sf::Vector2f& velocity, float radius)
    : velocity(velocity)
{
    shape.setRadius(radius);
    shape.setOrigin(sf::Vector2f{ radius, radius });
    shape.setFillColor(sf::Color::Red);
    shape.setPosition(position);
}

void Bullet::update(float dt)
{
    shape.move(velocity * dt);
}

void Bullet::draw(sf::RenderTarget& target) const
{
    target.draw(shape);
}

bool Bullet::isOffscreen(const sf::Vector2u& windowSize) const
{
    const auto& pos = shape.getPosition();
    float r = shape.getRadius();
    return (pos.x + r < 0 ||
        pos.y + r < 0 ||
        pos.x - r > windowSize.x ||
        pos.y - r > windowSize.y);
}

bool Bullet::intersectsCircle(const sf::Vector2f& center, float radius) const
{
    sf::Vector2f bulletCenter = shape.getPosition();
    float dx = bulletCenter.x - center.x;
    float dy = bulletCenter.y - center.y;
    float distSq = dx * dx + dy * dy;
    float radSum = shape.getRadius() + radius;
    return distSq <= radSum * radSum;
}

// -------- BulletHellEngine Implementation --------

BulletHellEngine::BulletHellEngine() = default;

void BulletHellEngine::start(Player& playerRef, Enemy& enemyRef)
{
    enemy = &enemyRef;
    bullets.clear();
    spawnTimer = 0.f;
    noBulletTimer = 0.f;
}

void BulletHellEngine::update(float dt,
    const sf::Vector2u& win, Player& playerRef)
{
    if (!enemy) return;

    // 
    spawnTimer += dt;
    if (spawnTimer >= bulletInterval) {
        spawnTimer = 0.f;
        spawnBullet();
    }

    // 
    for (auto& b : bullets) b->update(dt);

    // 
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [&](auto& b) {
                if (b->intersectsCircle(playerRef.getPosition(),
                    playerRef.getRadius())) {
                    playerRef.takeDamage(enemy->getAtk());
                    return true;
                }
                return b->isOffscreen(win);
            }),
        bullets.end()
    );

    // 
    if (bullets.empty()) noBulletTimer += dt;
    else                  noBulletTimer = 0.f;
}

bool BulletHellEngine::isBattleOver() const
{
    return noBulletTimer >= clearThreshold;
}

void BulletHellEngine::spawnBullet()
{
    static std::mt19937 gen{ std::random_device{}() };
    std::uniform_real_distribution<float> xDist(50.f, 750.f);
    std::uniform_real_distribution<float> vDist(100.f, 300.f);

    // 
    switch (enemy->getType())
    {
    case Enemy::Type::Basic:
    {
        // 
        float x = xDist(gen);
        bullets.push_back(std::make_unique<Bullet>(
            sf::Vector2f{ x, 0.f },
            sf::Vector2f{ 0.f, vDist(gen) },
            8.f));
    }
    break;

    case Enemy::Type::Ghost:
    {
        // 
        sf::Vector2f origin{ 400.f, 0.f };
        int N = 6;
        for (int i = 0; i < N; ++i) {
            float a = (2 * 3.14159f / N) * i;
            sf::Vector2f v{ std::cos(a) * vDist(gen),
                            std::sin(a) * vDist(gen) };
            bullets.push_back(std::make_unique<Bullet>(origin, v, 6.f));
        }
    }
    break;

    case Enemy::Type::Boss:
    {
        // 
        sf::Vector2f origin = enemy->getPosition();
        int N = 12;
        for (int i = 0; i < N; ++i) {
            float a = (2 * 3.14159f / N) * i;
            sf::Vector2f v{ std::cos(a) * 200.f,
                            std::sin(a) * 200.f };
            bullets.push_back(std::make_unique<Bullet>(origin, v, 10.f));
        }
        //
        if (enemy->getHp() < enemy->getMaxHp() / 2) {
            float x = xDist(gen);
            bullets.push_back(std::make_unique<Bullet>(
                sf::Vector2f{ x, 0.f },
                sf::Vector2f{ 0.f, vDist(gen) },
                12.f));
        }
    }
    break;
    }
}

void BulletHellEngine::draw(sf::RenderTarget& target,
    sf::RenderStates states) const
{
    for (auto& b : bullets) b->draw(target);
}
