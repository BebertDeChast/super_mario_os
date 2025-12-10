#pragma once
#include <sextant/Activite/Threads.h>
#include <Applications/Keyboard/Keyboard.h>
#include <sextant/Synchronisation/Semaphore/Semaphore.h>
#include <sextant/sprite.h>

struct SharedData {
    // Game State (Written by Logic, Read by Display)
    int marioX = 32;
    int marioY = 100;
    int scrollX = 0;
    int scrollY = 0;

    unsigned char *marioSprite = marioSpriteData;

    Semaphore lock;
};

// Thread Logic : gère la logique.
class LogicThread : public Threads {
    KeyboardData* kbdData;
    SharedData* data;
    int width, height;
public:
    LogicThread(KeyboardData* k, SharedData* d, int w, int h);
    void run() override;
};
