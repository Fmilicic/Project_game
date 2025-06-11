#include "Entity.h"
#include <algorithm>
#include <iostream>

Entity::Entity()
{
    shape.setRadius(15.f);
    shape.setOrigin(sf::Vector2f{ 15.f, 15.f });
    shape.setFillColor(sf::Color::White);
}

void Entity::takeDamage(int damage)
{
    if (shields > 0) {
        --shields;
        std::cout << "Shield blocked the hit!\n";
        return;
    }
    int finalDmg = std::max(1, damage - def);
    hp -= finalDmg;
    std::cout << "Took " << finalDmg << " damage, hp=" << hp << "\n";
    if (hp < 0) hp = 0;
}

void Entity::heal(int amount)
{
    hp += amount;
    if (hp > maxHp) hp = maxHp;
}

void Entity::reset()
{
    hp = maxHp;
    shields = 1;
}

bool Entity::isDead() const { return hp <= 0; }
int  Entity::getHp() const { return hp; }
int  Entity::getAtk() const { return atk; }
int  Entity::getDef() const { return def; }
int  Entity::getShields() const { return shields; }
int Entity::getMaxHp() const { return maxHp; }

void Entity::setPosition(const sf::Vector2f& pos)
{
    shape.setPosition(pos);
}

sf::Vector2f Entity::getPosition() const
{
    return shape.getPosition();
}

float Entity::getRadius() const
{
    return shape.getRadius();
}

void Entity::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(shape, states);
}

Player::Player()
{
    maxHp = 100; hp = maxHp;
    atk = 15; def = 8; shields = 1;
    shape.setFillColor(sf::Color::Green);
}

void Player::update(float) { }

void Player::setStats(int hp_, int atk_, int def_, int shields_)
{
    maxHp = hp_; hp = hp_;
    atk = atk_; def = def_; shields = shields_;
}

Enemy::Enemy(Type t) : type(t)
{
    configureStats();
    shape.setFillColor(sf::Color::Red);
}

void Enemy::update(float) { }

void Enemy::setType(Type t)
{
    type = t;
    configureStats();
}

Enemy::Type Enemy::getType() const { return type; }

void Enemy::configureStats()
{
    switch (type) {
    case Type::Basic: maxHp = 30;  atk = 8;  def = 2;  shields = 0; break;
    case Type::Ghost: maxHp = 50;  atk = 12; def = 5;  shields = 1; break;
    case Type::Boss:  maxHp = 150; atk = 1 /*temporary for testing purposes*/; def = 10; shields = 2; break;
    }
    hp = maxHp;
}
