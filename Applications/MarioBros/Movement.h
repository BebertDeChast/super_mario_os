#ifndef APPLICATIONS_MARIOBROS_MOVEMENT_H
#define APPLICATIONS_MARIOBROS_MOVEMENT_H

void update_mario_position(int& x, int& y, int& scrollX, int& scrollY, int screenWidth, int screenHeight, unsigned char*& currentSprite, bool wantLeft, bool wantRight, bool wantJump);

#endif
