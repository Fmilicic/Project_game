#include "BHE.h"

Bullet::Bullet(const sf::Vector2f& pos, const sf::Vector2f& vel, float radius)
    : velocity(vel) {
    shape.setRadius(radius);
    shape.setOrigin(sf::Vector2f{ radius, radius });
    shape.setPosition(pos);
    shape.setFillColor(sf::Color::Red);
}

void Bullet::update(float dt) {
    shape.move(velocity * dt);
}

bool Bullet::isOffscreen(const sf::RenderWindow& window) const {
    sf::Vector2f pos = shape.getPosition();
    sf::Vector2u size = window.getSize();
    return pos.x < 0 || pos.x > size.x || pos.y < 0 || pos.y > size.y;
}

void Bullet::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(shape, states);
}

sf::FloatRect Bullet::getBounds() const {
    return shape.getGlobalBounds();
}

void BulletHellEngine::spawnBullet(const sf::Vector2f& pos, const sf::Vector2f& vel) {
    bullets.push_back(std::make_unique<Bullet>(pos, vel));
}

void BulletHellEngine::update(float dt, const sf::RenderWindow& window) {
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [&](const std::unique_ptr<Bullet>& b) { return b->isOffscreen(window); }), bullets.end());

    for (auto& bullet : bullets) {
        bullet->update(dt);
    }
}

void BulletHellEngine::clear() {
    bullets.clear();
}

void BulletHellEngine::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (const auto& bullet : bullets) {
        target.draw(*bullet, states);
    }
}

const std::vector<std::unique_ptr<Bullet>>& BulletHellEngine::getBullets() const {
    return bullets;
}