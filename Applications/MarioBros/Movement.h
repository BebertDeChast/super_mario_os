#ifndef APPLICATIONS_MARIOBROS_MOVEMENT_H
#define APPLICATIONS_MARIOBROS_MOVEMENT_H


bool update_mario_position(int& x, int& y, int& scrollX, int& scrollY, int screenWidth, int screenHeight, unsigned char*& currentSprite, bool wantLeft, bool wantRight, bool wantJump);
void update_goomba_position(int& x, int& y, int& vx, int& vy, int screenWidth, int screenHeight);
void reset_mario_physics();
void bounce_mario();

#endif
