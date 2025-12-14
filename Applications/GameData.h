#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <sextant/Synchronisation/Semaphore/Semaphore.h>
#include <sprites/MarioSprites.h>
#include <drivers/PortSerie.h>

#define MAX_GOOMBAS 20

struct GoombaState {
    int x, y;
    bool active;
    bool flat;
    unsigned char *sprite;
};

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
    GoombaState goombas[MAX_GOOMBAS];

    Semaphore run_mob;
    bool resetGoomba;
    int killGoombaIndex; // -1 if none
    Semaphore lock;
    PortSerie ps;
};

#endif
