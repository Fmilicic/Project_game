#include "sceneManager.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <Windows.h>

//MapScene Implementation
MapScene::MapScene(Player& player, std::vector<Enemy>& enemies)
//procedural generation later on?
    : player(player), enemies(enemies), map() {
    objects.emplace_back(5, 5, Map::tileSize);
    objects.emplace_back(7, 8, Map::tileSize);
    map.setStairs(Map::width - 1, Map::height - 1);
    player.setPosition({ Map::tileSize / 2, Map::tileSize / 2 });
    if (!enemies.empty())
        enemies[0].setPosition({ 9 * Map::tileSize + Map::tileSize / 2, 7 * Map::tileSize + Map::tileSize / 2 });
    enemies[0].setType(Enemy::Type::Boss);
}

void MapScene::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        int dx = 0, dy = 0;
        switch (key->scancode) {
        case sf::Keyboard::Scan::W: dy = -1; break;
        case sf::Keyboard::Scan::S: dy = 1; break;
        case sf::Keyboard::Scan::A: dx = -1; break;
        case sf::Keyboard::Scan::D: dx = 1; break;
        default: return;
        }
        sf::Vector2f pos = player.getPosition();
        int px = static_cast<int>(pos.x / Map::tileSize);
        int py = static_cast<int>(pos.y / Map::tileSize);
        int nx = px + dx, ny = py + dy;

        // check whether we can into tile
        Tile* target = map.getTile(nx, ny);
        if (!target || !target->passable) return;

        // check obj
        for (const auto& obj : objects) {
            if (obj.getGridX() == nx && obj.getGridY() == ny) {
                std::cout << "Interacted with object at " << nx << "," << ny << "\n";
                return;
            }
        }

        // check enemy
        for (auto& enemy : enemies) {
            if (!enemy.isDead()) {
                int ex = static_cast<int>(enemy.getPosition().x / Map::tileSize);
                int ey = static_cast<int>(enemy.getPosition().y / Map::tileSize);
                if (ex == nx && ey == ny) {
                    battleEnemy = &enemy;
                    return;
                }
            }
        }

        // move
        player.setPosition({ nx * Map::tileSize + Map::tileSize / 2, ny * Map::tileSize + Map::tileSize / 2 });

        // win cond
        if (target->isStairs && !triggeredBattle()) {
            atStairs = true;
            std::cout << "You win!\n";
        }
    }
}

void MapScene::update(float, const sf::RenderWindow&) {}

void MapScene::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(map, states);
    for (const auto& obj : objects)
        target.draw(obj, states);
    target.draw(player, states);
    for (const auto& enemy : enemies)
        if (!enemy.isDead())
            target.draw(enemy, states);
}

//BulletHellScene
BulletHellScene::BulletHellScene(Player& player, Enemy& enemy)
    : player(player), enemy(enemy) {
    playerShape.setRadius(15.f);
    playerShape.setOrigin(sf::Vector2f{ 15.f, 15.f });
    playerShape.setFillColor(sf::Color::Green);
    playerShape.setPosition(sf::Vector2f{ 400.f, 500.f });
    velocity = sf::Vector2f(0.f, 0.f);
    playerHpCache = player.getHp();
    playerShieldsCache = player.getShields();
    engine.start(playerShape.getPosition(), playerShape.getRadius(), &enemy);
}

void BulletHellScene::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        switch (keyPressed->scancode) {
        case sf::Keyboard::Scan::W: velocity.y = -speed; break;
        case sf::Keyboard::Scan::S: velocity.y = speed; break;
        case sf::Keyboard::Scan::A: velocity.x = -speed; break;
        case sf::Keyboard::Scan::D: velocity.x = speed; break;
        default: break;
        }
    }
    if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>()) {
        switch (keyReleased->scancode) {
        case sf::Keyboard::Scan::W:
        case sf::Keyboard::Scan::S: velocity.y = 0.f; break;
        case sf::Keyboard::Scan::A:
        case sf::Keyboard::Scan::D: velocity.x = 0.f; break;
        default: break;
        }
    }
}

void BulletHellScene::update(float dt, const sf::RenderWindow& window) {
    sf::Vector2f pos = playerShape.getPosition();
    pos += velocity * dt;
    pos.x = std::clamp(pos.x, 0.f, 800.f);
    pos.y = std::clamp(pos.y, 0.f, 600.f);
    playerShape.setPosition(pos);

    // Pass hitbox
    engine.update(
        dt, window.getSize(),
        playerShape.getPosition(), playerShape.getRadius(),
        player, // pass the actual player object
        player.getDef()
    );

    // Sync back to player entity after battle
    if (player.isDead()) {
        sceneOver = true;
    }
    if (engine.isBattleOver()) {
        sceneOver = true;
    }


}

void BulletHellScene::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(engine, states);
    target.draw(playerShape, states);
}

bool BulletHellScene::isOver() const { return sceneOver; }

// --- SceneManager Implementation ---
SceneManager::SceneManager() : currentState(State::Map), player(), enemies() {
    enemies.emplace_back(Enemy::Type::Basic);
    switchTo(State::Map);
}

void SceneManager::switchTo(State newState, Enemy* battleEnemy) {
    currentState = newState;
    if (newState == State::Map) {
        currentScene = std::make_unique<MapScene>(player, enemies);
    }
    else if (newState == State::BulletHell && battleEnemy) {
        currentScene = std::make_unique<BulletHellScene>(player, *battleEnemy);
    }
    if (newState == State::GameOver) {
        currentScene = std::make_unique<GameOverScene>(player);
    }
}

void SceneManager::handleEvent(const sf::Event& event) {
    if (currentScene) currentScene->handleEvent(event);
}

void SceneManager::update(float dt, sf::RenderWindow& window) {
    if (currentScene) currentScene->update(dt, window);
    // Scene transitions
    if (currentState == State::Map) {
        auto* mapScene = dynamic_cast<MapScene*>(currentScene.get());
        if (mapScene && mapScene->triggeredBattle()) {
            switchTo(State::BulletHell, mapScene->getBattleEnemy());
            mapScene->resetBattleTrigger();
        }
        if (mapScene && mapScene->reachedStairs() && !(mapScene->triggeredBattle())) {
            std::cout << "Victory! Game Over.\n";
            // Optionally, trigger end screen or restart
        }
    }
    if (currentState == State::BulletHell) {
        auto* battleScene = dynamic_cast<BulletHellScene*>(currentScene.get());
        if (battleScene && battleScene->isOver()) {
            if (player.isDead())
                switchTo(State::GameOver);
            else
                switchTo(State::Map);
        }
    }

    if (currentState == State::GameOver) { // never entered
        auto* goScene = dynamic_cast<GameOverScene*>(currentScene.get());
        //Sleep(5);
        //std::cerr << "entered game over state\n";

        if (goScene && goScene->isRespawnRequested()) {
            player.reset();
            for (auto& enemy : enemies) enemy.reset();
            switchTo(State::Map); // here might be problem
        }
    }

}

void SceneManager::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (currentScene) target.draw(*currentScene, states);
}

GameOverScene::GameOverScene(Player& player) : player(player) {
    respawnRequested = false;
    lockoutTimer = 0.f;
    if (!font.openFromFile("game_over.ttf")) {
        std::cerr << "Failed to load font for Game Over screen!\n";
    }
}

void GameOverScene::handleEvent(const sf::Event& event) { // this also perhaps auto triggers
    // Only allow respawn after lockout
    if (lockoutTimer < lockoutDuration) return;
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->scancode == sf::Keyboard::Scan::Enter || key->scancode == sf::Keyboard::Scan::Space) {
            respawnRequested = true;
            //std::cerr << "respaw has been requested\n";
        }
    }
}

void GameOverScene::update(float dt, const sf::RenderWindow&) {
    lockoutTimer += dt;
}

bool GameOverScene::isRespawnRequested() const { return respawnRequested; }

void GameOverScene::draw(sf::RenderTarget& target, sf::RenderStates states) const { // we never enter this func
    sf::Text text(font, "GAME OVER\nPress Enter or Space to respawn", 48);
    text.setFillColor(sf::Color::Red);
    text.setStyle(sf::Text::Bold);
    text.setPosition(sf::Vector2f{ 120, 220 });
    //std::cerr << "game over screen has passed" << std::endl;
    target.draw(text, states);
}

