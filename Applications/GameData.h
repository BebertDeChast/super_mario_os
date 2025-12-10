#include <semaphore.h>

struct GameData
{
    // Game State (Written by Logic, Read by Display)
    int marioX;
    int marioY;
    int scrollX;
    int scrollY;
    // pointeur vers le sprite Mario (droit ou gauche)
    unsigned char *marioSprite;

    Semaphore lock;
};