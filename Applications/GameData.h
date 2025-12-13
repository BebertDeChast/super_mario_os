#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <sextant/Synchronisation/Semaphore/Semaphore.h>
#include <sextant/sprite.h>
#include <drivers/PortSerie.h>

struct GameData
{
    // Game State (Written by Logic, Read by Display)
    int marioX;
    int marioY;
    int scrollX;
    int scrollY;
    // pointeur vers le sprite Mario (droit ou gauche)
    unsigned char *marioSprite;

    // Goomba State
    int goombaX;
    int goombaY;
    bool goombaActive;

    Semaphore run_mob;
    bool resetGoomba;
    Semaphore lock;
    PortSerie ps;
};

#endif
