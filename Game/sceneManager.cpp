#include "sceneManager.h"
#include <iostream>
#include <algorithm>
#include <random>

MapScene::MapScene(Player& p,
    std::vector<Enemy>& e,
    Map& m,
    std::vector<MapObject>& o)
    : player(p), enemies(e), map(m), objects(o)
{
}

void MapScene::handleEvent(const sf::Event& ev) {
    if (auto* key = ev.getIf<sf::Event::KeyPressed>()) {
        int dx = 0, dy = 0;
        switch (key->scancode) {
        case sf::Keyboard::Scan::W: dy = -1; break;
        case sf::Keyboard::Scan::S: dy = 1;  break;
        case sf::Keyboard::Scan::A: dx = -1; break;
        case sf::Keyboard::Scan::D: dx = 1;  break;
        default: return;
        }
        sf::Vector2f pos = player.getPosition();
        int px = int(pos.x / Map::tileSize), py = int(pos.y / Map::tileSize);
        int nx = px + dx, ny = py + dy;
        Tile* t = map.getTile(nx, ny);
        if (!t || !t->passable) return;
        for (auto& obj : objects)
            if (obj.getGridX() == nx && obj.getGridY() == ny) return;
        for (auto& en : enemies)
            if (!en.isDead()) {
                int ex = int(en.getPosition().x / Map::tileSize),
                    ey = int(en.getPosition().y / Map::tileSize);
                if (ex == nx && ey == ny) { battleEnemy = &en; return; }
            }
        player.setPosition({ nx * Map::tileSize + Map::tileSize / 2,
                            ny * Map::tileSize + Map::tileSize / 2 });
        if (t->isStairs && !triggeredBattle()) {
            std::cout << "You win!\n"; atStairs = true;
        }
    }
}

void MapScene::update(float, const sf::RenderWindow&) { }

void MapScene::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(map, states);
    for (auto& obj : objects) target.draw(obj, states);
    target.draw(player, states);
    for (auto& en : enemies) if (!en.isDead()) target.draw(en, states);
}

BattleScene::BattleScene(Player& p, Enemy& e)
    : player(p), enemy(e)
{
    if (!font.openFromFile("game_over.ttf"))
        std::cerr << "Font load failed\n";

    if (!enemyTex.loadFromFile("Assets/basic_enemy.png")) { }
    switch (enemy.getType()) {
    case Enemy::Type::Ghost:
        enemyTex.loadFromFile("Assets/ghost_enemy.png"); break;
    case Enemy::Type::Boss:
        enemyTex.loadFromFile("Assets/boss_enemy.png");  break;
    default: break;
    }
    enemySpr.emplace(enemyTex);
    enemySpr->setOrigin({ enemyTex.getSize().x / 2.f,
                         enemyTex.getSize().y / 2.f });

    hpBarBg.emplace();
    hpBarFg.emplace();
    hpBarBg->setFillColor(sf::Color::White);
    hpBarFg->setFillColor(sf::Color::Red);

    if (!shieldTex.loadFromFile("Assets/shield.png"))
        std::cerr << "Shield load failed\n";
    shieldIcon.emplace(shieldTex);
    shieldIcon->setScale({ 0.3f, 0.3f });
    shieldIcon->setOrigin({ shieldTex.getSize().x / 2.f,
                           shieldTex.getSize().y / 2.f });

    setupUI();
}

void BattleScene::setupUI()
{
    rootMenu.clear();
    for (auto s : { "Attack", "Skills", "Flee" }) {
        sf::Text t(font,s, 30);
        t.setFillColor(sf::Color::White);
        rootMenu.push_back(t);
    }

    skillsMenu.clear();
    for (auto s : { "Fireball", "Heal", "Back" }) {
        sf::Text t(font, s, 30);
        t.setFillColor(sf::Color::White);
        skillsMenu.push_back(t);
    }
}

void BattleScene::handleMenuInput(const sf::Event& ev)
{
    if (auto* k = ev.getIf<sf::Event::KeyPressed>()) {
        auto& menu = (currentState == State::PlayerMenu ? rootMenu : skillsMenu);
        int& idx = (currentState == State::PlayerMenu ? rootIndex : skillIndex);
        int cnt = int(menu.size());

        if (k->scancode == sf::Keyboard::Scan::W || k->scancode == sf::Keyboard::Scan::Up)
            idx = (idx > 0 ? idx - 1 : cnt - 1);
        else if (k->scancode == sf::Keyboard::Scan::S || k->scancode == sf::Keyboard::Scan::Down)
            idx = (idx < cnt - 1 ? idx + 1 : 0);
        else if (k->scancode == sf::Keyboard::Scan::Enter ||
            k->scancode == sf::Keyboard::Scan::Space)
            executeAction();
        else if (k->scancode == sf::Keyboard::Scan::Escape &&
            currentState == State::SkillsMenu)
            currentState = State::PlayerMenu;
    }
}

void BattleScene::executeAction()
{
    if (currentState == State::PlayerMenu) {
        if (rootIndex == 0)
            enemy.takeDamage(player.getAtk());
        else if (rootIndex == 1) {
            currentState = State::SkillsMenu;
            return;
        }
        else {
            battleEnd = true;
            currentState = State::BattleEnded;
            return;
        }
    }
    else { 
        if (skillIndex == 0) {
            enemy.takeDamage(1);
            enemy.takeDamage(enemy.getHp()/2 + player.getAtk()*2);
        }
        else if (skillIndex == 1)
            player.heal(20);
        else { 
            currentState = State::PlayerMenu;
            return;
        }
    }

    if (enemy.isDead()) {
        battleEnd = true;
        currentState = State::BattleEnded;
    }
    else {
        startEnemyAttack();
    }
}

void BattleScene::startEnemyAttack()
{
    currentState = State::EnemyAttack;
    engine.start(player, enemy);
}

void BattleScene::handleEvent(const sf::Event& ev)
{
    if (currentState == State::PlayerMenu || currentState == State::SkillsMenu) {
        handleMenuInput(ev);
    }
    else if (currentState == State::EnemyAttack) {
        if (auto* kp = ev.getIf<sf::Event::KeyPressed>()) {
            switch (kp->scancode) {
            case sf::Keyboard::Scan::W: velocity.y = -speed; break;
            case sf::Keyboard::Scan::S: velocity.y = +speed; break;
            case sf::Keyboard::Scan::A: velocity.x = -speed; break;
            case sf::Keyboard::Scan::D: velocity.x = +speed; break;
            default: break;
            }
        }
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

void BattleScene::update(float dt, const sf::RenderWindow& win)
{
    if (currentState == State::EnemyAttack) {
        sf::Vector2f pos = player.getPosition();
        pos += velocity * dt;
        pos.x = std::clamp(pos.x, 0.f, float(win.getSize().x));
        pos.y = std::clamp(pos.y, 0.f, float(win.getSize().y));
        player.setPosition(pos);
    }

    if (currentState == State::PlayerMenu) {
        for (int i = 0; i < int(rootMenu.size()); ++i)
            rootMenu[i].setFillColor(i == rootIndex ? sf::Color::Yellow : sf::Color::White);
    }
    if (currentState == State::SkillsMenu) {
        for (int i = 0; i < int(skillsMenu.size()); ++i)
            skillsMenu[i].setFillColor(i == skillIndex ? sf::Color::Yellow : sf::Color::White);
    }

    if (currentState == State::EnemyAttack) {
        engine.update(dt, win.getSize(), player);
        if (player.isDead()) {
            battleEnd = true;
            currentState = State::BattleEnded;
        }
        else if (engine.isBattleOver()) {
            currentState = State::PlayerMenu;
        }
    }

    float W = float(win.getSize().x);
    float H = float(win.getSize().y);
    float Hs = H * 0.5f, Hb = H * 0.25f;
    float barW = W * 0.8f, barH = Hb * 0.4f;
    float bx = W * 0.1f, by = Hs + (Hb - barH) / 2.f;

    if (hpBarBg && hpBarFg) {
        hpBarBg->setSize({ barW, barH });
        hpBarBg->setPosition({ bx, by });
        hpBarBg->setFillColor(
            enemy.getShields() > 0
            ? sf::Color(130, 100, 100)
            : sf::Color::White
        );
        hpBarFg->setFillColor(enemy.getShields() > 0 ? sf::Color(100, 100, 100) : sf::Color::Red);

        float pct = float(enemy.getHp()) / float(enemy.getMaxHp());
        hpBarFg->setSize({ barW * pct, barH });
        hpBarFg->setPosition({ bx, by });
    }

    if (enemySpr) {
        enemySpr->setPosition({ W / 2.f, Hs / 2.f });
    }

    if (shieldIcon && enemy.getShields() > 0) {
        shieldIcon->setPosition({ bx - barH, by + barH / 2.f });
    }
}

void BattleScene::drawPlayerUI(sf::RenderTarget& t) const
{
    auto sz = t.getSize();
    float W = float(sz.x), H = float(sz.y);
    float menuH = H * 0.25f, y0 = H - menuH;

    sf::RectangleShape bg({ W, menuH });
    bg.setPosition({ 0, y0 });
    bg.setFillColor({ 0, 0, 0, 180 });
    t.draw(bg);

    const auto& menu = (currentState == State::PlayerMenu ? rootMenu : skillsMenu);
    int count = int(menu.size());
    if (count == 0) return;

    float spacing = W / float(count + 1);
    float textY = y0 + menuH * 0.5f;

    for (int i = 0; i < count; ++i) {
        sf::Text txt = menu[i]; 
        auto b = txt.getLocalBounds();
        float x = (i + 1) * spacing - (b.position.x + b.size.x / 2.f);
        float y = textY - (b.position.y + b.size.y / 2.f);
        txt.setPosition({ x, y });
        t.draw(txt);
    }
}

void BattleScene::draw(sf::RenderTarget& t, sf::RenderStates states) const
{
    t.clear(sf::Color::Black);

    if (currentState == State::PlayerMenu || currentState == State::SkillsMenu) {
        if (enemySpr)    t.draw(*enemySpr, states);
        if (hpBarBg)     t.draw(*hpBarBg, states);
        if (hpBarFg)     t.draw(*hpBarFg, states);
        if (shieldIcon && enemy.getShields() > 0) {
            t.draw(*shieldIcon, states);
            sf::Text count(font,
                std::to_string(enemy.getShields()),
                int(hpBarBg->getSize().y)
            );
            count.setFillColor(sf::Color::White);
            auto ip = shieldIcon->getPosition();
            auto b = count.getLocalBounds();
            count.setPosition({
                ip.x - (b.position.x + b.size.x / 2.f),
                ip.y - (b.position.y + b.size.y / 2.f)
                });
            t.draw(count, states);
        }
        drawPlayerUI(t);
    }
    else if (currentState == State::EnemyAttack) {
        t.draw(engine, states);
        t.draw(player, states);
    }
}
bool BattleScene::isOver() const
{
    return battleEnd;
}

bool BattleScene::enemyIsDead() const
{
    return enemy.isDead();
}

GameOverScene::GameOverScene(Player& p) : player(p) {
    if (!font.openFromFile("game_over.ttf"))
        std::cerr << "GameOver font failed\n";
}

void GameOverScene::handleEvent(const sf::Event& ev) {
    if (timer < lockout) return;
    if (auto* k = ev.getIf<sf::Event::KeyPressed>())
        if (k->scancode == sf::Keyboard::Scan::Enter || k->scancode == sf::Keyboard::Scan::Space)
            respawnRequested = true;
}

void GameOverScene::update(float dt, const sf::RenderWindow&) {
    timer += dt;
}

bool GameOverScene::isRespawnRequested()const { return respawnRequested; }

void GameOverScene::draw(sf::RenderTarget& t, sf::RenderStates)const {
    t.clear(sf::Color::Black);
    sf::Text text(font, "GAME OVER\nPress Enter to respawn", 100);
    text.setFillColor(sf::Color::Red);
    text.setPosition(sf::Vector2f{ 100, 200 });
    t.draw(text);
}

SceneManager::SceneManager()
    : currentState(State::Map),
    player(),
    enemies(),
    map(),
    mapObjects()
{
    enemies.emplace_back(Enemy::Type::Basic);
    enemies.emplace_back(Enemy::Type::Ghost);  
    enemies.emplace_back(Enemy::Type::Boss);   

    if (!hudFont.openFromFile("game_over.ttf")) {
        throw std::runtime_error("FATAL ERROR: Could not load HUD font 'game_over.ttf'.");
    }

    mapObjects.emplace_back(5, 5, Map::tileSize);
    mapObjects.emplace_back(7, 8, Map::tileSize);
    map.setStairs(Map::width - 1, Map::height - 1);

    std::vector<sf::Vector2i> spawnTiles;
    spawnTiles.reserve(Map::width * Map::height);
    for (int y = 0; y < Map::height; ++y) {
        for (int x = 0; x < Map::width; ++x) {
            Tile* t = map.getTile(x, y);
            if (t && t->passable && !t->isStairs) {
                spawnTiles.emplace_back(x, y);
            }
        }
    }

    static std::mt19937_64 rng{ std::random_device{}() };
    std::shuffle(spawnTiles.begin(), spawnTiles.end(), rng);

    for (size_t i = 0; i < enemies.size() && i < spawnTiles.size(); ++i) {
        int tx = spawnTiles[i].x;
        int ty = spawnTiles[i].y;
        enemies[i].setPosition({
            tx * Map::tileSize + Map::tileSize * 0.5f,
            ty * Map::tileSize + Map::tileSize * 0.5f
            });


    }
    player.setPosition({ Map::tileSize / 2.f, Map::tileSize / 2.f });
    switchTo(State::Map);
}

void SceneManager::switchTo(State ns, Enemy* be) {
    currentState = ns;
    if (ns == State::Map)
        currentScene = std::make_unique<MapScene>(player, enemies, map, mapObjects);
    else if (ns == State::Battle && be) {
        preBattlePos = player.getPosition();
        battleInitiated = true;
        currentScene = std::make_unique<BattleScene>(player, *be);
    }
    else if (ns == State::GameOver) {
        currentScene = std::make_unique<GameOverScene>(player);
    }
}

void SceneManager::handleEvent(const sf::Event& ev) {
    if (currentScene) currentScene->handleEvent(ev);
}

void SceneManager::update(float dt, sf::RenderWindow& w) {
    if (!currentScene) return;
    currentScene->update(dt, w);

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
            else if (bs->enemyIsDead()) {
                player.setPosition(preBattlePos);
                switchTo(State::Map);
            }
            battleInitiated = false;
        }
    }
    else if (currentState == State::GameOver) {
        auto* gs = dynamic_cast<GameOverScene*>(currentScene.get());
        if (gs && gs->isRespawnRequested()) {
            player.reset();
            for (auto& e : enemies) e.reset();
            player.setPosition({ Map::tileSize / 2.f, Map::tileSize / 2.f });
            switchTo(State::Map);
        }
    }
}

void SceneManager::draw(sf::RenderTarget& t, sf::RenderStates s) const {
    if (currentScene) t.draw(*currentScene, s);
    drawPlayerHUD(t, s);
}

void SceneManager::drawPlayerHUD(sf::RenderTarget& target, sf::RenderStates states) const {
    std::string hudText = "HP: " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp())
        + "  |  Shields: " + std::to_string(player.getShields());

    sf::Text text(hudFont, hudText, 40);
    text.setFillColor(sf::Color::White);
    text.setPosition(sf::Vector2f{ 10.f, -10.f });

    sf::RectangleShape bg;
    sf::FloatRect textBounds = text.getLocalBounds();
    bg.setSize(sf::Vector2f(textBounds.size.x + 20.f, textBounds.size.y + 20.f));
    bg.setPosition(sf::Vector2f(10, 10));
    bg.setFillColor(sf::Color(0, 0, 0, 150));

    target.draw(bg);
    target.draw(text);
}
