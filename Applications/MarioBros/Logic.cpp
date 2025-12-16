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
    
    // Wait for user input on title screen
    while (true) {
        data->lock.P();
        bool show = data->showWelcomeScreen;
        data->lock.V();

        if (!show) break;

        render_next_frame.P(); // Wait for next frame tick

        kbdData->lock.P();
        bool pressed = kbdData->wantLeft || kbdData->wantRight || kbdData->wantJump;
        // Consume inputs to prevent immediate movement
        kbdData->wantLeft = false;
        kbdData->wantRight = false;
        kbdData->wantJump = false;
        kbdData->lock.V();

        if (pressed) {
            data->lock.P();
            data->showWelcomeScreen = false;
            data->lock.V();
        }
    }

    data->lives = 3;
    invincibilityTimer = 0;

    data->lock.P();
    data->score = 0;
    data->lock.V();

    while(true) {
        render_next_frame.P();
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
        // Read Goomba state for collision
        data->lock.V();
        // data->ps.ecrireMot("[Logic] Unlocked GameData\n");
        
        if (invincibilityTimer > 0) {
            invincibilityTimer--;
        }

        int prevMy = my; // Capture previous Y to detect falling direction
        if (update_mario_position(mx, my, sx, sy, width, height, mSprite, wLeft, wRight, wJump)) {
            data->ps.ecrireMot("[Logic] Mario fell in a hole!\n");
            data->lives--;

            if (data->lives <= 0) {
                data->ps.ecrireMot("[Logic] GAME OVER\n");
                data->gameOver = true;
                thread_exit();
            } else {
                resetMarioPosition();
                data->lock.P();
                data->resetGoomba = true;
                data->lock.V();
                
                mx = 50;
                my = 180;
                sx = 0;
                sy = 0;
            }
        }

        // Check for flag (Win condition) - Position approx du drapeau (198 * 16)
        if (mx >= 3168) {
            data->ps.ecrireMot("[Logic] YOU WIN!\n");
            data->lock.P();
            data->gameFinished = true;
            data->lock.V();
            thread_exit();
        }

        // Signal MobLogic to run
        data->run_mob.V();

        // Check collision with all active Goombas
        for (int i = 0; i < MAX_GOOMBAS; i++) {
            data->lock.P();
            bool gActive = data->goombas[i].active;
            bool gFlat = data->goombas[i].flat;
            int gx = data->goombas[i].x;
            int gy = data->goombas[i].y;
            data->lock.V();

            if (gActive && !gFlat) {
                // Check collision with Mario
                // Adjusted hitbox: Mario (x+8, w=16) and Goomba (x+2, w=12)
                if (checkCollision(mx + 8, my + 4, 16, 28, gx + 2, gy + 4, 12, 12)) {
                    // Check for stomp
                    if (prevMy + 32 <= gy + 12) {
                        data->ps.ecrireMot("[Logic] Stomped Goomba!\n");
                        data->lock.P();
                        data->killGoombaIndex = i;
                        data->goombas[i].flat = true;
                        data->score += 100;
                        data->lock.V();
                        bounce_mario();
                    } else if (invincibilityTimer == 0) {
                        data->lives--;
                        data->lock.P();
                        data->lock.V();
                        invincibilityTimer = 50;
                        data->ps.ecrireMot("[Logic] Hit Goomba!\n");

                        if (data->lives <= 0) {
                            data->ps.ecrireMot("[Logic] GAME OVER\n");
                            data->gameOver = true;
                            thread_exit();
                        } else {
                            resetMarioPosition();
                            data->lock.P();
                            data->resetGoomba = true;
                            data->lock.V();
                            
                            mx = 50;
                            my = 180;
                            sx = 0;
                            sy = 0;
                        }
                    }
                    // Break after one collision to avoid double hits in same frame
                    break;
                }
            }
        }

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
