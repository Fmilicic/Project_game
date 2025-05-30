#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "MapObject.h"
#include "BHE.h"

// --- Abstract Scene Interface ---
class GameScene : public sf::Drawable {
public:
    virtual ~GameScene() = default;
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float dt, const sf::RenderWindow& window) = 0;
};

// --- MapScene ---
class MapScene : public GameScene {
public:
    MapScene(Player& player, std::vector<Enemy>& enemies);

    void handleEvent(const sf::Event& event) override;
    void update(float dt, const sf::RenderWindow& window) override;
    bool reachedStairs() const { return atStairs; }
    bool triggeredBattle() const { return battleEnemy != nullptr; }
    Enemy* getBattleEnemy() { return battleEnemy; }
    void resetBattleTrigger() { battleEnemy = nullptr; }

private:
    Player& player;
    std::vector<Enemy>& enemies;
    Map map;
    std::vector<MapObject> objects;
    bool atStairs = false;
    Enemy* battleEnemy = nullptr;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

// --- BulletHellScene ---
class BulletHellScene : public GameScene {
public:
    BulletHellScene(Player& player, Enemy& enemy);

    void handleEvent(const sf::Event& event) override;
    void update(float dt, const sf::RenderWindow& window) override;
    bool isOver() const;

private:
    BulletHellEngine engine;
    Player& player;
    Enemy& enemy;
    sf::CircleShape playerShape;
    sf::Vector2f velocity;
    float speed = 200.f;
    bool sceneOver = false;
    int playerHpCache;
    int playerShieldsCache;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

class GameOverScene : public GameScene {
public:
    GameOverScene(Player& player);
    void update(float dt, const sf::RenderWindow& window) override;
    bool isRespawnRequested() const;
    void handleEvent(const sf::Event& event) override;
private:
    Player& player;
    sf::Font font;
    bool respawnRequested = false;
    float lockoutTimer = 0.f;
    static constexpr float lockoutDuration = 0.5f; // 0.5 seconds lockout
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

// --- SceneManager ---
class SceneManager : public sf::Drawable {
public:
    enum class State { Map, BulletHell, GameOver, PlayerTurn };
    SceneManager();
    void switchTo(State newState, Enemy* battleEnemy = nullptr);
    void handleEvent(const sf::Event& event);
    void update(float dt, sf::RenderWindow& window);
private:
    State currentState;
    std::unique_ptr<GameScene> currentScene;
    Player player;
    std::vector<Enemy> enemies;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

#endif // SCENEMANAGER_H
