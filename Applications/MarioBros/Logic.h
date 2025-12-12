#pragma once
#include <sextant/Activite/Threads.h>
#include <Applications/Keyboard/Keyboard.h>
#include <sextant/Synchronisation/Semaphore/Semaphore.h>
#include <sextant/sprite.h>
#include <Applications/GameData.h>

// Thread Logic : gère la logique.
class LogicThread : public Threads {
    KeyboardData* kbdData;
    GameData* data;
    int width, height;
public:
    LogicThread(KeyboardData* k, GameData* d, int w, int h);
    void run() override;
};
