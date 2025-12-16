#include "Applications/MarioBros/Movement.h"
#include "Applications/MarioBros/LevelCollision.h"
#include "sprites/MarioSprites.h"

namespace {
    // --- CONSTANTES PHYSIQUES ---
    const int GRAVITY = 1; 
    const int JUMP_FORCE = -12; 
    
    // Inertie et Vitesse
    const int MAX_SPEED = 6;     
    const int ACCEL = 1;           
    const int FRICTION = 1;
    const int MAX_FALL_SPEED = 12;
    
    const int TILE_SIZE = 16;
    
    // Variables persistantes (état)
    int verticalVelocity = 0;
    int velocityX = 0;
    bool facingRight = true;            

    // --- LOGIQUE DE COLLISION ---
    bool is_solid(int x, int y) {
        int gridX = x / TILE_SIZE;
        int gridY = y / TILE_SIZE;
        if (gridX < 0 || gridX >= MAP_WIDTH || gridY < 0 || gridY >= MAP_HEIGHT) return false;
        return collision_map[gridY * MAP_WIDTH + gridX] != 0; 
    }

    bool check_collision(int x, int y, int w, int h) {
        // Coins
        if (is_solid(x, y)) return true;
        if (is_solid(x + w - 1, y)) return true;
        if (is_solid(x, y + h - 1)) return true;
        if (is_solid(x + w - 1, y + h - 1)) return true;
        
        // Milieux
        if (is_solid(x, y + h / 2)) return true;
        if (is_solid(x + w - 1, y + h / 2)) return true;
        if (is_solid(x + w / 2, y)) return true;
        if (is_solid(x + w / 2, y + h - 1)) return true;
        
        // Centre
        if (is_solid(x + w / 2, y + h / 2)) return true;
        return false;
    }

    
}

void reset_mario_physics() {
    verticalVelocity = 0;
    velocityX = 0;
    facingRight = true;
}

void update_goomba_position(int& x, int& y, int& vx, int& vy, int screenWidth, int screenHeight) {
    // Gravity
    vy += GRAVITY;
    if (vy > MAX_FALL_SPEED) vy = MAX_FALL_SPEED;

    // Horizontal Movement
    if (vx == 0) vx = -1; // Start moving left if stationary

    int nextX = x + vx;
    int nextY = y + vy;

    // Check for void ahead (smart goomba)
    // If grounded, check if next position has ground
    if (y > 170 && check_collision(x, y + 16, 16, 1)) {
        bool willFall = false;
        if (vx < 0) {
            if (!check_collision(nextX, y + 16, 1, 1)) willFall = true;
        } else if (vx > 0) {
            if (!check_collision(nextX + 15, y + 16, 1, 1)) willFall = true;
        }
        if (willFall) {
            vx = -vx;
            nextX = x;
        }
    }

    // Collision X
    if (check_collision(nextX, y, 16, 16)) {
        vx = -vx; // Reverse direction
    } else {
        x = nextX;
    }

    // Collision Y
    if (check_collision(x, nextY, 16, 16)) {
        if (vy > 0) { // Landing
            int blockBottomY = ((nextY + 16 - 1) / TILE_SIZE) * TILE_SIZE;
            y = blockBottomY - 16;
            vy = 0;
        } else {
            y = nextY;
        }
    } else {
        y = nextY;
    }
}

bool update_mario_position(int& x, int& y, int& scrollX, int& scrollY, int screenWidth, int screenHeight, unsigned char*& currentSprite, bool wantLeft, bool wantRight, bool wantJump) {
    int middleScreenX = scrollX + (screenWidth / 2 - MARIO_SPRITE_WIDTH);
    
    // Définition d'une hitbox plus étroite (10px) pour éviter de se coincer dans les murs
    int hitboxWidth = 10;
    int hitboxOffsetX = (MARIO_SPRITE_WIDTH - hitboxWidth) / 2;

    // 2. PHYSIQUE HORIZONTALE (Inertie)
    
    if (wantRight) {
        velocityX += ACCEL;
        facingRight = true;
    } else if (wantLeft) {
        velocityX -= ACCEL;
        facingRight = false;
    } else {
        // Friction (décélération naturelle si on n'appuie sur rien)
        if (velocityX > 0) {
            velocityX -= FRICTION;
            if (velocityX < 0) velocityX = 0;
        } else if (velocityX < 0) {
            velocityX += FRICTION;
            if (velocityX > 0) velocityX = 0;
        }
    }

    // Update sprite based on facing direction
    currentSprite = facingRight ? marioSpriteData : marioSpriteDataReversed;

    // Limitation de vitesse (Clamp)
    if (velocityX > MAX_SPEED) velocityX = MAX_SPEED;
    if (velocityX < -MAX_SPEED) velocityX = -MAX_SPEED;

    // Application du mouvement X
    int nextX = x + velocityX;

    // Prevent Mario from going past the end of the level
    if (nextX > MAP_WIDTH * TILE_SIZE - MARIO_SPRITE_WIDTH) {
        nextX = MAP_WIDTH * TILE_SIZE - MARIO_SPRITE_WIDTH;
        velocityX = 0;
    }

    // Collisions X
    if (check_collision(nextX + hitboxOffsetX, y, hitboxWidth, MARIO_SPRITE_HEIGHT)) {
        velocityX = 0; // On s'arrête net contre un mur
    } else {
        if (nextX < scrollX) {
            x = scrollX;
            velocityX = 0;
        } else {
            x = nextX;
        }
    }

    // Mise à jour Caméra
    if (x > middleScreenX) {
        scrollX = x - (screenWidth / 2 - MARIO_SPRITE_WIDTH);
    }

    // Stop scrolling at the end of the level
    if (scrollX > (MAP_WIDTH * TILE_SIZE) - screenWidth) {
        scrollX = (MAP_WIDTH * TILE_SIZE) - screenWidth;
    }

    // 3. PHYSIQUE VERTICALE
    // On utilise la même hitbox pour le sol pour rester cohérent avec les murs
    bool isGrounded = check_collision(x + hitboxOffsetX, y + 1, hitboxWidth, MARIO_SPRITE_HEIGHT);

    // Saut
    if (wantJump && isGrounded) {
        verticalVelocity = JUMP_FORCE;
        isGrounded = false;
    }

    // Gravité
    verticalVelocity += GRAVITY;
    if (verticalVelocity > MAX_FALL_SPEED) verticalVelocity = MAX_FALL_SPEED;

    int nextY = y + verticalVelocity;

    // Collisions Y
    if (verticalVelocity > 0) { // Chute
        if (check_collision(x + hitboxOffsetX, nextY, hitboxWidth, MARIO_SPRITE_HEIGHT)) {
            // Sol
            int blockBottomY = ((nextY + MARIO_SPRITE_HEIGHT - 1) / TILE_SIZE) * TILE_SIZE;
            y = blockBottomY - MARIO_SPRITE_HEIGHT;
            verticalVelocity = 0;
        } else {
            y = nextY;
        }
    } else if (verticalVelocity < 0) { // Montée
        if (check_collision(x + hitboxOffsetX, nextY, hitboxWidth, MARIO_SPRITE_HEIGHT)) {
            // Plafond
            int blockTopY = ((nextY) / TILE_SIZE + 1) * TILE_SIZE;
            y = blockTopY;
            verticalVelocity = 0;
        } else {
            y = nextY;
        }
    }
    int limitY = 207; // Calculé à la main

    if (y >= limitY) {
        return true;
    }
    return false;
}

void bounce_mario() {
    verticalVelocity = -8;
}
