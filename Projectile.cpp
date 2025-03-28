#include "Projectile.h"
#include "iostream"


const float Projectile::speed = 10.0f, Projectile::size = 0.2f,
	Projectile::distanceTraveledMax = 50.0f;




Projectile::Projectile(SDL_Renderer* renderer, Vector2D setPos, Vector2D setDirectionNormal)
    : pos(setPos), directionNormal(setDirectionNormal) {

    texture = TextureLoader::loadTexture(renderer, "Fire.png");

}


Projectile::~Projectile() {
    if (texture) {
        SDL_DestroyTexture(texture);
    }
}



void Projectile::update(float dT, std::vector<std::shared_ptr<Unit>>& listUnits) {
    if (collisionOccurred) return;

    pos = pos + (directionNormal * dT);  // ✅ Không nhân lại speed nữa


    frameTimer += dT;
    if (frameTimer >= frameTime) {
        frameTimer = 0.0f;
        frame = (frame + 1) % frameCount;
    }

    if (distanceTraveled >= distanceTraveledMax) {
        collisionOccurred = true;
    }



}






void Projectile::draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos) { // Thêm cameraPos
    if (!texture || collisionOccurred) return;

    int frameWidth = 352 / 11;
    int frameHeight = 32;

    SDL_Rect srcRect = { frame * frameWidth, 0, frameWidth, frameHeight };
    SDL_Rect destRect = { (int)(pos.x * tileSize) - frameWidth / 2 - (int)(cameraPos.x * tileSize),
                      (int)(pos.y * tileSize) - frameHeight / 2 - (int)(cameraPos.y * tileSize),
                      frameWidth, frameHeight };

    SDL_RenderCopy(renderer, texture, &srcRect, &destRect);
}








bool Projectile::getCollisionOccurred() {
	return collisionOccurred;
}




