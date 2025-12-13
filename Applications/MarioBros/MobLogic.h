#pragma once
#include <sextant/Activite/Threads.h>
#include <Applications/GameData.h>

struct Goomba {
    int x, y;
    int width, height;
    int vx, vy;
    int minX, maxX;
    bool active;
};

class MobLogic : public Threads {
    GameData* data;
    int width, height;
    Goomba goomba;
public:
    MobLogic(GameData* d, int w, int h);
    void run() override;
};
