#include "Entity.h"
#include <iostream>
#include <algorithm>

// ========== ENTITY BASE CLASS ==========

Entity::Entity() {
    shape.setRadius(15.f);
    shape.setFillColor(sf::Color::White);
    shape.setOrigin(sf::Vector2f{ shape.getRadius(), shape.getRadius() });
}

void Entity::takeDamage(int damage) {
    if (shields > 0) {
        shields--;
        std::cout << "Shield blocked the hit!\n";
        return;
    }
    int finalDmg = std::max(1, damage - def);
    hp -= finalDmg;
    std::cout << "Took " << finalDmg << " damage, hp is now " << hp << "\n";
    if (hp < 0) hp = 0;
}

void Entity::reset() {
    hp = maxHp;
    shields = 1;
}

bool Entity::isDead() const { return hp <= 0; }
int Entity::getHp() const { return hp; }
int Entity::getAtk() const { return atk; }
int Entity::getDef() const { return def; }
int Entity::getShields() const { return shields; }

void Entity::setPosition(const sf::Vector2f& pos) { shape.setPosition(pos); }
sf::Vector2f Entity::getPosition() const { return shape.getPosition(); }
sf::CircleShape Entity::getBounds() const { return shape; }

void Entity::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(shape, states);
}

// ========== PLAYER CLASS ==========

Player::Player() {
    maxHp = 100;
    hp = 100;
    atk = 15;
    def = 8;
    shields = 1;
    shape.setFillColor(sf::Color::Green);
}

void Player::update(float) {
    // For now, nothing special
}

void Player::setStats(int newHp, int newAtk, int newDef, int newShields) {
    maxHp = newHp; hp = newHp; atk = newAtk; def = newDef; shields = newShields;
}

// ========== ENEMY CLASS ==========

Enemy::Enemy(Type type) : type(type) {
    configureStats();
    shape.setFillColor(sf::Color::Red);
}

void Enemy::update(float) {
    // Idle or enemy logic here
}

void Enemy::setType(Type newType) {
    type = newType;
    configureStats();
}

Enemy::Type Enemy::getType() const { return type; }

void Enemy::configureStats() {
    switch (type) {
    case Type::Basic: maxHp = 30; atk = 8; def = 2; shields = 0; break;
    case Type::Boss:  maxHp = 150; atk = 20; def = 10; shields = 2; break;
    case Type::Ghost: maxHp = 50; atk = 12; def = 5; shields = 1; break;
    }
    hp = maxHp;
}