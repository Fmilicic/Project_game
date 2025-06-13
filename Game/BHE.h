#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <memory>
#include "Entity.h"

class Bullet {
public:
    Bullet(const sf::Vector2f& position, const sf::Vector2f& velocity, float radius, std::optional<float> lifetime = std::nullopt);
    void update(float dt);
    void draw(sf::RenderTarget& target) const;
    bool isOffscreen(const sf::Vector2u& windowSize) const;
    bool intersectsCircle(const sf::Vector2f& center, float radius) const;
    bool isExpired() const;
private:
    sf::CircleShape shape;
    sf::Vector2f   velocity;
    std::optional<float> lifetime;
};

class BulletHellEngine : public sf::Drawable {
public:
    struct Pattern {
        float duration;
        float spawnInterval;
        float timer;
        float intervalTimer;
        std::function<void(Player&)> spawnAction;
        std::optional<std::function<void(Pattern&)>> onTurnStart;
    };

    BulletHellEngine();
    void start(Player& playerRef, Enemy& enemyRef);
    void update(float dt, const sf::Vector2u& windowSize, Player& playerRef);
    bool isBattleOver() const;

private:
    // Active patterns for the current attack phase
    std::vector<Pattern> patterns;

    // Pools of all available patterns, built once
    std::vector<Pattern> genericPool;
    std::vector<Pattern> bossPoolHigh; // 100-51% HP
    std::vector<Pattern> bossPoolMid;  // 50-26% HP
    std::vector<Pattern> bossPoolLow;  // 25-0% HP

    std::vector<std::unique_ptr<Bullet>> bullets;
    std::size_t currentPattern = 0;
    Enemy* enemy = nullptr;
    float       noBulletTimer = 0.f;
    const float clearThreshold = 1.f;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void advancePattern();

    // One-time setup to build the pattern pools
    void setupPools();
    // Selects patterns for the current attack phase from the pools
    void pickPatternsForPhase();
    Pattern makeLionSwipesPattern();
    Pattern makeLionRoarPattern();
};



