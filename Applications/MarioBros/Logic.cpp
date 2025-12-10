#include "Logic.h"
#include <sextant/ordonnancements/preemptif/thread.h>
#include <Applications/MarioBros/Movement.h>

LogicThread::LogicThread(KeyboardData* k, SharedData* d, int w, int h) {
    kbdData = k;
    data = d;
    width = w;
    height = h;
}

void LogicThread::run() {
    while(true) {
        kbdData->lock.P();
        bool wLeft = kbdData->wantLeft;
        bool wRight = kbdData->wantRight;
        bool wJump = kbdData->wantJump;
        
        // Reset inputs for next frame logic
        kbdData->wantLeft = false;
        kbdData->wantRight = false;
        kbdData->wantJump = false;
        kbdData->lock.V();
        
        data->lock.P();
        int mx = data->marioX;
        int my = data->marioY;
        int sx = data->scrollX;
        int sy = data->scrollY;
        unsigned char *mSprite = data->marioSprite;
        data->lock.V();

        // Run physics
        update_mario_position(mx, my, sx, sy, width, height, mSprite, wLeft, wRight, wJump);

        data->lock.P();
        data->marioX = mx;
        data->marioY = my;
        data->scrollX = sx;
        data->scrollY = sy;
        data->marioSprite = mSprite;
        data->lock.V();
        
        thread_yield();
    }
}
