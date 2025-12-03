#include "Applications/MarioBros/Movement.h"
#include "Applications/Level/LevelCollision.h"
#include "drivers/Clavier.h"
#include "sextant/sprite.h"

namespace {
    Clavier keyboard;
    
    // --- CONSTANTES PHYSIQUES ---
    const int GRAVITY = 1; 
    const int JUMP_FORCE = -17; 
    
    // Inertie et Vitesse
    const int MAX_SPEED = 200;     
    const int ACCEL = 4;           
    const int FRICTION = 2;
    const int MAX_FALL_SPEED = 12;
    
    const int TILE_SIZE = 16;
    
    // Variables persistantes (état)
    int verticalVelocity = 0;
    int velocityX = 0;            

    // --- LOGIQUE DE COLLISION ---
    bool is_solid(int x, int y) {
        int gridX = x / TILE_SIZE;
        int gridY = y / TILE_SIZE;
        if (gridX < 0 || gridX >= MAP_WIDTH || gridY < 0 || gridY >= MAP_HEIGHT) return false;
        return collision_map[gridY * MAP_WIDTH + gridX] != 0; 
    }

    bool check_collision(int x, int y) {
        int w = MARIO_SPRITE_WIDTH;
        int h = MARIO_SPRITE_HEIGHT;
        
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

void update_mario_position(int& x, int& y, int& scrollX, int& scrollY, int screenWidth, int screenHeight, bool& isRight) {
    int middleScreenX = scrollX + (screenWidth / 2 - MARIO_SPRITE_WIDTH);
    
    // 1. INPUTS
    bool wantLeft = false;
    bool wantRight = false;
    bool wantJump = false;

    while (keyboard.testChar()) {
        char key = keyboard.getchar();
        if (key == 'q' || key == 'Q') wantLeft = true;
        if (key == 'd' || key == 'D') wantRight = true;
        if (key == 'z' || key == 'Z') wantJump = true;
    }

    // 2. PHYSIQUE HORIZONTALE (Inertie)
    
    if (wantRight) {
        velocityX += ACCEL;
        isRight = true;
    } else if (wantLeft) {
        velocityX -= ACCEL;
        isRight = false;
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

    // Limitation de vitesse (Clamp)
    if (velocityX > MAX_SPEED) velocityX = MAX_SPEED;
    if (velocityX < -MAX_SPEED) velocityX = -MAX_SPEED;

    // Application du mouvement X
    int nextX = x + velocityX;

    // Collisions X
    if (check_collision(nextX, y)) {
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

    // 3. PHYSIQUE VERTICALE
    bool isGrounded = check_collision(x, y + 1);

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
        if (check_collision(x, nextY)) {
            // Sol
            int blockBottomY = ((nextY + MARIO_SPRITE_HEIGHT - 1) / TILE_SIZE) * TILE_SIZE;
            y = blockBottomY - MARIO_SPRITE_HEIGHT;
            verticalVelocity = 0;
        } else {
            y = nextY;
        }
    } else if (verticalVelocity < 0) { // Montée
        if (check_collision(x, nextY)) {
            // Plafond
            int blockTopY = ((nextY) / TILE_SIZE + 1) * TILE_SIZE;
            y = blockTopY;
            verticalVelocity = 0;
        } else {
            y = nextY;
        }
    }

    // Respawn chute
    if (y > screenHeight + 64) {
        y = 0;
        verticalVelocity = 0;
        velocityX = 0; // Reset inertie si on meurt
    }
}