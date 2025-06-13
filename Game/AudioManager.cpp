#include "AudioManager.h"
#include <stdexcept>

AudioManager::AudioManager() {
    // Associate each MusicID with its file path
    // NOTE: You must have an 'assets/audio' folder with these files!
    loadMusic(MusicID::Exploration, "assets/audio/exploration.mp3");
    loadMusic(MusicID::Battle_Basic, "assets/audio/battle_basic.mp3");
    loadMusic(MusicID::Battle_Ghost, "assets/audio/battle_ghost.mp3");
    loadMusic(MusicID::Battle_Boss_High, "assets/audio/boss_phase1.mp3");
    loadMusic(MusicID::Battle_Boss_Mid, "assets/audio/boss_phase2.mp3");
    loadMusic(MusicID::Battle_Boss_Low, "assets/audio/boss_phase3.ogg");
    loadMusic(MusicID::GameOver_Victory, "assets/audio/victory.mp3");
    loadMusic(MusicID::GameOver_Defeat, "assets/audio/defeat.mp3");

    // Pre-load all sound effects into buffers
    loadSound(SoundID::BuffPickup, "assets/audio/buff_pickup.mp3");
}

void AudioManager::loadMusic(MusicID id, const std::string& path) {
    musicFilepaths[id] = path;
}

void AudioManager::loadSound(SoundID id, const std::string& path) {
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(path)) {
        throw std::runtime_error("FATAL ERROR: Failed to load sound: " + path);
    }
    soundBuffers[id] = buffer;
}

void AudioManager::playMusic(MusicID id, bool loop) {
    // Don't restart the music if the same track is already playing
    if (currentMusic.has_value() && currentMusic.value() == id && music.getStatus() == sf::Music::Status::Playing) {
        return;
    }

    std::string path = musicFilepaths.at(id);
    if (!music.openFromFile(path)) {
        throw std::runtime_error("FATAL ERROR: Failed to open music file: " + path);
    }

    music.setLooping(loop);
    music.play();
    currentMusic = id;
}

void AudioManager::playSound(SoundID id) {
    sounds.emplace_back(soundBuffers.at(id));
    sounds.back().play();

    sounds.remove_if([](const sf::Sound& s) {
        return s.getStatus() == sf::Sound::Status::Stopped;
        });
}

void AudioManager::stopMusic() {
    music.stop();
    currentMusic.reset();
}

void AudioManager::updateBossMusic(const Enemy& boss) {
    float hpPercent = static_cast<float>(boss.getHp()) / boss.getMaxHp();
    int newPhase = 0;

    if (hpPercent > 0.50f)       newPhase = 1;
    else if (hpPercent > 0.25f)  newPhase = 2;
    else                         newPhase = 3;

    if (newPhase != currentBossPhase) {
        currentBossPhase = newPhase;
        switch (currentBossPhase) {
        case 1: playMusic(MusicID::Battle_Boss_High); break;
        case 2: playMusic(MusicID::Battle_Boss_Mid);  break;
        case 3: playMusic(MusicID::Battle_Boss_Low);  break;
        }
    }
}