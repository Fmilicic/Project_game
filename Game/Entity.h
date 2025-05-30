#ifndef ENTITY_H
#define ENTITY_H

#include <SFML/Graphics.hpp>

// --- Abstract Base Class ---
class Entity : public sf::Drawable {
public:
    Entity();
    virtual ~Entity() = default;
    virtual void update(float dt) = 0;
    virtual void takeDamage(int damage);
    virtual void reset();
    bool isDead() const;
    int getHp() const;
    int getAtk() const;
    int getDef() const;
    int getShields() const;
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const;
    sf::CircleShape getBounds() const;
protected:
    int maxHp = 100;
    int hp = 100;
    int atk = 10;
    int def = 5;
    int shields = 0;
    sf::CircleShape shape;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

// --- Player Class ---
class Player : public Entity {
public:
    Player();
    void update(float dt) override;
    void setStats(int hp, int atk, int def, int shields);
};

// --- Enemy Class ---
class Enemy : public Entity {
public:
    enum class Type { Basic, Boss, Ghost };
    Enemy(Type type = Type::Basic);
    void update(float dt) override;
    void setType(Type type);
    Type getType() const;
private:
    Type type;
    void configureStats();
};

#endif // ENTITY_H