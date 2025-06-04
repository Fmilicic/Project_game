#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <optional>
#include "Entity.h"
#include "Map.h"
#include "MapObject.h"
#include "BHE.h"

class GameScene : public sf::Drawable {
public:
    virtual ~GameScene() = default;
    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float dt, const sf::RenderWindow& window) = 0;
};

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

// ---------------- BattleScene ----------------
class BattleScene : public GameScene {
public:
    enum class State { PlayerMenu, SkillsMenu, EnemyAttack, BattleEnded };
    BattleScene(Player& player, Enemy& enemy);

    void handleEvent(const sf::Event& event) override;
    void update(float dt, const sf::RenderWindow& window) override;
    bool isOver() const;

private:
    Player& player;
    Enemy& enemy;
    State   currentState = State::PlayerMenu;

    // UI
    sf::Font              font;
    std::vector<sf::Text> rootMenu, skillsMenu;
    int                   rootIndex = 0, skillIndex = 0;

    // Bullet hell
    BulletHellEngine      engine;
    bool                  battleEnd = false;

    // Player movement
    sf::Vector2f          velocity{ 0.f,0.f };
    float                 speed = 200.f;

    // Enemy visuals: use optional so we can construct after loading
    sf::Texture           enemyTex;
    std::optional<sf::Sprite> enemySpr;

    // HP bar & shield
    std::optional<sf::RectangleShape> hpBarBg, hpBarFg;
    sf::Texture           shieldTex;
    std::optional<sf::Sprite> shieldIcon;

    void setupUI();
    void handleMenuInput(const sf::Event& event);
    void executeAction();
    void startEnemyAttack();

    void drawPlayerUI(sf::RenderTarget& target) const;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

class GameOverScene : public GameScene {
public:
    GameOverScene(Player& player);
    void handleEvent(const sf::Event& event) override;
    void update(float dt, const sf::RenderWindow& window) override;
    bool isRespawnRequested() const;

private:
    Player& player;
    sf::Font font;
    bool respawnRequested = false;
    float timer = 0.f;
    static constexpr float lockout = 0.5f;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

class SceneManager : public sf::Drawable {
public:
    enum class State { Map, Battle, GameOver };
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
