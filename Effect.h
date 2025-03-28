#ifndef EFFECT_H
#define EFFECT_H
#include <SDL.h>
#include "TextureLoader.h" // Đảm bảo bạn có lớp này để tải texture

class Effect {
public:
    Effect(SDL_Renderer* renderer, Vector2D position, float duration);
    void update(float dT);
    void draw(SDL_Renderer* renderer);

private:
    SDL_Texture* fireTexture;  // Texture chứa sprite lửa
    SDL_Rect srcRect;          // Hình chữ nhật của từng frame sprite
    SDL_Rect dstRect;          // Vị trí hiển thị trên màn hình
    float duration;            // Thời gian hiệu ứng tồn tại
    float elapsedTime;         // Thời gian đã trôi qua
    int currentFrame;          // Chỉ số frame hiện tại
    int totalFrames;           // Tổng số frame trong sprite sheet
};

#endif // EFFECT_H
