#include "MobLogic.h"
#include <Applications/MarioBros/Movement.h>
#include <sextant/ordonnancements/preemptif/thread.h>

MobLogic::MobLogic(GameData* d, int w, int h) {
    data = d;
    width = w;
    height = h;
}

void MobLogic::run() {
    data->ps.ecrireMot("[MobLogic] Starting ...\n");

    goomba.x = 200;
    goomba.y = 180;
    goomba.width = 16;
    goomba.height = 16;
    goomba.active = true;
    goomba.vx = -1;
    goomba.vy = 0;
    goomba.minX = 100;
    goomba.maxX = 300;

    while(true) {
        data->run_mob.P();

        data->lock.P();
        if (data->resetGoomba) {
            goomba.x = 200;
            goomba.vx = -1;
            data->resetGoomba = false;
        }
        data->lock.V();

        if (goomba.active) {
            update_goomba_position(goomba.x, goomba.y, goomba.vx, goomba.vy, width, height);

            // Patrol logic
            if (goomba.x <= goomba.minX) {
                goomba.x = goomba.minX;
                goomba.vx = 1;
            } else if (goomba.x >= goomba.maxX) {
                goomba.x = goomba.maxX;
                goomba.vx = -1;
            }
        }

        data->lock.P();
        data->goombaX = goomba.x;
        data->goombaY = goomba.y;
        data->goombaActive = goomba.active;
        data->lock.V();
        
        thread_yield();
    }
}
