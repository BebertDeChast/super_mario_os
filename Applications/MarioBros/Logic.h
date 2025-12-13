#pragma once
#include <sextant/Activite/Threads.h>
#include <Applications/Keyboard/Keyboard.h>
#include <sextant/Synchronisation/Semaphore/Semaphore.h>
#include <sextant/sprite.h>
#include <Applications/GameData.h>
struct Goomba {
    int x, y;
    int width, height;
    int vx, vy;
    int minX, maxX;
    bool active;
};

// Thread Logic : gère la logique.
class LogicThread : public Threads {
    KeyboardData* kbdData;
    GameData* data;
    int width, height;
    
public:
    Goomba goomba;
    int lives;
    int invincibilityTimer;
    void resetMarioPosition();
    LogicThread(KeyboardData* k, GameData* d, int w, int h);
    void run() override;
private:
    bool checkCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
};
