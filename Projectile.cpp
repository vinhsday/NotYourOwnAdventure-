#include "Projectile.h"
#include "iostream"


const float Projectile::speed = 10.0f, Projectile::size = 0.2f,
	Projectile::distanceTraveledMax = 50.0f;




Projectile::Projectile(SDL_Renderer* renderer, Vector2D setPos, Vector2D setDirectionNormal)
    : pos(setPos), directionNormal(setDirectionNormal) {

    texture = TextureLoader::loadTexture(renderer, "Fire.png");

    if (!texture) {
        std::cout << "Error: Failed to load Fire.png" << std::endl;
    } else {
        std::cout << "Successfully loaded Fire.png for projectile!" << std::endl;
    }
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

    for (auto& unit : listUnits) {
        if (unit && (unit->getPos() - pos).magnitude() < size) {
            unit->takeDamage(1);
            collisionOccurred = true;
            break;
        }
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

    std::cout << "🔥 Drawing projectile at: " << destRect.x << ", " << destRect.y << std::endl;
    SDL_RenderCopy(renderer, texture, &srcRect, &destRect);
}








bool Projectile::getCollisionOccurred() {
	return collisionOccurred;
}



void Projectile::checkCollisions(std::vector<std::shared_ptr<Unit>>& listUnits) {
	//Check for a collision with any of the units.
	if (collisionOccurred == false) {
		//Check if this overlaps any of the enemy units or not.
		for (int count = 0; count < listUnits.size() && collisionOccurred == false; count++) {
			auto& unitSelected = listUnits[count];
			if (unitSelected != nullptr && unitSelected->checkOverlap(pos, size)) {
				unitSelected->takeDamage(1);
				collisionOccurred = true;
			}
		}
	}
}
