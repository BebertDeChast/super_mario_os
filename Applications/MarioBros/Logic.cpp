#include "Logic.h"
#include <sextant/ordonnancements/preemptif/thread.h>
#include <Applications/MarioBros/Movement.h>

extern Semaphore render_next_frame;

LogicThread::LogicThread(KeyboardData* k, GameData* d, int w, int h) {
    kbdData = k;
    data = d;
    width = w;
    height = h;
}

void LogicThread::resetMarioPosition() {
    data->lock.P();
    data->marioX = 50;
    data->marioY = 180;
    data->scrollX = 0;
    data->scrollY = 0;
    data->lock.V();
    reset_mario_physics();
}

bool LogicThread::checkCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2);
}


void LogicThread::run() {
    data->ps.ecrireMot("[Logic] Starting ...\n");
    
    lives = 3;
    invincibilityTimer = 0;
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
        render_next_frame.P();
        kbdData->lock.P();
        data->ps.ecrireMot("[Logic] Locked KeyboardData\n");
        bool wLeft = kbdData->wantLeft;
        bool wRight = kbdData->wantRight;
        bool wJump = kbdData->wantJump;
        
        // Reset inputs for next frame logic
        kbdData->wantLeft = false;
        kbdData->wantRight = false;
        kbdData->wantJump = false;
        kbdData->lock.V();
        data->ps.ecrireMot("[Logic] Unlocked KeyboardData\n");
        
        data->lock.P();
        data->ps.ecrireMot("[Logic] Locked GameData\n");
        int mx = data->marioX;
        int my = data->marioY;
        int sx = data->scrollX;
        int sy = data->scrollY;
        unsigned char *mSprite = data->marioSprite;
        data->lock.V();
        data->ps.ecrireMot("[Logic] Unlocked GameData\n");
        
        if (invincibilityTimer > 0) {
            invincibilityTimer--;
        }

        update_mario_position(mx, my, sx, sy, width, height, mSprite, wLeft, wRight, wJump);

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
            
            // Check collision with Mario
            // Adjusted hitbox: Mario (x+8, w=16) and Goomba (x+2, w=12) to ignore transparent pixels
            if (invincibilityTimer == 0 && checkCollision(mx + 8, my + 4, 16, 28, goomba.x + 2, goomba.y + 4, 12, 12)) {
                lives--;
                invincibilityTimer = 50; // Increased invincibility time
                 data->ps.ecrireMot("[Logic] Hit Goomba! Lives left: ");
                // Simple int to char conversion or just print dots
                for(int i=0; i<lives; i++) data->ps.ecrireMot("|");
                data->ps.ecrireMot("\n");

                if (lives <= 0) {
                    data->ps.ecrireMot("[Logic] GAME OVER\n");
                    while(true) thread_yield(); // Stop the game
                } else {
                    resetMarioPosition();
                    // Reset Goomba position to avoid spawn-kill loop
                    goomba.x = 200;
                    goomba.vx = -1;
                    
                    // Force local variables to reset position immediately
                    mx = 50;
                    my = 180;
                    sx = 0;
                    sy = 0;
                }
            }
        }

        data->lock.P();
        data->ps.ecrireMot("[Logic] Locked GameData\n");
        data->marioX = mx;
        data->marioY = my;
        data->scrollX = sx;
        data->scrollY = sy;
        data->marioSprite = mSprite;
        data->goombaX = goomba.x;
        data->goombaY = goomba.y;
        data->goombaActive = goomba.active;
        data->lock.V();
        data->ps.ecrireMot("[Logic] Unlocked GameData\n");
        
        thread_yield();
    }
}
