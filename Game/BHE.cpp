#include "BHE.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <vector>

Bullet::Bullet(const sf::Vector2f& pos, const sf::Vector2f& vel, float radius, std::optional<float> life)
    : velocity(vel), lifetime(life)
{
    shape.setRadius(radius);
    shape.setOrigin(sf::Vector2f{ radius, radius });
    shape.setFillColor(sf::Color::Red);
    shape.setPosition(pos);
}

bool Bullet::isExpired() const {
    return lifetime.has_value() && *lifetime <= 0.f;
}

void Bullet::update(float dt) {
    shape.move(velocity * dt);
    if (lifetime) {
        *lifetime -= dt;
    }
}

void Bullet::draw(sf::RenderTarget& target) const {
    target.draw(shape);
}

bool Bullet::isOffscreen(const sf::Vector2u& win) const {
    auto p = shape.getPosition(); float r = shape.getRadius();
    return (p.x + r < 0 || p.y + r < 0 || p.x - r > win.x || p.y - r > win.y);
}

bool Bullet::intersectsCircle(const sf::Vector2f& c, float r) const {
    auto p = shape.getPosition();
    float dx = p.x - c.x, dy = p.y - c.y;
    float dist2 = dx * dx + dy * dy;
    float sum = shape.getRadius() + r;
    return dist2 <= sum * sum;
}

BulletHellEngine::BulletHellEngine() {
    setupPools();
}

void BulletHellEngine::start(Player& playerRef, Enemy& enemyRef) {
    enemy = &enemyRef;
    bullets.clear();
    patterns.clear();
    currentPattern = 0;
    noBulletTimer = 0.f;

    pickPatternsForPhase();
}

void BulletHellEngine::update(float dt, const sf::Vector2u& win, Player& playerRef) {
    if (!enemy || currentPattern >= patterns.size()) {
        if (currentPattern >= patterns.size() && bullets.empty()) {
            noBulletTimer += dt;
        }
    }
    else {
        // active pattern to process
        auto& pat = patterns[currentPattern];

        // check if pattern is one-shot
        if (pat.timer == 0.f && pat.spawnInterval == 0.f) {
            pat.spawnAction(playerRef);
        }

        pat.timer += dt;

        //  interval timer for repeating patterns
        if (pat.spawnInterval > 0.f) {
            pat.intervalTimer += dt;
            if (pat.intervalTimer >= pat.spawnInterval) {
                pat.intervalTimer -= pat.spawnInterval; 
                pat.spawnAction(playerRef);
            }
        }

        if (pat.timer >= pat.duration) {
            advancePattern();
        }
    }

    for (auto& b : bullets) {
        b->update(dt);
    }
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [&](auto& b) {
                if (b->intersectsCircle(playerRef.getPosition(), playerRef.getRadius())) {
                    playerRef.takeDamage(enemy->getAtk());
                    return true;
                }
                return b->isExpired() || b->isOffscreen(win);
            }),
        bullets.end());
}

bool BulletHellEngine::isBattleOver() const {
    return currentPattern >= patterns.size() && noBulletTimer >= clearThreshold;
}

void BulletHellEngine::advancePattern() {
    ++currentPattern;
}

void BulletHellEngine::draw(sf::RenderTarget& target, sf::RenderStates) const {
    for (auto& b : bullets) b->draw(target);
}

void BulletHellEngine::setupPools() {

    static std::mt19937 gen{ std::random_device{}() };
    // GENERIC PATTERN POOL -> basic and ghost
    genericPool.clear();
    srand(time(nullptr));

    // PATTERN 1: Downward Rain
    // bullets from random positions at the top, moving down
    genericPool.emplace_back(Pattern{
        4.f, 0.4f, 0.f, 0.f,
        [this](Player&) mutable {
            std::uniform_real_distribution<float> xDist(50.f, 750.f);
            std::uniform_real_distribution<float> vDist(180.f, 250.f);
            float x = xDist(gen);
            float v = vDist(gen);
            bullets.push_back(std::make_unique<Bullet>(
                sf::Vector2f{x, 0.f}, sf::Vector2f{0.f, v}, 8.f));
        }
        });

    // PATTERN 2: 8-Way Starburst
    // 8 bullets radiating outwards.
    genericPool.emplace_back(Pattern{
        5.f, 1.2f, 0.f, 0.f,
        [this](Player&) {
            const int N = 8;
            const sf::Vector2f pos{ float(rand() % 800), float(rand() % 600) };
            const float speed = 200.f;
            for (int i = 0; i < N; ++i) {
                float angle = i * 2 * 3.14159f / N;
                bullets.push_back(std::make_unique<Bullet>(
                    pos,
                    sf::Vector2f{std::cos(angle) * speed, std::sin(angle) * speed},
                    7.f
                ));
            }
        }
        });

    // PATTERN 3: X-Shape Sweep
    // tetrades of bullets at 45-degree angles
    genericPool.emplace_back(Pattern{
        4.f, 0.6f, 0.f, 0.f,
        [this](Player&) {
            const float speed = 220.f;
            const sf::Vector2f pos{ float(rand() % 800), float(rand() % 600) };
            // Left bullet
            bullets.push_back(std::make_unique<Bullet>(
                pos , sf::Vector2f{-0.707f * speed, 0.707f * speed}, 8.f));
            // Right bullet
            bullets.push_back(std::make_unique<Bullet>(
                pos, sf::Vector2f{0.707f * speed, 0.707f * speed}, 8.f));
            bullets.push_back(std::make_unique<Bullet>(
                pos, sf::Vector2f{ 0.707f * speed, -0.707f * speed }, 8.f));
            bullets.push_back(std::make_unique<Bullet>(
                pos, sf::Vector2f{ -0.707f * speed, -0.707f * speed }, 8.f));
        }
        });

    // PATTERN 4: Horizontal Wall
    // full horizontal line of bullets top->bottom
    genericPool.emplace_back(Pattern{
        3.f, 1.5f, 0.f, 0.f,
        [this](Player) {
            const float speed = 150.f;
            for (int i = 0; i < 10; ++i) {
                bullets.push_back(std::make_unique<Bullet>(
                    sf::Vector2f{float(i * 80 + 40), 0.f}, sf::Vector2f{0.f, speed}, 10.f));
            }
        }
        });

    // PATTERN 5: Random Burst
    // small cluster of bullets with random velocities
    genericPool.emplace_back(Pattern{
        4.f, 0.8f, 0.f, 0.f,
        [this](Player&) mutable {
            std::uniform_real_distribution<float> angleDist(0.f, 2 * 3.14159f);
            std::uniform_real_distribution<float> speedDist(150.f, 250.f);
            const sf::Vector2f pos{ float(rand() % 800), float(rand() % 600) };
            for (int i = 0; i < 5; ++i) {
                float angle = angleDist(gen);
                float speed = speedDist(gen);
                bullets.push_back(std::make_unique<Bullet>(
                    pos,
                    sf::Vector2f{std::cos(angle) * speed, std::sin(angle) * speed},
                    6.f
                ));
            }
        }
        });

    // BOSS PATTERNS
    // Helper lambda to create a circle pattern
    auto createCirclePattern = [this](
        int num_bullets,
        float speed,
        float radius,
        float duration,
        float interval
        ) -> BulletHellEngine::Pattern {
            return BulletHellEngine::Pattern{
                duration,
                interval,
                0.f, 0.f, // timers
                {},       // spawnAction starts empty
                [this, num_bullets, speed, radius](BulletHellEngine::Pattern& self) {
                    // Generate random position
                    static std::mt19937 gen{ std::random_device{}() };
                    std::uniform_real_distribution<float> xDist(50.f, 750.f);
                    std::uniform_real_distribution<float> yDist(50.f, 550.f);
                    const sf::Vector2f randomPos = { xDist(gen), yDist(gen) };

                    self.spawnAction = [this, num_bullets, speed, radius, randomPos](Player&) {
                        for (int i = 0; i < num_bullets; ++i) {
                            float angle = i * 2 * 3.14159f / num_bullets;
                            bullets.push_back(std::make_unique<Bullet>(
                                randomPos,
                                sf::Vector2f{ std::cos(angle) * speed, std::sin(angle) * speed },
                                radius
                            ));
                        }
                    };
                }
            };
        };

    // Helper lambda for fans attack
    auto makeIntersectingFansPattern = [this](
        int num_lines_per_fan,
        float bullet_speed,
        // arc
        float sweep_arc_degrees,
        // speed of sweep
        float sweep_speed_degrees,
        float duration,
        float spawn_interval
        ) -> BulletHellEngine::Pattern {
            float current_sweep_angle = 0.f;
            float sweep_direction = 1.f;

            return BulletHellEngine::Pattern{
                duration,
                spawn_interval,
                0.f, 0.f, // timers
                {}, // spawnAction is defined by initalizer
                // onTurnStart initializer rests for new turn
                [this, num_lines_per_fan, bullet_speed, sweep_arc_degrees, sweep_speed_degrees,
                 current_sweep_angle, sweep_direction](BulletHellEngine::Pattern& self) mutable {
                    current_sweep_angle = 0.f;
                    sweep_direction = 1.f;

                    // define spawnAction
                    self.spawnAction = [this, num_lines_per_fan, bullet_speed, sweep_arc_degrees, sweep_speed_degrees,
                                        current_sweep_angle, sweep_direction](Player&) mutable {
                        const float PI = 3.14159265f;
                        // emitter positions
                        const sf::Vector2f left_emitter_pos = {200.f, 50.f};
                        const sf::Vector2f right_emitter_pos = {600.f, 50.f};
                        const float bullet_radius = 6.f;

                        // update angle
                        current_sweep_angle += sweep_speed_degrees * sweep_direction;

                        // reverse sweep when hits edge
                        float max_sweep_angle = sweep_arc_degrees / 2.f;
                        if (std::abs(current_sweep_angle) >= max_sweep_angle) {
                            sweep_direction *= -1.f;
                        }

                        // Loop to spawn one bullet for each "line" in the fan.
                        for (int i = 0; i < num_lines_per_fan; ++i) {
                            // Calculate angle specific line
                            float line_angle_offset_deg = -max_sweep_angle + (i * (sweep_arc_degrees / (num_lines_per_fan - 1)));
                            float final_angle_deg = 90.f + current_sweep_angle + line_angle_offset_deg;
                            float final_angle_rad = final_angle_deg * PI / 180.f;

                            // Calculate velocity for left fan
                            sf::Vector2f left_velocity = {
                                std::cos(final_angle_rad) * bullet_speed,
                                std::sin(final_angle_rad) * bullet_speed
                            };
                            bullets.push_back(std::make_unique<Bullet>(left_emitter_pos, left_velocity, bullet_radius));

                            // flip x-vel for right fan
                            sf::Vector2f right_velocity = {-left_velocity.x, left_velocity.y};
                            bullets.push_back(std::make_unique<Bullet>(right_emitter_pos, right_velocity, bullet_radius));
                        }
                    };
                }
            };
        };

    // BOSS POLL: High HP (51-100%)
    bossPoolHigh.clear();
    bossPoolHigh.emplace_back(createCirclePattern(16, 230.f, 15.f, 5.5f, 1.f));
    bossPoolHigh.emplace_back(Pattern{ 5.f, 2.f, 0.f, 0.f, genericPool[3].spawnAction });

    // BOSS POOL: Mid HP (50-26%)

    bossPoolMid.clear();
    bossPoolMid.emplace_back(Pattern{ 4.f, 0.3f, 0.f, 0.f, genericPool[0].spawnAction });
    float spiralAngle = 0.f;
    bossPoolMid.emplace_back(Pattern{
        10.0f,
        0.1f,
        0.f, 0.f,
        [this, spiralAngle](Player& /*player*/) mutable {
            const sf::Vector2f screenCenter = {400.f, 300.f};

            // spiral settings
            const float radialSpeed = 50.f;     // how fast spiral expands outwards
            const float tangentialSpeed = 200.f;  // How fast it spins
            const float angleIncrement = 0.15f;   // How much to rotate between each bullet
            const float bulletRadius = 7.f;

            // Calculate components of the bullet velocity
            sf::Vector2f radialDirection = { std::cos(spiralAngle), std::sin(spiralAngle) };
            // For clockwise, tangential direction is perpendicular
            sf::Vector2f tangentialDirection = { -radialDirection.y, radialDirection.x };

            // velocity is combination of moving outwards and sideways
            sf::Vector2f finalVelocity = (radialDirection * radialSpeed) + (tangentialDirection * tangentialSpeed);

            // Spawn bullet with calculated velocity
            bullets.push_back(std::make_unique<Bullet>(screenCenter, finalVelocity, bulletRadius));
            bullets.push_back(std::make_unique<Bullet>(screenCenter, -finalVelocity, bulletRadius));
            // Increment angle for next bullet
            spiralAngle += angleIncrement;
        }
        });
    bossPoolMid.emplace_back(
        makeIntersectingFansPattern(
            /*num_lines_per_fan*/   4,
            /*bullet_speed*/        250.f,
            /*sweep_arc_degrees*/   60.f,
            /*sweep_speed_degrees*/ 1.5f,
            /*duration*/            7.f,
            /*spawn_interval*/      0.1f
        )
    );
    // BOSS POOL: Low HP (25-0%) - Very fast and large bullets
    bossPoolLow.clear();
    bossPoolLow.emplace_back(
        makeIntersectingFansPattern(
            /*num_lines_per_fan*/   6,
            /*bullet_speed*/        320.f,
            /*sweep_arc_degrees*/   75.f,
            /*sweep_speed_degrees*/ 2.0f,
            /*duration*/            8.f,
            /*spawn_interval*/      0.08f
        )
    );
    bossPoolLow.emplace_back(Pattern{ 4.f, 0.2f, 0.f, 0.f, genericPool[0].spawnAction });
    bossPoolLow.emplace_back(Pattern{
        4.0f, 2.0f, 0.f, 0.f,
        // coordinated volley from all screen edges
        [this](Player& player) {
            const int bullets_per_edge = 5;
            const float bullet_speed = 260.f;
            const float bullet_radius = 12.f;
            // player's position at moment of spawning
            const sf::Vector2f target_pos = player.getPosition();

            const float screen_width = 800.f;
            const float screen_height = 600.f;

            std::vector<sf::Vector2f> spawn_points;
            spawn_points.reserve(bullets_per_edge * 4);

            // spawn points evenly along edges
            for (int i = 0; i < bullets_per_edge; ++i) {
                float interp = float(i) / (bullets_per_edge - 1);
                spawn_points.emplace_back(interp * screen_width, 0.f); // Top
                spawn_points.emplace_back(interp * screen_width, screen_height); // Bottom
                spawn_points.emplace_back(0.f, interp * screen_height); // Left
                spawn_points.emplace_back(screen_width, interp * screen_height); // Right
            }

            // For each spawn point create a bullet aimed at target 
            for (const auto& spawn_pos : spawn_points) {
                sf::Vector2f direction = target_pos - spawn_pos;
                float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                sf::Vector2f velocity = (distance > 0)
                    ? (direction / distance) * bullet_speed
                    : sf::Vector2f(0.f, bullet_speed);

                bullets.push_back(std::make_unique<Bullet>(spawn_pos, velocity, bullet_radius));
            }
        }
        });
    bossPoolLow.emplace_back(makeLionSwipesPattern());
    bossPoolLow.emplace_back(makeLionRoarPattern());
}

void BulletHellEngine::pickPatternsForPhase() {
    static std::mt19937 gen{ std::random_device{}() };

    patterns.clear();

    if (enemy->getType() != Enemy::Type::Boss) {
        //  pick 2 random patterns from the pool
        std::sample(
            genericPool.begin(), genericPool.end(),
            std::back_inserter(patterns),
            2,
            gen
        );
    }
    else {
        // Boss: select a pool based on current HP
        float hpPercent = float(enemy->getHp()) / float(enemy->getMaxHp());
        if (hpPercent > 0.5f) {
            patterns = bossPoolHigh;
        }
        else if (hpPercent > 0.25f) {
            patterns = bossPoolMid;
        }
        else {
            patterns = bossPoolLow;
        }
    }


    for (auto& pat : patterns) {
        pat.timer = 0.f;
        pat.intervalTimer = 0.f;

        // If pattern has onTurnStart initializer, call it 
        if (pat.onTurnStart) {
            // Call initializer function, pass pattern reference
            (*pat.onTurnStart)(pat);
        }
    }
}

BulletHellEngine::Pattern BulletHellEngine::makeLionSwipesPattern() {
    return BulletHellEngine::Pattern{
        7.0f, 0.05f, 0.f, 0.f, {},
        [this](BulletHellEngine::Pattern& self) {
            int swipes_fired = 0;
            float swipe_timer = 0.f;

            self.spawnAction = [this, swipes_fired, swipe_timer](Player& player) mutable {
                const int num_swipes = 3;
                const float time_between_swipes = 2.0f;
                const float silhouette_lifetime = 0.1f;
                const float swipe_bullet_speed = 320.f;
                const float scatter_degrees = 15.f;

                const float screen_width = 800.f, screen_height = 600.f;
                const float lion_area_width = screen_width * 0.25f;
                const float lion_area_offset_x = screen_width - lion_area_width;
                const float lion_area_height = lion_area_width * 1.3f;
                const float lion_area_offset_y = (screen_height - lion_area_height) / 2.f;

                const std::vector<sf::Vector2f> lion_art_points = {
                    {0.90f, 0.15f}, {0.85f, 0.12f}, {0.80f, 0.10f}, {0.75f, 0.08f}, {0.68f, 0.06f}, {0.60f, 0.05f},
                    {0.50f, 0.06f}, {0.40f, 0.08f}, {0.30f, 0.12f}, {0.22f, 0.20f}, {0.15f, 0.30f},
                    {0.10f, 0.40f}, {0.08f, 0.50f}, {0.10f, 0.60f}, {0.15f, 0.70f}, {0.22f, 0.80f}, {0.30f, 0.88f},
                    {0.40f, 0.94f}, {0.50f, 0.98f}, {0.60f, 1.0f},
                    {0.70f, 0.96f}, {0.78f, 0.90f}, {0.85f, 0.82f}, {0.90f, 0.75f},
                    {0.85f, 0.68f}, {0.90f, 0.65f},
                    {0.98f, 0.63f}, {1.00f, 0.58f}, {0.97f, 0.53f}, {0.92f, 0.50f},
                    {0.85f, 0.45f}, {0.78f, 0.42f}, {0.72f, 0.45f}, {0.78f, 0.50f},
                    {0.70f, 0.25f}, {0.60f, 0.20f}, {0.50f, 0.18f}, {0.40f, 0.22f}, {0.30f, 0.35f},
                    {0.25f, 0.50f}, {0.30f, 0.65f}, {0.40f, 0.78f}, {0.55f, 0.85f}, {0.70f, 0.80f},
                    {0.80f, 0.35f}, {0.70f, 0.55f}, {0.55f, 0.35f}, {0.45f, 0.55f}, {0.65f, 0.65f}
                };

                for (const auto& p : lion_art_points) {
                    sf::Vector2f spawn_pos(lion_area_offset_x + p.x * lion_area_width, lion_area_offset_y + p.y * lion_area_height);
                    bullets.push_back(std::make_unique<Bullet>(spawn_pos, sf::Vector2f{0.f, 0.f}, 3.5f, silhouette_lifetime));
                }

                swipe_timer += 0.05f;
                if (swipe_timer >= time_between_swipes && swipes_fired < num_swipes) {
                    swipes_fired++;
                    swipe_timer = 0.f;
                    static std::mt19937 gen{ std::random_device{}() };
                    const sf::Vector2f target_pos = player.getPosition();

                    for (const auto& p : lion_art_points) {
                        sf::Vector2f spawn_pos(lion_area_offset_x + p.x * lion_area_width, lion_area_offset_y + p.y * lion_area_height);
                        sf::Vector2f direction = target_pos - spawn_pos;
                        float base_angle_rad = std::atan2(direction.y, direction.x);

                        for (int i = 0; i < 2; ++i) {
                            std::uniform_real_distribution<float> scatter(-scatter_degrees, scatter_degrees);
                            float final_angle = base_angle_rad + (scatter(gen) * 3.14159f / 180.f);
                            sf::Vector2f velocity(std::cos(final_angle) * swipe_bullet_speed, std::sin(final_angle) * swipe_bullet_speed);
                            bullets.push_back(std::make_unique<Bullet>(spawn_pos, velocity, 7.f));
                        }
                    }
                }
            };
        }
    };
}

BulletHellEngine::Pattern BulletHellEngine::makeLionRoarPattern() {
    return BulletHellEngine::Pattern{
        6.0f, 0.05f, 0.f, 0.f, {},
        [this](BulletHellEngine::Pattern& self) {
            float roar_timer = 0.f;

            self.spawnAction = [this, roar_timer](Player&) mutable {
                const float time_between_roar_lines = 0.25f;
                const float roar_bullet_speed = 450.f;

                const float screen_width = 800.f, screen_height = 600.f;
                const float lion_area_width = screen_width * 0.25f;
                const float lion_area_offset_x = screen_width - lion_area_width;
                const float lion_area_height = lion_area_width * 1.3f;
                const float lion_area_offset_y = (screen_height - lion_area_height) / 2.f;

                const std::vector<sf::Vector2f> lion_art_open = {
                    {0.90f, 0.15f}, {0.85f, 0.12f}, {0.80f, 0.10f}, {0.75f, 0.08f}, {0.68f, 0.06f}, {0.60f, 0.05f},
                    {0.50f, 0.06f}, {0.40f, 0.08f}, {0.30f, 0.12f}, {0.22f, 0.20f}, {0.15f, 0.30f},
                    {0.10f, 0.40f}, {0.08f, 0.50f},
                    {0.98f, 0.63f}, {1.00f, 0.58f}, {0.97f, 0.53f}, {0.92f, 0.50f},
                    {0.85f, 0.45f}, {0.78f, 0.42f}, {0.72f, 0.45f}, {0.78f, 0.50f},
                    {0.70f, 0.25f}, {0.60f, 0.20f}, {0.50f, 0.18f}, {0.40f, 0.22f}, {0.30f, 0.35f},
                    {0.80f, 0.35f},
                    {0.60f, 1.0f}, {0.70f, 0.96f}
                };

                for (const auto& p : lion_art_open) {
                    sf::Vector2f spawn_pos(lion_area_offset_x + p.x * lion_area_width, lion_area_offset_y + p.y * lion_area_height);
                    bullets.push_back(std::make_unique<Bullet>(spawn_pos, sf::Vector2f{0.f, 0.f}, 3.5f, 0.1f));
                }

                roar_timer += 0.05f;
                if (roar_timer >= time_between_roar_lines) {
                    roar_timer = 0.f;

                    float roar_start_y = lion_area_offset_y + (0.7f * lion_area_height);
                    float roar_start_x = lion_area_offset_x + (0.8f * lion_area_width);
                    for (int i = 0; i < 6; ++i) {
                        float y_pos = roar_start_y + i * 12.f;
                        bullets.push_back(std::make_unique<Bullet>(
                            sf::Vector2f(roar_start_x, y_pos),
                            sf::Vector2f(-roar_bullet_speed, 0.f), 9.f
                        ));
                    }
                }
            };
        }
    };
}

