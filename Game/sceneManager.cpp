#include "sceneManager.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <cstdint>
#include <stdexcept>

SceneManager* SceneManager::s_instance = nullptr;

MapScene::MapScene(Player& p, std::vector<Enemy>& e, Map& m, std::vector<MapObject>& o, bool isBossDefeated, AudioManager& audio)
    : player(p), enemies(e), map(m), objects(o), bossDefeated(isBossDefeated), audioManager(audio) {}

void MapScene::handleEvent(const sf::Event& ev) {
    if (auto* key = ev.getIf<sf::Event::KeyPressed>()) {
        int dx = 0, dy = 0;
        switch (key->scancode) {
        case sf::Keyboard::Scan::W: dy = -1; break;
        case sf::Keyboard::Scan::S: dy = 1; break;
        case sf::Keyboard::Scan::A: dx = -1; break;
        case sf::Keyboard::Scan::D: dx = 1; break;
        default: return;
        }

        sf::Vector2f pos = player.getPosition();
        int px = int(pos.x / Map::tileSize), py = int(pos.y / Map::tileSize);
        int nx = px + dx, ny = py + dy;

        Tile* t = map.getTile(nx, ny);

        for (auto& obj : objects) {
            if (obj.getGridX() == nx && obj.getGridY() == ny) {
                if (!obj.isUsed()) {
                    audioManager.playSound(AudioManager::SoundID::BuffPickup);
                    player.applyBuff(obj.getBuffType());
                    obj.use();
                }
                return;
            }
        }

        if (!t || !t->passable) return;
        for (auto& obj : objects) if (obj.getGridX() == nx && obj.getGridY() == ny) return;
        for (auto& en : enemies) if (!en.isDead()) {
            int ex = int(en.getPosition().x / Map::tileSize), ey = int(en.getPosition().y / Map::tileSize);
            if (ex == nx && ey == ny) { battleEnemy = &en; return; }
        }

        player.setPosition({ nx * Map::tileSize + Map::tileSize / 2, ny * Map::tileSize + Map::tileSize / 2 });

        if (t->isStairs) {
            if (bossDefeated) {
                atStairs = true;
            }
            else {
                SceneManager::pushNotification("You must defeat the boss before you can escape!");
            }
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

GameOverScene::GameOverScene(Player& p, bool isVictory)
    : player(p), victory(isVictory) {
    if (!font.openFromFile("game_over.ttf")) {
        throw std::runtime_error("FATAL ERROR: Could not load font 'game_over.ttf' in GameOverScene.");
    }
}

void GameOverScene::handleEvent(const sf::Event& ev) {
    if (victory) return;
    if (timer < lockout) return;
    if (auto* k = ev.getIf<sf::Event::KeyPressed>()) if (k->scancode == sf::Keyboard::Scan::Enter || k->scancode == sf::Keyboard::Scan::Space) respawnRequested = true;
}

void GameOverScene::update(float dt, const sf::RenderWindow&) { timer += dt; }
bool GameOverScene::isRespawnRequested()const { return respawnRequested; }
void GameOverScene::draw(sf::RenderTarget& t, sf::RenderStates) const {
    t.clear(sf::Color::Black);
    sf::Text text(font, "", 100);
    if (victory) {
        text.setString("VICTORY!");
        text.setFillColor(sf::Color::Yellow);
    }
    else {
        text.setString("GAME OVER\nPress Enter to respawn");
        text.setFillColor(sf::Color::Red);
    }
    text.setPosition(sf::Vector2f{ 100, 200 });
    t.draw(text);
}

BattleScene::BattleScene(Player& p, Enemy& e, AudioManager& audio)
    : player(p), enemy(e), audioManager(audio) {
    if (!font.openFromFile("game_over.ttf")) {
        throw std::runtime_error("FATAL ERROR: Could not load font 'game_over.ttf' in BattleScene.");
    }
    std::string texturePath;
    switch (enemy.getType()) {
    case Enemy::Type::Ghost: texturePath = "Assets/ghost_enemy.png"; break;
    case Enemy::Type::Boss: texturePath = "Assets/boss_enemy.png"; break;
    default: texturePath = "Assets/basic_enemy.png"; break;
    }
    if (!enemyTex.loadFromFile(texturePath)) {
        std::cerr << "Warning: Failed to load enemy texture: " << texturePath << std::endl;
    }
    enemySpr.emplace(enemyTex);
    enemySpr->setOrigin({ enemyTex.getSize().x / 2.f, enemyTex.getSize().y / 2.f });
    hpBarBg.emplace(); hpBarFg.emplace();
    hpBarBg->setFillColor(sf::Color::White);
    hpBarFg->setFillColor(sf::Color::Red);
    if (!shieldTex.loadFromFile("Assets/shield.png")) std::cerr << "Warning: Shield load failed\n";
    shieldIcon.emplace(shieldTex);
    shieldIcon->setScale({ 0.3f, 0.3f });
    shieldIcon->setOrigin({ shieldTex.getSize().x / 2.f, shieldTex.getSize().y / 2.f });
    setupUI();
}

void BattleScene::setupUI() {
    rootMenu.clear();
    for (auto s : { "Attack", "Skills", "Flee" }) {
        sf::Text t(font, s, 30);
        t.setFillColor(sf::Color::White);
        rootMenu.push_back(t);
    }

    skillsMenu.clear();
    for (auto s : { "Fireball", "Heal", "Protect" }) {
        sf::Text t(font, s, 30);
        t.setFillColor(sf::Color::White);
        skillsMenu.push_back(t);
    }
}

void BattleScene::executeAction() {
    if (currentState == State::PlayerMenu) {
        if (rootIndex == 0) enemy.takeDamage(player.getAtk());
        else if (rootIndex == 1) { currentState = State::SkillsMenu; return; }
        else { battleEnd = true; currentState = State::BattleEnded; return; }
    }
    else { // In SkillsMenu
        if (skillIndex == 0) {
            enemy.takeDamage(player.getAtk() * 2);
        }
        else if (skillIndex == 1) {
            player.heal(20);
        }
        else if (skillIndex == 2) {
            player.addShields(2);
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

void BattleScene::handleMenuInput(const sf::Event& ev) {
    if (auto* k = ev.getIf<sf::Event::KeyPressed>()) {
        auto& menu = (currentState == State::PlayerMenu ? rootMenu : skillsMenu);
        int& idx = (currentState == State::PlayerMenu ? rootIndex : skillIndex);
        int cnt = int(menu.size());

        if (k->scancode == sf::Keyboard::Scan::W || k->scancode == sf::Keyboard::Scan::Up) idx = (idx > 0 ? idx - 1 : cnt - 1);
        else if (k->scancode == sf::Keyboard::Scan::S || k->scancode == sf::Keyboard::Scan::Down) idx = (idx < cnt - 1 ? idx + 1 : 0);
        else if (k->scancode == sf::Keyboard::Scan::Enter || k->scancode == sf::Keyboard::Scan::Space) executeAction();
        else if (k->scancode == sf::Keyboard::Scan::Escape && currentState == State::SkillsMenu) currentState = State::PlayerMenu;
    }
}

void BattleScene::startEnemyAttack() {
    currentState = State::EnemyAttack;
    engine.start(player, enemy);
}

void BattleScene::handleEvent(const sf::Event& ev) {
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
            case sf::Keyboard::Scan::W: case sf::Keyboard::Scan::S: velocity.y = 0.f; break;
            case sf::Keyboard::Scan::A: case sf::Keyboard::Scan::D: velocity.x = 0.f; break;
            default: break;
            }
        }
    }
}

void BattleScene::update(float dt, const sf::RenderWindow& win) {
    if (enemy.getType() == Enemy::Type::Boss) {
        audioManager.updateBossMusic(enemy);
    }

    if (currentState == State::EnemyAttack) {
        sf::Vector2f pos = player.getPosition();
        pos += velocity * dt;
        pos.x = std::clamp(pos.x, 0.f, float(win.getSize().x));
        pos.y = std::clamp(pos.y, 0.f, float(win.getSize().y));
        player.setPosition(pos);
    }

    if (currentState == State::PlayerMenu) {
        for (int i = 0; i < int(rootMenu.size()); ++i) rootMenu[i].setFillColor(i == rootIndex ? sf::Color::Yellow : sf::Color::White);
    }
    else if (currentState == State::SkillsMenu) {
        for (int i = 0; i < int(skillsMenu.size()); ++i) skillsMenu[i].setFillColor(i == skillIndex ? sf::Color::Yellow : sf::Color::White);
    }

    if (currentState == State::EnemyAttack) {
        engine.update(dt, win.getSize(), player);
        if (player.isDead()) { battleEnd = true; currentState = State::BattleEnded; }
        else if (engine.isBattleOver()) { currentState = State::PlayerMenu; }
    }

    float W = float(win.getSize().x);
    float H = float(win.getSize().y);
    float Hs = H * 0.5f, Hb = H * 0.25f;
    float barW = W * 0.8f, barH = Hb * 0.4f;
    float bx = W * 0.1f, by = Hs + (Hb - barH) / 2.f;

    if (hpBarBg && hpBarFg) {
        hpBarBg->setSize({ barW, barH });
        hpBarBg->setPosition({ bx, by });
        hpBarBg->setFillColor(enemy.getShields() > 0 ? sf::Color(130, 100, 100) : sf::Color::White);
        hpBarFg->setFillColor(enemy.getShields() > 0 ? sf::Color(100, 100, 100) : sf::Color::Red);

        float pct = float(enemy.getHp()) / float(enemy.getMaxHp());
        hpBarFg->setSize({ barW * pct, barH });
        hpBarFg->setPosition({ bx, by });
    }

    if (enemySpr) { enemySpr->setPosition({ W / 2.f, Hs / 2.f }); }
    if (shieldIcon && enemy.getShields() > 0) { shieldIcon->setPosition({ bx - barH, by + barH / 2.f }); }
}

void BattleScene::drawPlayerUI(sf::RenderTarget& t) const {
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

void BattleScene::draw(sf::RenderTarget& t, sf::RenderStates states) const {
    t.clear(sf::Color::Black);
    if (currentState == State::PlayerMenu || currentState == State::SkillsMenu) {
        if (enemySpr) t.draw(*enemySpr, states);
        if (hpBarBg) t.draw(*hpBarBg, states);
        if (hpBarFg) t.draw(*hpBarFg, states);
        if (shieldIcon && enemy.getShields() > 0) {
            t.draw(*shieldIcon, states);
            sf::Text count(font, std::to_string(enemy.getShields()), int(hpBarBg->getSize().y));
            count.setFillColor(sf::Color::White);
            auto ip = shieldIcon->getPosition();
            auto b = count.getLocalBounds();
            count.setPosition({ ip.x - (b.position.x + b.size.x / 2.f), ip.y - (b.position.y + b.size.y / 2.f) });
            t.draw(count, states);
        }
        drawPlayerUI(t);
    }
    else if (currentState == State::EnemyAttack) {
        t.draw(engine, states);
        t.draw(player, states);
    }
}

bool BattleScene::isOver() const { return battleEnd; }
bool BattleScene::enemyIsDead() const { return enemy.isDead(); }
Enemy::Type BattleScene::getEnemyType() { return enemy.getType(); }

SceneManager::SceneManager() : currentState(State::Map), player(), enemies(), map(), mapObjects() {
    s_instance = this;

    std::vector<sf::Vector2i> validSpawnPoints;
    do {
        map.regenerate();
        validSpawnPoints = map.findLargestConnectedArea();
    } while (validSpawnPoints.size() < (3 + 3 + 2));

    static std::mt19937_64 rng{ std::random_device{}() };
    std::shuffle(validSpawnPoints.begin(), validSpawnPoints.end(), rng);

    sf::Vector2i playerTile = validSpawnPoints.back(); validSpawnPoints.pop_back();
    player.setPosition({ playerTile.x * Map::tileSize + Map::tileSize * 0.5f, playerTile.y * Map::tileSize + Map::tileSize * 0.5f });

    sf::Vector2i stairsTile = validSpawnPoints.back(); validSpawnPoints.pop_back();
    map.setStairs(stairsTile.x, stairsTile.y);

    enemies.emplace_back(Enemy::Type::Basic); enemies.emplace_back(Enemy::Type::Basic); enemies.emplace_back(Enemy::Type::Basic);
    enemies.emplace_back(Enemy::Type::Ghost); enemies.emplace_back(Enemy::Type::Ghost);
    enemies.emplace_back(Enemy::Type::Boss);

    for (auto& enemy : enemies) {
        if (validSpawnPoints.empty()) break;
        sf::Vector2i enemyTile = validSpawnPoints.back(); validSpawnPoints.pop_back();
        enemy.setPosition({ enemyTile.x * Map::tileSize + Map::tileSize * 0.5f, enemyTile.y * Map::tileSize + Map::tileSize * 0.5f });
    }

    std::vector<MapObject::BuffType> buffsToPlace = { MapObject::BuffType::Health, MapObject::BuffType::Health , MapObject::BuffType::Attack, MapObject::BuffType::Attack, MapObject::BuffType::Defense };
    for (const auto& buffType : buffsToPlace) {
        if (validSpawnPoints.empty()) { std::cerr << "Warning: Ran out of valid spawn points for map objects!\n"; break; }
        sf::Vector2i objectTile = validSpawnPoints.back(); validSpawnPoints.pop_back();
        mapObjects.emplace_back(objectTile.x, objectTile.y, Map::tileSize, buffType);
    }

    if (!hudFont.openFromFile("game_over.ttf")) { throw std::runtime_error("FATAL ERROR: Could not load HUD font 'game_over.ttf'."); }

    // start 1st track
    audioManager.playMusic(AudioManager::MusicID::Exploration);
    switchTo(State::Map);
}

void SceneManager::addNotification(const std::string& message) {
    if (!notifications.empty() && notifications.front().text.getString() == message) {
        notifications.front().lifetime = 3.0f;
        return;
    }

    sf::Text text(hudFont, message, 48);
    text.setFillColor(sf::Color::White);
    notifications.push_front({ text, 3.0f });

    if (notifications.size() > 5) {
        notifications.pop_back();
    }
}

void SceneManager::pushNotification(const std::string& message) {
    if (s_instance) {
        s_instance->addNotification(message);
    }
}

void SceneManager::updateNotifications(float dt) {
    if (notifications.empty()) return;

    for (auto& notif : notifications) {
        notif.lifetime -= dt;
    }

    notifications.erase(
        std::remove_if(notifications.begin(), notifications.end(),
            [](const Notification& notif) {
                return notif.lifetime <= 0.f;
            }),
        notifications.end());
}

void SceneManager::drawNotifications(sf::RenderTarget& target) const {
    float y_pos = 70.f;
    const float margin = 15.f;
    const float window_width = static_cast<float>(target.getSize().x);

    for (const auto& notif : notifications) {
        sf::Text text = notif.text;

        sf::Color color = sf::Color::Yellow;
        if (notif.lifetime < 1.0f) {
            color.a = static_cast<std::uint8_t>(255 * notif.lifetime);
        }
        text.setFillColor(color);

        sf::FloatRect text_bounds = text.getLocalBounds();
        float x_pos = window_width - text_bounds.size.x - text_bounds.position.x - margin;
        text.setPosition({ x_pos, y_pos });

        target.draw(text);
        y_pos += text.getGlobalBounds().size.y + 25.f;
    }
}

void SceneManager::switchTo(State ns, Enemy* be, bool isVictory) {
    currentState = ns;
    if (ns == State::Map) {
        currentScene = std::make_unique<MapScene>(player, enemies, map, mapObjects, bossDefeated, audioManager);
        audioManager.playMusic(AudioManager::MusicID::Exploration);
    }
    else if (ns == State::Battle && be) {
        preBattlePos = player.getPosition();
        battleInitiated = true;
        currentScene = std::make_unique<BattleScene>(player, *be, audioManager);

        switch (be->getType()) {
        case Enemy::Type::Basic:
            audioManager.playMusic(AudioManager::MusicID::Battle_Basic);
            break;
        case Enemy::Type::Ghost:
            audioManager.playMusic(AudioManager::MusicID::Battle_Ghost);
            break;
        case Enemy::Type::Boss:
            audioManager.updateBossMusic(*be);
            break;
        }
    }
    else if (ns == State::GameOver) {
        currentScene = std::make_unique<GameOverScene>(player, isVictory);
        if (isVictory) {
            audioManager.playMusic(AudioManager::MusicID::GameOver_Victory, true);
        }
        else {
            audioManager.playMusic(AudioManager::MusicID::GameOver_Defeat, true);
        }
    }
}

void SceneManager::handleEvent(const sf::Event& ev) {
    if (currentScene) currentScene->handleEvent(ev);
}

void SceneManager::update(float dt, sf::RenderWindow& w, sf::View& gameView) {
    if (!currentScene) return;

    updateNotifications(dt);
    currentScene->update(dt, w);

    if (currentState == State::Map) {
        auto* ms = dynamic_cast<MapScene*>(currentScene.get());
        if (ms && ms->reachedStairs()) {
            switchTo(State::GameOver, nullptr, true);
            return;
        }
        if (ms && ms->triggeredBattle()) {
            switchTo(State::Battle, ms->getBattleEnemy());
            ms->resetBattleTrigger();
        }
    }
    else if (currentState == State::Battle) {
        auto* bs = dynamic_cast<BattleScene*>(currentScene.get());
        if (bs && bs->isOver()) {
            if (player.isDead()) {
                switchTo(State::GameOver);
            }
            else if (bs->enemyIsDead()) {
                player.setPosition(preBattlePos);
                if (bs->getEnemyType() == Enemy::Type::Boss) {
                    bossDefeated = true;
                    SceneManager::pushNotification("Boss defeated! The way is clear.");
                }
                switchTo(State::Map);
            }
            else {
                player.setPosition(preBattlePos);
                switchTo(State::Map);
                SceneManager::pushNotification("You fled from the battle!");
            }
        }
    }
    else if (currentState == State::GameOver) {
        auto* gs = dynamic_cast<GameOverScene*>(currentScene.get());
        if (gs && gs->isRespawnRequested()) {
            player.reset();
            for (auto& e : enemies) e.reset();
            sf::Vector2i playerTile = map.findLargestConnectedArea().front();
            player.setPosition({ playerTile.x * Map::tileSize + Map::tileSize * 0.5f, playerTile.y * Map::tileSize + Map::tileSize * 0.5f });
            switchTo(State::Map);
        }
    }

    if (currentState == State::Map) {
        sf::Vector2f targetCenter = player.getPosition();
        sf::Vector2f viewSize = gameView.getSize();
        sf::Vector2f halfViewSize = viewSize / 2.f;
        float mapWidthPixels = Map::width * Map::tileSize;
        float mapHeightPixels = Map::height * Map::tileSize;
        targetCenter.x = std::clamp(targetCenter.x, halfViewSize.x, mapWidthPixels - halfViewSize.x);
        targetCenter.y = std::clamp(targetCenter.y, halfViewSize.y, mapHeightPixels - halfViewSize.y);
        gameView.setCenter(targetCenter);
    }
    else {
        gameView.setCenter({ gameView.getSize().x / 2.f, gameView.getSize().y / 2.f });
    }
}

void SceneManager::draw(sf::RenderTarget& target, sf::View& gameView, sf::View& hudView) const {
    target.setView(gameView);
    if (currentScene) { target.draw(*currentScene); }

    target.setView(hudView);
    drawPlayerHUD(target);
    drawNotifications(target);
}

void SceneManager::drawPlayerHUD(sf::RenderTarget& target) const {
    std::string hudText = "HP: " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + " | Shields: " + std::to_string(player.getShields());
    sf::Text text(hudFont, hudText, 48);
    text.setFillColor(sf::Color::White);
    text.setPosition({ 15.f, -15.f });

    sf::RectangleShape bg;
    sf::FloatRect textBounds = text.getGlobalBounds();
    bg.setSize({ textBounds.size.x + 20.f, textBounds.size.y + 20.f });
    bg.setPosition({ 10.f, 10.f });
    bg.setFillColor({ 0, 0, 0, 150 });
    bg.setOutlineColor(sf::Color::White);
    bg.setOutlineThickness(1.f);

    target.draw(bg);
    target.draw(text);
}

void SceneManager::draw(sf::RenderTarget&, sf::RenderStates) const {
    assert(false); // Should not be called
}
