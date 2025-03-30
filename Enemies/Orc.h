#ifndef ORC_H
#define ORC_H

#include "Unit.h"


class Orc : public Unit {
public:
    Orc(SDL_Renderer* renderer, Vector2D setPos);
};

#endif
