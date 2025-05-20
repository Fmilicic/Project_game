#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "BHE.h"

class GameScene : public sf::Drawable {
public:
    virtual ~GameScene() = default;
    virtual void handleEvent(const sf::Event &event) = 0;
    virtual void update(float dt, const sf::RenderWindow& window) = 0;
};

class MapScene : public GameScene {
public:
    void handleEvent(const sf::Event &event) override;
    void update(float dt, const sf::RenderWindow& window) override;
private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

class BulletHellScene : public GameScene {
public:
    BulletHellScene();
    void handleEvent(const sf::Event &event) override;
    void update(float dt, const sf::RenderWindow& window) override;
private:
    BulletHellEngine engine;
    sf::CircleShape player;
    sf::Vector2f velocity;
    float speed = 200.f;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

class SceneManager: public sf::Drawable {
public:
    enum class State { Map, BulletHell };

    SceneManager();
    void switchTo(State newState);
    void handleEvent(const sf::Event& event);
    void update(float dt, sf::RenderWindow& window);
    

private:
    State currentState;
    std::unique_ptr<GameScene> currentScene;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
