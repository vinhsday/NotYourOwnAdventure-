#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <SDL_mixer.h>
#include <map>
#include <string>

class AudioManager {
public:
    static void init();               // Khởi tạo âm thanh
    static void close();              // Giải phóng tài nguyên
    static void playMusic(const std::string& file, int loops = -1);  // Phát nhạc nền
    static void playSound(const std::string& file);                  // Phát hiệu ứng
    static void setVolume(int volume);  // Điều chỉnh âm lượng

private:
    static std::map<std::string, Mix_Chunk*> soundEffects;
    static Mix_Music* backgroundMusic;
};

#endif
