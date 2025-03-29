#ifndef BAT_H
#define BAT_H


#include "Unit.h"


class Bat : public Unit {
public:
    Bat(SDL_Renderer* renderer,Vector2D setPos);
};

#endif // BAT_H
