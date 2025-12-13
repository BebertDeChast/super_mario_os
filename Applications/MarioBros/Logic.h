#pragma once
#include <sextant/Activite/Threads.h>
#include <Applications/Keyboard/Keyboard.h>
#include <sextant/Synchronisation/Semaphore/Semaphore.h>
#include <sprites/MarioSprites.h>
#include <Applications/GameData.h>

// Thread Logic : gère la logique.
class LogicThread : public Threads {
    KeyboardData* kbdData;
    GameData* data;
    int width, height;
    
public:
    int lives;
    int invincibilityTimer;
    void resetMarioPosition();
    LogicThread(KeyboardData* k, GameData* d, int w, int h);
    void run() override;
private:
    bool checkCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
};
