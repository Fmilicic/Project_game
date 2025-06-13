#pragma once
#include <SFML/Audio.hpp>
#include <map>
#include <string>
#include <list>
#include <optional>
#include "Entity.h"

class AudioManager {
public:
    // Enum to easily reference music tracks
    enum class MusicID {
        Exploration,
        Battle_Basic,
        Battle_Ghost,
        Battle_Boss_High,
        Battle_Boss_Mid,
        Battle_Boss_Low,
        GameOver_Victory,
        GameOver_Defeat
    };

    // Enum for sound effects
    enum class SoundID {
        BuffPickup
    };

    AudioManager();
    void playMusic(MusicID id, bool loop = true);
    void playSound(SoundID id);
    void stopMusic();

    void updateBossMusic(const Enemy& boss);

private:
    void loadMusic(MusicID id, const std::string& path);
    void loadSound(SoundID id, const std::string& path);

    sf::Music music;
    std::map<MusicID, std::string> musicFilepaths;
    std::map<SoundID, sf::SoundBuffer> soundBuffers;
    std::list<sf::Sound> sounds;

    std::optional<MusicID> currentMusic;
    int currentBossPhase = 0;
};

