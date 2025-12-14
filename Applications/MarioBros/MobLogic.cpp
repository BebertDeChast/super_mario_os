#include "MobLogic.h"
#include <Applications/MarioBros/Movement.h>
#include <sextant/ordonnancements/preemptif/thread.h>
#include <sprites/GoombaSprite.h>

MobLogic::MobLogic(GameData* d, int w, int h) {
    data = d;
    width = w;
    height = h;
}

void MobLogic::run() {
    data->ps.ecrireMot("[MobLogic] Starting ...\n");

    // Initialize Level Map
    // Generate enemies across the level starting at x=200
    int spawnX = 200;
    for(int i=0; i<MAX_LEVEL_ENEMIES; i++) {
        levelEnemies[i].startX = spawnX;
        levelEnemies[i].startY = 50;
        levelEnemies[i].spawned = false;
        spawnX += (i % 2 == 0) ? 300 : 500;
    }

    // Initialize Active Slots
    for(int i=0; i<MAX_GOOMBAS; i++) {
        activeGoombas[i].active = false;
        activeGoombas[i].deathTimer = 0;
    }

    while(true) {
        data->run_mob.P();

        data->lock.P();
        if (data->resetGoomba) {
            // Reset logic if needed, for now just clear active
            for(int i=0; i<MAX_GOOMBAS; i++) activeGoombas[i].active = false;
            // Reset level spawn state
            for(int i=0; i<MAX_LEVEL_ENEMIES; i++) levelEnemies[i].spawned = false;
            data->resetGoomba = false;
        }
        
        // Handle Kill Signal
        if (data->killGoombaIndex != -1) {
            int idx = data->killGoombaIndex;
            if (idx >= 0 && idx < MAX_GOOMBAS && activeGoombas[idx].active) {
                activeGoombas[idx].deathTimer = 60; // 1 second flat
            }
            data->killGoombaIndex = -1;
        }
        
        int currentScrollX = data->scrollX;
        data->lock.V();

        // 1. SPAWN LOGIC
        // Check if any level enemy is within view (scrollX to scrollX + width)
        for(int i=0; i<MAX_LEVEL_ENEMIES; i++) {
            if (!levelEnemies[i].spawned) {
                // Spawn if it enters the screen (with some margin)
                if (levelEnemies[i].startX < currentScrollX + width + 50 && levelEnemies[i].startX > currentScrollX - 50) {
                    // Find a free slot
                    for(int j=0; j<MAX_GOOMBAS; j++) {
                        if (!activeGoombas[j].active) {
                            activeGoombas[j].active = true;
                            activeGoombas[j].x = levelEnemies[i].startX;
                            activeGoombas[j].y = levelEnemies[i].startY;
                            activeGoombas[j].vx = -1;
                            activeGoombas[j].vy = 0;
                            activeGoombas[j].deathTimer = 0;
                            levelEnemies[i].spawned = true;
                            break;
                        }
                    }
                }
            }
        }

        // 2. UPDATE LOGIC
        for(int i=0; i<MAX_GOOMBAS; i++) {
            if (activeGoombas[i].active) {
                if (activeGoombas[i].deathTimer > 0) {
                    activeGoombas[i].deathTimer--;
                    if (activeGoombas[i].deathTimer == 0) {
                        activeGoombas[i].active = false;
                    }
                } else {
                    update_goomba_position(activeGoombas[i].x, activeGoombas[i].y, activeGoombas[i].vx, activeGoombas[i].vy, width, height);
                    
                    // Despawn if too far left
                    if (activeGoombas[i].x < currentScrollX - 200) {
                        activeGoombas[i].active = false;
                    }
                }
            }
        }

        // 3. SYNC TO GAMEDATA
        data->lock.P();
        for(int i=0; i<MAX_GOOMBAS; i++) {
            data->goombas[i].x = activeGoombas[i].x;
            data->goombas[i].y = activeGoombas[i].y;
            data->goombas[i].active = activeGoombas[i].active;
            data->goombas[i].flat = (activeGoombas[i].deathTimer > 0);
            data->goombas[i].sprite = data->goombas[i].flat ? goombaFlatSpriteData : goombaSpriteData;
        }
        data->lock.V();
        
        thread_yield();
    }
}
