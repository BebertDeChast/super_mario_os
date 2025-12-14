#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <sextant/Synchronisation/Semaphore/Semaphore.h>
#include <sprites/MarioSprites.h>
#include <drivers/PortSerie.h>

struct GameData
{
    bool showWelcomeScreen;

    // Game State (Written by Logic, Read by Display)
    int marioX;
    int marioY;
    int scrollX;
    int scrollY;
    // pointeur vers le sprite Mario (droit ou gauche)
    unsigned char *marioSprite;

    // HUD
    bool showHUD;
    int score;
    int lives;
    bool gameOver;

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
