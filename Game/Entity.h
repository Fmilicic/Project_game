#pragma once

#include <SFML/Graphics.hpp>
#include "MapObject.h"

class Entity : public sf::Drawable {
public:
    Entity();
    virtual ~Entity() = default;
    virtual void update(float dt) = 0;
    virtual void takeDamage(int damage);
    void heal(int amount);
    void addShields(int amount);
    virtual void reset();
    bool isDead() const;
    int getHp() const;
    int getAtk() const;
    int getDef() const;
    int getShields() const;
    int getMaxHp() const;
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;
    float getRadius() const;

protected:
    int maxHp = 100, hp = 100;
    int atk = 10, def = 5, shields = 0;
    sf::CircleShape shape;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

class Player : public Entity {
public:
    Player();
    void update(float dt) override;
    void setStats(int hp, int atk, int def, int shields);
    void applyBuff(MapObject::BuffType type);
};

class Enemy : public Entity {
public:
    enum class Type { Basic, Ghost, Boss };
    Enemy(Type type = Type::Basic);
    void update(float dt) override;
    void setType(Type type);
    Type getType() const;

private:
    void configureStats();
    Type type;
};


