#ifndef BAT_H
#define BAT_H


#include "Unit.h"


class Bat : public Unit {
public:
    Bat(SDL_Renderer* renderer, Vector2D setPos);
    void draw(SDL_Renderer* renderer, int tileSize, Vector2D cameraPos) override;
    int getFrameCount() override; // Override getFrameCount
};

#endif // BAT_H
