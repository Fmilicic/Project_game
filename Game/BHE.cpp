#include "BHE.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <iostream>

// -------- Bullet Implementation --------

Bullet::Bullet(const sf::Vector2f& pos, const sf::Vector2f& vel, float radius)
    : velocity(vel)
{
    shape.setRadius(radius);
    shape.setOrigin(sf::Vector2f{ radius, radius });
    shape.setFillColor(sf::Color::Red);
    shape.setPosition(pos);
}

void Bullet::update(float dt) {
    shape.move(velocity * dt);
}

void Bullet::draw(sf::RenderTarget& target) const {
    target.draw(shape);
}

bool Bullet::isOffscreen(const sf::Vector2u& win) const {
    auto p = shape.getPosition();
    float r = shape.getRadius();
    return p.x + r < 0 || p.y + r < 0 || p.x - r > win.x || p.y - r > win.y;
}

bool Bullet::intersectsCircle(const sf::Vector2f& c, float r) const {
    auto p = shape.getPosition();
    float dx = p.x - c.x, dy = p.y - c.y;
    float dist2 = dx * dx + dy * dy, sum = shape.getRadius() + r;
    return dist2 <= sum * sum;
}

// -------- BulletHellEngine Implementation --------

BulletHellEngine::BulletHellEngine() = default;

void BulletHellEngine::start(Player& playerRef, Enemy& enemyRef) {
    enemy = &enemyRef;
    bullets.clear();
    patterns.clear();
    currentPattern = 0;
    patternTimer = 0.f;

    // Enemy origin for spawning
    sf::Vector2f origin = enemy->getPosition();

    // Set up patterns based on enemy type
    setupPatterns(enemy->getType(), origin);
}

void BulletHellEngine::update(float dt,
    const sf::Vector2u& win, Player& playerRef)
{
    if (!enemy) return;

    // ① Only run a pattern if one remains
    if (currentPattern < patterns.size())
    {
        auto& pat = patterns[currentPattern];
        pat.timer += dt;
        pat.intervalTimer += dt;

        if (pat.intervalTimer >= pat.spawnInterval) {
            pat.intervalTimer -= pat.spawnInterval;
            pat.spawnAction();
        }
        if (pat.timer >= pat.duration) {
            advancePattern();
        }
    }

    // ② Move bullets
    for (auto& b : bullets) b->update(dt);

    // ③ Collision and off-screen removal
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [&](auto& b) {
                if (b->intersectsCircle(playerRef.getPosition(),
                    playerRef.getRadius()))
                {
                    playerRef.takeDamage(enemy->getAtk());
                    return true;
                }
                return b->isOffscreen(win);
            }),
        bullets.end());

    // ④ Once *all* patterns are done, wait for bullets to clear
    if (currentPattern >= patterns.size() && bullets.empty())
        noBulletTimer += dt;
}



bool BulletHellEngine::isBattleOver() const
{
    return currentPattern >= patterns.size()
        && noBulletTimer >= clearThreshold;
}

void BulletHellEngine::advancePattern() {
    patternTimer = 0.f;
    ++currentPattern;
    if (currentPattern < patterns.size()) {
        // reset timers for the next pattern
        patterns[currentPattern].timer = 0.f;
        patterns[currentPattern].intervalTimer = 0.f;
    }
}

void BulletHellEngine::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (const auto& b : bullets)
        b->draw(target);
}

void BulletHellEngine::setupPatterns(Enemy::Type type, const sf::Vector2f& origin)
{
    // static RNG with static storage: no capture needed
    static std::mt19937 gen{ std::random_device{}() };

    // Local distributions
    std::uniform_real_distribution<float> xDist(50.f, 750.f);
    std::uniform_real_distribution<float> speedDist(150.f, 300.f);

    if (type == Enemy::Type::Basic) {
        // single‐bullet pattern for 4s, every 0.5s
        patterns.emplace_back(Pattern{
            /*duration*/      4.f,
            /*spawnInterval*/ 0.5f,
            /*timer*/         0.f,
            /*intervalTimer*/ 0.f,
            /*spawnAction*/   [this, xDist, speedDist]() mutable {
                float x = xDist(gen);                // gen is static
                float v = speedDist(gen);
                bullets.push_back(std::make_unique<Bullet>(
                    sf::Vector2f{x, 0.f},
                    sf::Vector2f{0.f, v},
                    8.f
                ));
            }
            });
    }
    else if (type == Enemy::Type::Ghost) {
        // star‐burst for 5s, every 1s
        patterns.emplace_back(Pattern{
            5.f, 1.f, 0.f, 0.f,
            [this, origin]() {
                const int N = 6;
                float speed = 200.f;
                for (int i = 0; i < N; ++i) {
                    float angle = i * 2 * 3.14159f / N;
                    bullets.push_back(std::make_unique<Bullet>(
                        origin,
                        sf::Vector2f{std::cos(angle) * speed, std::sin(angle) * speed},
                        6.f
                    ));
                }
            }
            });
        // downward shots for 3s, every 0.4s
        patterns.emplace_back(Pattern{
            3.f, 0.4f, 0.f, 0.f,
            [this, xDist, speedDist]() mutable {
                float x = xDist(gen);
                float v = speedDist(gen);
                bullets.push_back(std::make_unique<Bullet>(
                    sf::Vector2f{x, 0.f},
                    sf::Vector2f{0.f, v},
                    6.f
                ));
            }
            });
    }
    else if (type == Enemy::Type::Boss) {
        // full circle for 6s, every 1.5s
        patterns.emplace_back(Pattern{
            6.f, 1.5f, 0.f, 0.f,
            [this, origin]() {
                const int N = 12;
                float speed = 250.f;
                for (int i = 0; i < N; ++i) {
                    float angle = i * 2 * 3.14159f / N;
                    bullets.push_back(std::make_unique<Bullet>(
                        origin,
                        sf::Vector2f{std::cos(angle) * speed, std::sin(angle) * speed},
                        10.f
                    ));
                }
            }
            });
        // star burst for 5s, every 1s
        patterns.emplace_back(Pattern{
            5.f, 1.f, 0.f, 0.f,
            [this, origin]() {
                const int N = 6;
                float speed = 220.f;
                for (int i = 0; i < N; ++i) {
                    float angle = i * 2 * 3.14159f / N;
                    bullets.push_back(std::make_unique<Bullet>(
                        origin,
                        sf::Vector2f{std::cos(angle) * speed, std::sin(angle) * speed},
                        8.f
                    ));
                }
            }
            });
        // downward blitz for 4s, every 0.2s
        patterns.emplace_back(Pattern{
            4.f, 0.2f, 0.f, 0.f,
            [this, xDist]() mutable {
                float x = xDist(gen);
                bullets.push_back(std::make_unique<Bullet>(
                    sf::Vector2f{x, 0.f},
                    sf::Vector2f{0.f, 300.f},
                    12.f
                ));
            }
            });
        // rapid salvo for 3s, every 0.1s
        patterns.emplace_back(Pattern{
            3.f, 0.1f, 0.f, 0.f,
            [this, xDist]() mutable {
                float x = xDist(gen);
                bullets.push_back(std::make_unique<Bullet>(
                    sf::Vector2f{x, 0.f},
                    sf::Vector2f{0.f, 400.f},
                    14.f
                ));
            }
            });
    }
}
