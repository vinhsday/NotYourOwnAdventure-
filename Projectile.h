#pragma once
#include <memory>
#include "SDL.h"
#include "Vector2D.h"
#include "TextureLoader.h"
#include "Unit.h"

class Unit;

class Projectile
{
public:
    ~Projectile();

	Projectile(SDL_Renderer* renderer, Vector2D setPos, Vector2D setDirectionNormal);
	void update(float dT, std::vector<std::shared_ptr<Unit>>& listUnit);
	void draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos);
	bool getCollisionOccurred();


private:
	void checkCollisions(std::vector<std::shared_ptr<Unit>>& listUnits);


	Vector2D pos, directionNormal;
	static const float speed, size, distanceTraveledMax;
	float distanceTraveled = 0.0f;

	SDL_Texture* texture = nullptr;

	bool collisionOccurred = false;

private:
    int frame = 0;
    int frameCount = 11; // Giả sử cầu lửa có 4 frame
    float frameTime = 0.1f; // 0.1s mỗi frame
    float frameTimer = 0.0f;



};
