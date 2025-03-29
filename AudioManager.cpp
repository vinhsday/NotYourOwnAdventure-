#include "AudioManager.h"
#include <iostream>

std::map<std::string, Mix_Chunk*> AudioManager::soundEffects;
Mix_Music* AudioManager::backgroundMusic = nullptr;

void AudioManager::init() {
    int flags = MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_FLAC;
    int initted = Mix_Init(flags);

    if ((initted & flags) != flags) {
        std::cout << "❌ SDL_mixer init error: " << Mix_GetError() << std::endl;
    } else {
        std::cout << "✅ SDL_mixer format initialized.\n";
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "❌ SDL_mixer audio error: " << Mix_GetError() << std::endl;
    } else {
        std::cout << "✅ SDL_mixer audio initialized.\n";
    }
}


void AudioManager::close() {
    Mix_FreeMusic(backgroundMusic);
    for (auto& sound : soundEffects) {
        Mix_FreeChunk(sound.second);
    }
    soundEffects.clear();
    Mix_Quit();
}

void AudioManager::playMusic(const std::string& path, int loop) {
    // Giải phóng nhạc cũ nếu có
    if (backgroundMusic != nullptr) {
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }

    // Sử dụng Mix_LoadMUS thay vì Mix_LoadWAV
    backgroundMusic = Mix_LoadMUS(path.c_str());
    if (!backgroundMusic) {
        std::cout << "❌ Mix_LoadMUS Error: " << Mix_GetError() << std::endl;
        return;
    }

    // Phát nhạc nền với chế độ loop
    if (Mix_PlayMusic(backgroundMusic, loop) == -1) {
        std::cout << "❌ Mix_PlayMusic Error: " << Mix_GetError() << std::endl;
    } else {
        std::cout << "🎵 Đang phát nhạc: " << path << std::endl;
    }
}


void AudioManager::playSound(const std::string& file) {
    if (soundEffects.find(file) == soundEffects.end()) {
        Mix_Chunk* sound = Mix_LoadWAV(file.c_str());
        if (!sound) {
            std::cout << "⚠️ Failed to load sound: " << Mix_GetError() << std::endl;
            return;
        }
        soundEffects[file] = sound;
    }
    Mix_PlayChannel(-1, soundEffects[file], 0);
}

void AudioManager::setVolume(int volume) {
    Mix_VolumeMusic(volume);
}
