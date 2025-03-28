#include "Effect.h"

FireEffect::FireEffect(SDL_Renderer* renderer, Vector2D position, float duration)
    : duration(duration), elapsedTime(0), currentFrame(0), totalFrames(6) // Giả sử có 4 frame lửa
{
    fireTexture = TextureLoader::loadTexture(renderer, "fire_sprite.png"); // Tải sprite lửa
    if (!fireTexture) {
        std::cout << "⚠️ Failed to load fire sprite!" << std::endl;
    }

    // Khởi tạo các rectangle để vẽ sprite
    srcRect = {0, 0, 64, 64};  // Kích thước mỗi frame là 64x64 (có thể thay đổi)
    dstRect = {position.x, position.y, 64, 64}; // Vị trí hiển thị trên màn hình
}

void Effect::update(float dT) {
    elapsedTime += dT;

    if (elapsedTime >= duration) {
        // Kết thúc hiệu ứng khi hết thời gian
        return;
    }

    // Cập nhật frame của sprite
    currentFrame = (int)((elapsedTime / duration) * totalFrames);
    srcRect.x = currentFrame * 64;  // Cập nhật frame trên sprite sheet
}

void Effect::draw(SDL_Renderer* renderer) {
    if (elapsedTime >= duration) return;

    // Vẽ sprite
    SDL_RenderCopy(renderer, fireTexture, &srcRect, &dstRect);
}
