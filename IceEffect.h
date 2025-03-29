#pragma once
#include "SDL.h"
#include "Vector2D.h"
#include "TextureLoader.h"
#include "Timer.h"
#include <vector>
#include "Unit.h"

enum class IceState {
    Appear,    // Trạng thái xuất hiện
    Active,    // Trạng thái hoạt động (đóng băng quái)
    Disappear  // Trạng thái biến mất
};

class IceEffect {
public:
    IceEffect(SDL_Renderer* renderer, Vector2D setPos);
    void update(float dT, std::vector<std::shared_ptr<Unit>>& listUnits);
    void draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos);
    bool isFinished()  { return state == IceState::Disappear && durationTimer.timeSIsZero(); }

private:
    Vector2D pos;
    IceState state = IceState::Appear;

    SDL_Texture* textureAppear = nullptr;    // Sprite sheet khi xuất hiện
    SDL_Texture* textureActive = nullptr;    // Sprite sheet khi hoạt động
    SDL_Texture* textureDisappear = nullptr; // Sprite sheet khi biến mất

    Timer durationTimer = Timer(0.5f); // Thời gian mỗi trạng thái (0.5s x 3 = 1.5s tổng)
    Timer freezeTimer = Timer(2.0f);   // Thời gian đóng băng quái (2 giây)
    float frameTime = 0.1f;            // Mỗi frame 0.1 giây
    float frameTimer = 0.0f;
    int frame = 0;
    int frameCount = 8;                // Giả sử mỗi sprite sheet có 6 frame
    int frameWidth = 256/8;          // 480px chia 6 = 80px mỗi frame
    int frameHeight = 32;              // Chiều cao sprite sheet (48px)

    float radius = 100.0f;               // Phạm vi đóng băng
    std::vector<std::shared_ptr<Unit>> frozenUnits; // Danh sách quái bị đóng băng
};
