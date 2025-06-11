#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
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

    /// True when all patterns have run and bullets have cleared
    bool isBattleOver() const;

private:
    struct Pattern {
        float            duration;       // pattern duration
        float            spawnInterval;  // wave interval
        float            timer = 0.f;    // time since start
        float            intervalTimer = 0.f; // time since last spawn
        std::function<void()> spawnAction;    // called every spawnInterval
    };

    std::vector<std::unique_ptr<Bullet>> bullets;
    std::vector<Pattern>                 patterns;
    std::size_t                          currentPattern = 0;
    float                                patternTimer = 0.f;
    float noBulletTimer = 0.0f;
    float clearThreshold = 2.0f;
    Enemy* enemy = nullptr;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void advancePattern();

    void setupPatterns(Enemy::Type type, const sf::Vector2f& origin);
};


