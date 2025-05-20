#pragma once
#include <SFML/Graphics.hpp>
class Entity{ ///virtual class entity - we'll derive all further entity classes from it
protected:
	virtual void update() = 0;
	virtual ~Entity() = default;
};

class Object: public Entity { // abstract class for map objects: we'll derive chest and key from it.

};

class Character : public Entity { //abstract class for entites that can move and interact
protected:
	int atk;
	int hp;
	int shield;
public:
	virtual void update_state();
	virtual void takeDamage(int dmg);
	virtual void shieldsUp(int shield);
};

class Player : public Character { // class for player character
private:

public:
	void update_state();
	void takeDamage(int dmg);
	void shieldsUp(int shield);


};

class Enemy : public Character {
private:
	// store attack patterns here?
public:
	void update_state();
	void takeDamage(int dmg);
	void shieldsUp(int shield);
};