#ifndef VAMPIRE_H
#define VAMPIRE_H
#include "Unit.h"

class Vampire : public Unit
{
public:
    Vampire(SDL_Renderer* renderer, Vector2D setPos);
};

#endif // VAMPIRE_H
