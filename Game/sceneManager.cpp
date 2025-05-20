#include "SceneManager.h"
#include <iostream>
#include <cmath>
#include <algorithm>

// mapScene
void MapScene::handleEvent(const sf::Event& event) {
    // map event handling yon
}

void MapScene::update(float dt, const sf::RenderWindow& window) {
    // map logic yon
}

void MapScene::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    sf::RectangleShape map;
    map.setSize(sf::Vector2f(800.f, 600.f));
    map.setFillColor(sf::Color(30, 60, 90));
    target.draw(map, states);
}

// BHScene
BulletHellScene::BulletHellScene() {
    for (int i = 0; i < 20; ++i) {
        float angle = i * 18.f * 3.14159265f / 180.f;
        sf::Vector2f dir(std::cos(angle) * 100.f, std::sin(angle) * 100.f);
        engine.spawnBullet(sf::Vector2f(400.f, 300.f), dir);
    }
    player.setRadius(10.f);
    player.setFillColor(sf::Color::Cyan);
    player.setOrigin(sf::Vector2f(10.f, 10.f));
    player.setPosition(sf::Vector2f(400.f, 500.f));
    velocity = sf::Vector2f(0.f, 0.f);
}

void BulletHellScene::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) { // fixed
        switch (keyPressed->scancode) {
        case sf::Keyboard::Scan::W: velocity.y = -speed; break;
        case sf::Keyboard::Scan::S: velocity.y = speed; break;
        case sf::Keyboard::Scan::A: velocity.x = -speed; break;
        case sf::Keyboard::Scan::D: velocity.x = speed; break;
        default: break;
     
    }
        }
    else if (const auto* keyPressed = event.getIf<sf::Event::KeyReleased>()) {
        switch (keyPressed->scancode) {
        case sf::Keyboard::Scan::W:
        case sf::Keyboard::Scan::S: velocity.y = 0.f; break;
        case sf::Keyboard::Scan::A:
        case sf::Keyboard::Scan::D: velocity.x = 0.f; break;
        default: break;
        }

    }
}

void BulletHellScene::update(float dt, const sf::RenderWindow& window) {
    engine.update(dt, window );
    sf::Vector2f pos = player.getPosition();
    pos += this->velocity * dt;
    pos.x = std::clamp(pos.x, 0.f, 800.f);
    pos.y = std::clamp(pos.y, 0.f, 600.f);
    player.setPosition(pos);
    sf::FloatRect playerBounds = player.getGlobalBounds();
    for (const auto& bullet : engine.getBullets()) {
        if (player.getGlobalBounds().findIntersection(bullet->getBounds())) {
            std::cout << "Hit!" << std::endl;
            break;
            // replace w/
            // player->takeDamage(bullet->getDamage());
            // bullet should also be removed on hit
        }
    }
}

void BulletHellScene::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(engine, states);
    target.draw(player, states);
}
//sceneManager
SceneManager::SceneManager() {
    switchTo(State::Map);
}

void SceneManager::switchTo(State newState) {
    currentState = newState;
    switch (newState) {
    case State::Map:
        currentScene = std::make_unique<MapScene>();
        break;
    case State::BulletHell:
        currentScene = std::make_unique<BulletHellScene>();
        break;
    }
}

void SceneManager::handleEvent(const sf::Event& event) {
    if (currentScene) currentScene->handleEvent(event);
}

void SceneManager::update(float dt, sf::RenderWindow& window) {
    if (currentScene) currentScene->update(dt, window);
}

void SceneManager::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (currentScene) target.draw(*currentScene, states);
}
