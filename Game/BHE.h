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

    /// Initialize pattern queue based on enemy type
    void start(Player& playerRef, Enemy& enemyRef);

    /// Update bullets and pattern timers
    void update(float dt, const sf::Vector2u& windowSize, Player& playerRef);

    /// True when all patterns have run and bullets have cleared
    bool isBattleOver() const;

private:
    struct Pattern {
        float            duration;       // total seconds for this pattern
        float            spawnInterval;  // seconds between spawns
        float            timer = 0.f;    // elapsed since pattern start
        float            intervalTimer = 0.f; // time since last spawn
        std::function<void()> spawnAction;    // called every spawnInterval
    };

    std::vector<std::unique_ptr<Bullet>> bullets;
    std::vector<Pattern>                 patterns;
    std::size_t                          currentPattern = 0;
    float                                patternTimer = 0.f;
    float noBulletTimer = 0.0f;
    float clearThreshold = 3.0f;
    Enemy* enemy = nullptr;

    /// Draw all bullets
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    /// Advance to the next pattern
    void advancePattern();

    /// Factory for patterns by enemy type
    void setupPatterns(Enemy::Type type, const sf::Vector2f& origin);
};


