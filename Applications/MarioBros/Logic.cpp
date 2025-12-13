#include "Logic.h"
#include <sextant/ordonnancements/preemptif/thread.h>
#include <Applications/MarioBros/Movement.h>

LogicThread::LogicThread(KeyboardData* k, GameData* d, int w, int h) {
    kbdData = k;
    data = d;
    width = w;
    height = h;
}

void LogicThread::run() {
    data->ps.ecrireMot("[Logic] Starting ...\n");
    while(true) {
        kbdData->lock.P();
        // data->ps.ecrireMot("[Logic] Locked KeyboardData\n");
        bool wLeft = kbdData->wantLeft;
        bool wRight = kbdData->wantRight;
        bool wJump = kbdData->wantJump;
        
        // Reset inputs for next frame logic
        kbdData->wantLeft = false;
        kbdData->wantRight = false;
        kbdData->wantJump = false;
        kbdData->lock.V();
        // data->ps.ecrireMot("[Logic] Unlocked KeyboardData\n");
        
        data->lock.P();
        // data->ps.ecrireMot("[Logic] Locked GameData\n");
        int mx = data->marioX;
        int my = data->marioY;
        int sx = data->scrollX;
        int sy = data->scrollY;
        unsigned char *mSprite = data->marioSprite;
        data->lock.V();
        // data->ps.ecrireMot("[Logic] Unlocked GameData\n");
        

        // Run physics
        update_mario_position(mx, my, sx, sy, width, height, mSprite, wLeft, wRight, wJump);

        data->lock.P();
        // data->ps.ecrireMot("[Logic] Locked GameData\n");
        data->marioX = mx;
        data->marioY = my;
        data->scrollX = sx;
        data->scrollY = sy;
        data->marioSprite = mSprite;
        data->lock.V();
        // data->ps.ecrireMot("[Logic] Unlocked GameData\n");
        
        thread_yield();
    }
}
