#pragma once
#include <sextant/Activite/Threads.h>
#include <Applications/GameData.h>

struct LevelEnemy {
    int startX, startY;
    bool spawned;
};

struct ActiveGoomba {
    int x, y;
    int vx, vy;
    int minX, maxX;
    bool active;
    int deathTimer;
};

class MobLogic : public Threads {
    GameData* data;
    int width, height;
    
    static const int MAX_LEVEL_ENEMIES = 30;
    LevelEnemy levelEnemies[MAX_LEVEL_ENEMIES];
    ActiveGoomba activeGoombas[MAX_GOOMBAS];

public:
    MobLogic(GameData* d, int w, int h);
    void run() override;
};
