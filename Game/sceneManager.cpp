#include "sceneManager.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// ---------------- MapScene Implementation ----------------

MapScene::MapScene(Player& player, std::vector<Enemy>& enemies)
    : player(player), enemies(enemies), map()
{
    objects.emplace_back(5, 5, Map::tileSize);
    objects.emplace_back(7, 8, Map::tileSize);
    map.setStairs(Map::width - 1, Map::height - 1);

    player.setPosition({ Map::tileSize / 2, Map::tileSize / 2 });
    if (!enemies.empty()) {
        enemies[0].setPosition({ 9 * Map::tileSize + Map::tileSize / 2,
                                 7 * Map::tileSize + Map::tileSize / 2 });
        enemies[0].setType(Enemy::Type::Boss);
    }
}

void MapScene::handleEvent(const sf::Event& event)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        int dx = 0, dy = 0;
        switch (key->scancode) {
        case sf::Keyboard::Scan::W: dy = -1; break;
        case sf::Keyboard::Scan::S: dy = 1;  break;
        case sf::Keyboard::Scan::A: dx = -1; break;
        case sf::Keyboard::Scan::D: dx = 1;  break;
        default: return;
        }
        sf::Vector2f pos = player.getPosition();
        int px = static_cast<int>(pos.x / Map::tileSize);
        int py = static_cast<int>(pos.y / Map::tileSize);
        int nx = px + dx, ny = py + dy;

        // Bounds & passable
        Tile* target = map.getTile(nx, ny);
        if (!target || !target->passable) return;

        // Map objects
        for (auto& obj : objects) {
            if (obj.getGridX() == nx && obj.getGridY() == ny) {
                std::cout << "Interacted with object at " << nx << "," << ny << "\n";
                return;
            }
        }

        // Enemies
        for (auto& e : enemies) {
            if (!e.isDead()) {
                int ex = static_cast<int>(e.getPosition().x / Map::tileSize);
                int ey = static_cast<int>(e.getPosition().y / Map::tileSize);
                if (ex == nx && ey == ny) {
                    battleEnemy = &e;
                    return;
                }
            }
        }

        // Move player
        player.setPosition({ nx * Map::tileSize + Map::tileSize / 2,
                             ny * Map::tileSize + Map::tileSize / 2 });

        // Win
        if (target->isStairs && !triggeredBattle()) {
            atStairs = true;
            std::cout << "You win!\n";
        }
    }
}

void MapScene::update(float, const sf::RenderWindow&) { }

void MapScene::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(map, states);
    for (auto& obj : objects) target.draw(obj, states);
    target.draw(player, states);
    for (auto& e : enemies)
        if (!e.isDead()) target.draw(e, states);
}

// ---------------- BattleScene Implementation ----------------

BattleScene::BattleScene(Player& p, Enemy& e)
    : player(p), enemy(e)
{
    // Load font
    if (!font.openFromFile("game_over.ttf"))
        std::cerr << "Failed to load font for BattleScene\n";

    // Load enemy sprite
    if (e.getType() == Enemy::Type::Basic) {
        if (!enemyTex.loadFromFile("Assets/basic_enemy.png"))
            std::cerr << "Failed to load placeholder.png\n";
        enemySpr.emplace(enemyTex);
        enemySpr->setOrigin(sf::Vector2f{
            float(enemyTex.getSize().x) / 2.f,
            float(enemyTex.getSize().y) / 2.f });
    }
    if (e.getType() == Enemy::Type::Ghost) {
        if (!enemyTex.loadFromFile("Assets/ghost_enemy.png"))
            std::cerr << "Failed to load placeholder.png\n";
        enemySpr.emplace(enemyTex);
        enemySpr->setOrigin(sf::Vector2f{
            float(enemyTex.getSize().x) / 2.f,
            float(enemyTex.getSize().y) / 2.f });
    }
    if (e.getType() == Enemy::Type::Boss) {
        if (!enemyTex.loadFromFile("Assets/boss_enemy.png"))
            std::cerr << "Failed to load placeholder.png\n";
        enemySpr.emplace(enemyTex);
        enemySpr->setOrigin(sf::Vector2f{
            float(enemyTex.getSize().x) / 2.f,
            float(enemyTex.getSize().y) / 2.f });
    }
    // Prepare HP bar shapes
    hpBarBg.emplace();
    hpBarFg.emplace();
    hpBarBg->setFillColor(sf::Color::White);
    hpBarFg->setFillColor(sf::Color::Red);

    // Load shield icon
    if (!shieldTex.loadFromFile("Assets/shield.png"))
        std::cerr << "Failed to load shield.png\n";
    shieldIcon.emplace(shieldTex);
    shieldIcon->setScale(sf::Vector2f{ 0.5f, 0.5f });
    shieldIcon->setOrigin(sf::Vector2f{
        shieldTex.getSize().x * 0.5f,
        shieldTex.getSize().y * 0.5f });

    setupUI();
}

void BattleScene::handleMenuInput(const sf::Event& event)
{
    if (auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        // Determine which menu we’re on
        auto& menu = (currentState == State::PlayerMenu ? rootMenu : skillsMenu);
        int& index = (currentState == State::PlayerMenu ? rootIndex : skillIndex);
        int count = static_cast<int>(menu.size());

        // Up / Down
        if (key->scancode == sf::Keyboard::Scan::W || key->scancode == sf::Keyboard::Scan::Up)
            index = (index > 0 ? index - 1 : count - 1);
        else if (key->scancode == sf::Keyboard::Scan::S || key->scancode == sf::Keyboard::Scan::Down)
            index = (index < count - 1 ? index + 1 : 0);

        // Confirm
        else if (key->scancode == sf::Keyboard::Scan::Enter || key->scancode == sf::Keyboard::Scan::Space)
            executeAction();

        // Back from Skills
        else if (key->scancode == sf::Keyboard::Scan::Escape && currentState == State::SkillsMenu)
            currentState = State::PlayerMenu;
    }
}

void BattleScene::handleEvent(const sf::Event& ev)
{
    if (currentState == State::PlayerMenu || currentState == State::SkillsMenu) {
        handleMenuInput(ev);
    }
    else if (currentState == State::EnemyAttack) {
        // Movement pressed
        if (auto* kp = ev.getIf<sf::Event::KeyPressed>()) {
            switch (kp->scancode) {
            case sf::Keyboard::Scan::W: velocity.y = -speed; break;
            case sf::Keyboard::Scan::S: velocity.y = +speed; break;
            case sf::Keyboard::Scan::A: velocity.x = -speed; break;
            case sf::Keyboard::Scan::D: velocity.x = +speed; break;
            default: break;
            }
        }
        // Movement released
        if (auto* kr = ev.getIf<sf::Event::KeyReleased>()) {
            switch (kr->scancode) {
            case sf::Keyboard::Scan::W:
            case sf::Keyboard::Scan::S: velocity.y = 0.f; break;
            case sf::Keyboard::Scan::A:
            case sf::Keyboard::Scan::D: velocity.x = 0.f; break;
            default: break;
            }
        }
    }
}

// Below your BattleScene constructor in sceneManager.cpp

void BattleScene::setupUI()
{
    // Root menu: Attack, Skills, Flee
    rootMenu.clear();
    std::array<const char*, 3> rootItems = { "Attack", "Skills", "Flee" };
    for (int i = 0; i < 3; ++i)
    {
        sf::Text txt(font, rootItems[i], 30);
        txt.setFillColor(sf::Color::White);
        rootMenu.push_back(txt);
    }

    // Skills menu: Fireball, Heal, Back
    skillsMenu.clear();
    std::array<const char*, 3> skillItems = { "Fireball", "Heal", "Back" };
    for (int i = 0; i < 3; ++i)
    {
        sf::Text txt(font, skillItems[i], 30);
        txt.setFillColor(sf::Color::White);
        skillsMenu.push_back(txt);
    }
}

void BattleScene::executeAction()
{
    // Player’s choice from the root menu or skills menu
    if (currentState == State::PlayerMenu)
    {
        switch (rootIndex)
        {
        case 0: // Attack
            enemy.takeDamage(player.getAtk());
            break;

        case 1: // Skills → switch into skills submenu
            currentState = State::SkillsMenu;
            return;

        case 2: // Flee
            battleEnd = true;
            currentState = State::BattleEnded;
            return;
        }
    }
    else if (currentState == State::SkillsMenu)
    {
        switch (skillIndex)
        {
        case 0: // Fireball
            enemy.takeDamage(player.getAtk() * 2);
            break;

        case 1: // Heal
            player.heal(20);
            break;

        case 2: // Back to main menu
            currentState = State::PlayerMenu;
            return;
        }
    }

    // After performing an action, check if enemy died
    if (enemy.isDead())
    {
        battleEnd = true;
        currentState = State::BattleEnded;
    }
    else
    {
        // Otherwise let the enemy take its turn
        startEnemyAttack();
    }
}

void BattleScene::startEnemyAttack()
{
    // Switch to enemy’s bullet‐hell turn
    currentState = State::EnemyAttack;

    // Begin spawning bullets targeting the player
    engine.start(player, enemy);
}

void BattleScene::update(float dt, const sf::RenderWindow& window)
{
    // Move player in bullet‐hell
    if (currentState == State::EnemyAttack) {
        sf::Vector2f pos = player.getPosition();
        pos += velocity * dt;
        pos.x = std::clamp(pos.x, 0.f, float(window.getSize().x));
        pos.y = std::clamp(pos.y, 0.f, float(window.getSize().y));
        player.setPosition(pos);
    }

    // Highlight menus
    if (currentState == State::PlayerMenu) {
        for (int i = 0; i < int(rootMenu.size()); ++i)
            rootMenu[i].setFillColor(i == rootIndex ? sf::Color::Yellow : sf::Color::White);
    }
    else if (currentState == State::SkillsMenu) {
        for (int i = 0; i < int(skillsMenu.size()); ++i)
            skillsMenu[i].setFillColor(i == skillIndex ? sf::Color::Yellow : sf::Color::White);
    }

    // Enemy turn: spawn & move bullets, check collisions
    if (currentState == State::EnemyAttack) {
        engine.update(dt, window.getSize(), player);
        if (player.isDead() || engine.isBattleOver()) {
            battleEnd = true;
            currentState = State::BattleEnded;
        }
    }

    // Update HP bar sizes & colors
    // Region: next 25% of screen
    float W = float(window.getSize().x);
    float H = float(window.getSize().y);
    float Hs = H * 0.5f, Hb = H * 0.25f;
    float barW = W * 0.8f, barH = Hb * 0.4f;
    float bx = W * 0.1f, by = Hs + (Hb - barH) / 2.f;

    // --- Position enemy sprite and shield icon in update(), not draw() ---
    if (enemySpr) {
        float Hs = H * 0.5f;
        enemySpr->setPosition({ W / 2.f, Hs / 2.f });
    }

    // Position shield icon—only if shields remain
    if (hpBarBg && shieldIcon && enemy.getShields() > 0) {
        // hpBarBg was already positioned earlier in update
        auto pos = hpBarBg->getPosition();
        float barH = hpBarBg->getSize().y;
        shieldIcon->setPosition({ pos.x - barH, pos.y + barH / 2.f });
    }
    if (enemy.getShields() > 0) {
        hpBarBg->setFillColor(sf::Color(100, 100, 100)); // greyed
        hpBarFg->setFillColor(sf::Color(100, 100, 100));
    }
    else {
        hpBarBg->setFillColor(sf::Color::White);
    }


    hpBarBg->setSize({ barW, barH });
    hpBarBg->setPosition({ bx, by });
    hpBarBg->setFillColor(enemy.getShields() > 0
        ? sf::Color(100, 100, 100)
        : sf::Color::White);

    float pct = float(enemy.getHp()) / float(enemy.getMaxHp());
    hpBarFg->setSize({ barW * pct, barH });
    hpBarFg->setPosition({ bx, by });
}

bool BattleScene::isOver() const
{
    return battleEnd;
}

void BattleScene::drawPlayerUI(sf::RenderTarget& target) const
{
    auto size = target.getSize();
    float W = float(size.x), H = float(size.y);
    float menuH = H * 0.25f, y0 = H - menuH;

    // semi‐transparent background
    sf::RectangleShape bg({ W, menuH });
    bg.setPosition({ 0, y0 });
    bg.setFillColor({ 0,0,0,180 });
    target.draw(bg);

    // choose menu
    const auto& menu = (currentState == State::PlayerMenu
        ? rootMenu : skillsMenu);
    int count = int(menu.size());
    if (count == 0) return;

    float spacing = W / float(count + 1);
    float textY = y0 + menuH * 0.5f;

    for (int i = 0; i < count; ++i) {
        sf::Text txt = menu[i]; // copy
        auto b = txt.getLocalBounds();
        float x = (i + 1) * spacing - (b.position.x + b.size.x / 2.f);
        float y = textY - (b.position.y + b.size.y / 2.f);
        txt.setPosition({ x,y });
        target.draw(txt);
    }
}

void BattleScene::draw(sf::RenderTarget& target, sf::RenderStates) const
{
    target.clear(sf::Color::Black);

    // Draw enemy + HP bar + menu only during player’s turn
    if (currentState == State::PlayerMenu || currentState == State::SkillsMenu) {
        // Enemy sprite
        if (enemySpr) {
            float W = float(target.getSize().x);
            float H = float(target.getSize().y);
            float Hs = H * 0.5f;
            // Position already set in update()
            target.draw(*enemySpr);
        }

        // HP bar
        if (hpBarBg && hpBarFg) {
            target.draw(*hpBarBg);
            target.draw(*hpBarFg);

            // Shield icon + count
            if (enemy.getShields() > 0 && shieldIcon) {
                target.draw(*shieldIcon);

                // Centered shield count
                sf::Text count(font,
                    std::to_string(enemy.getShields()),
                    int(hpBarBg->getSize().y));
                count.setFillColor(sf::Color::White);
                auto iconPos = shieldIcon->getPosition();
                auto b = count.getLocalBounds();
                count.setPosition({
                    iconPos.x - (b.position.x + b.size.x / 2.f),
                    iconPos.y - (b.position.y + b.size.y / 2.f)
                    });
                target.draw(count);
            }
        }

        // Player menu
        drawPlayerUI(target);
    }
    else if (currentState == State::EnemyAttack) {
        // Only bullet‐hell + player here
        target.draw(engine, sf::RenderStates::Default);
        target.draw(player, sf::RenderStates::Default);
    }
}


GameOverScene::GameOverScene(Player& p) : player(p)
{
    if (!font.openFromFile("game_over.ttf"))
        std::cerr << "Failed to load font for GameOverScene\n";
}

void GameOverScene::handleEvent(const sf::Event& ev)
{
    // lockout if needed
    if (timer < lockout) return;
    if (auto* k = ev.getIf<sf::Event::KeyPressed>()) {
        if (k->scancode == sf::Keyboard::Scan::Enter ||
            k->scancode == sf::Keyboard::Scan::Space)
        {
            respawnRequested = true;
        }
    }
}

void GameOverScene::update(float dt, const sf::RenderWindow&)
{
    timer += dt;
}

bool GameOverScene::isRespawnRequested() const
{
    return respawnRequested;
}

void GameOverScene::draw(sf::RenderTarget& t, sf::RenderStates) const
{
    t.clear(sf::Color::Black);
    sf::Text text(font,"GAME OVER\nPress Enter to respawn", 48);
    text.setFillColor(sf::Color::Red);
    text.setPosition(sf::Vector2f{ 100, 200 });
    t.draw(text);
}

// ---------------- SceneManager Implementation ----------------

SceneManager::SceneManager()
    : currentState(State::Map)
{
    enemies.emplace_back(Enemy::Type::Basic);
    switchTo(State::Map);
}

void SceneManager::switchTo(State newState, Enemy* e)
{
    currentState = newState;
    if (newState == State::Map) {
        currentScene = std::make_unique<MapScene>(player, enemies);
    }
    else if (newState == State::Battle && e) {
        currentScene = std::make_unique<BattleScene>(player, *e);
    }
    else if (newState == State::GameOver) {
        currentScene = std::make_unique<GameOverScene>(player);
    }
}

void SceneManager::handleEvent(const sf::Event& ev)
{
    if (currentScene) currentScene->handleEvent(ev);
}

void SceneManager::update(float dt, sf::RenderWindow& win)
{
    if (currentScene) currentScene->update(dt, win);

    if (currentState == State::Map) {
        auto* ms = dynamic_cast<MapScene*>(currentScene.get());
        if (ms && ms->triggeredBattle()) {
            switchTo(State::Battle, ms->getBattleEnemy());
            ms->resetBattleTrigger();
        }
    }
    else if (currentState == State::Battle) {
        auto* bs = dynamic_cast<BattleScene*>(currentScene.get());
        if (bs && bs->isOver()) {
            if (player.isDead()) switchTo(State::GameOver);
            else switchTo(State::Map);
        }
    }
    else if (currentState == State::GameOver) {
        auto* gs = dynamic_cast<GameOverScene*>(currentScene.get());
        if (gs && gs->isRespawnRequested()) {
            player.reset();
            for (auto& e : enemies) e.reset();
            switchTo(State::Map);
        }
    }
}

void SceneManager::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    if (currentScene) target.draw(*currentScene, states);
}
