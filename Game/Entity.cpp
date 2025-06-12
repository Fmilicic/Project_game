#include "Entity.h"
#include "sceneManager.h"
#include <iostream>
#include <algorithm>
#include <string> 

Entity::Entity() {
    shape.setRadius(15.f);
    shape.setOrigin({ 15.f, 15.f });
    shape.setFillColor(sf::Color::White);
}

void Entity::takeDamage(int damage) {
    if (shields > 0) {
        --shields;
        SceneManager::pushNotification("Shield blocked the hit!"); 
        return;
    }

    int finalDmg = std::max(1, damage - def);
    hp -= finalDmg;
    SceneManager::pushNotification("Took " + std::to_string(finalDmg) + " damage!"); 
    if (hp < 0) {
        hp = 0;
    }
}

void Entity::heal(int amount) {
    hp += amount;
    if (hp > maxHp) {
        hp = maxHp;
    }
}

void Entity::addShields(int amount) {
    shields += amount;
    SceneManager::pushNotification("Gained " + std::to_string(amount) + " shields!");
}

void Entity::reset() {
    hp = maxHp;
    shields = 1;
}

bool Entity::isDead() const {
    return hp <= 0;
}

int Entity::getHp() const { return hp; }
int Entity::getAtk() const { return atk; }
int Entity::getDef() const { return def; }
int Entity::getShields() const { return shields; }
int Entity::getMaxHp() const { return maxHp; }
void Entity::setPosition(const sf::Vector2f& pos) { shape.setPosition(pos); }
sf::Vector2f Entity::getPosition() const { return shape.getPosition(); }
float Entity::getRadius() const { return shape.getRadius(); }
void Entity::draw(sf::RenderTarget& target, sf::RenderStates states) const { target.draw(shape, states); }

Player::Player() {
    maxHp = 100; hp = maxHp;
    atk = 15; def = 8; shields = 1;
    shape.setFillColor(sf::Color::Green);
}

void Player::update(float) { }
void Player::setStats(int hp_, int atk_, int def_, int shields_) {
    maxHp = hp_; hp = hp_;
    atk = atk_; def = def_; shields = shields_;
}

void Player::applyBuff(MapObject::BuffType type) {
    switch (type) {
    case MapObject::BuffType::Health:
        maxHp += 20; heal(20);
        SceneManager::pushNotification("Max HP increased!"); 
        break;
    case MapObject::BuffType::Attack:
        atk += 3;
        SceneManager::pushNotification("Attack increased!");
        break;
    case MapObject::BuffType::Defense:
        def += 2;
        SceneManager::pushNotification("Defense increased!");
        break;
    }
}

Enemy::Enemy(Type t) : type(t) {
    configureStats();
    switch (t) {
    case Enemy::Type::Ghost:
        shape.setFillColor(sf::Color::White); break;
    case Enemy::Type::Basic:
        shape.setFillColor(sf::Color::Red); break;
    case Enemy::Type::Boss:
        shape.setFillColor(sf::Color(139, 0, 0)); break;
    }
}

void Enemy::update(float) { }

void Enemy::setType(Type t) {
    type = t;
    configureStats();
}

Enemy::Type Enemy::getType() const {
    return type;
}

void Enemy::configureStats() {
    switch (type) {
    case Type::Basic: maxHp = 30; atk = 8; def = 2; shields = 0; break;
    case Type::Ghost: maxHp = 50; atk = 12; def = 5; shields = 1; break;
    case Type::Boss: maxHp = 150; atk = 20; def = 10; shields = 2; break;
    }
    hp = maxHp;
}
