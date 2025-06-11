#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "MapObject.h"
#include "BHE.h"

class GameScene : public sf::Drawable {
public:
    virtual ~GameScene() = default;
    virtual void handleEvent(const sf::Event& ev) = 0;
    virtual void update(float dt, const sf::RenderWindow& window) = 0;
};

class MapScene : public GameScene {
public:
    MapScene(Player& player,
        std::vector<Enemy>& enemies,
        Map& map,
        std::vector<MapObject>& objects);

    void handleEvent(const sf::Event& ev) override;
    void update(float dt, const sf::RenderWindow& window) override;
    bool reachedStairs() const { return atStairs; }
    bool triggeredBattle() const { return battleEnemy != nullptr; }
    Enemy* getBattleEnemy() { return battleEnemy; }
    void   resetBattleTrigger() { battleEnemy = nullptr; }

private:
    Player& player;
    std::vector<Enemy>& enemies;
    Map& map;
    std::vector<MapObject>& objects;
    bool                    atStairs = false;
    Enemy* battleEnemy = nullptr;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

class BattleScene : public GameScene {
public:
    enum class State { PlayerMenu, SkillsMenu, EnemyAttack, BattleEnded };
    BattleScene(Player& player, Enemy& enemy);

    void handleEvent(const sf::Event& ev) override;
    void update(float dt, const sf::RenderWindow& window) override;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    bool isOver() const;
    bool enemyIsDead() const;

private:
    Player& player;
    Enemy& enemy;
    State   currentState = State::PlayerMenu;

    mutable sf::Font              font;
    std::vector<sf::Text> rootMenu, skillsMenu;
    int                   rootIndex = 0, skillIndex = 0;

    BulletHellEngine      engine;
    bool                  battleEnd = false;

    // movement
    sf::Vector2f          velocity{ 0.f,0.f };
    float                 speed = 200.f;

    // visuals
    sf::Texture           enemyTex;
    std::optional<sf::Sprite> enemySpr;
    std::optional<sf::RectangleShape> hpBarBg, hpBarFg;
    sf::Texture           shieldTex;
    std::optional<sf::Sprite> shieldIcon;

    void setupUI();
    void handleMenuInput(const sf::Event& ev);
    void executeAction();
    void startEnemyAttack();

    void drawPlayerUI(sf::RenderTarget& target) const;
};

class GameOverScene : public GameScene {
public:
    GameOverScene(Player& player);
    void handleEvent(const sf::Event& ev) override;
    void update(float dt, const sf::RenderWindow& window) override;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    bool isRespawnRequested() const;
private:
    Player& player;
    mutable sf::Font font;
    bool     respawnRequested = false;
    float    timer = 0.f;
    static constexpr float lockout = 0.5f;
};

class SceneManager : public sf::Drawable {
public:
    enum class State { Map, Battle, GameOver };
    SceneManager();

    void switchTo(State newState, Enemy* battleEnemy = nullptr);
    void handleEvent(const sf::Event& ev);
    void update(float dt, sf::RenderWindow& window);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    State currentState;
    std::unique_ptr<GameScene> currentScene;

    Player                     player;
    std::vector<Enemy>         enemies;

    mutable sf::Font hudFont;
    void drawPlayerHUD(sf::RenderTarget& target, sf::RenderStates states) const;

    // persistent map
    Map                         map;
    std::vector<MapObject>      mapObjects;

    // turn & position
    sf::Vector2f               preBattlePos;
    bool                       battleInitiated = false;
};

