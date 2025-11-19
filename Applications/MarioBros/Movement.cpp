#include "Applications/MarioBros/Movement.h"
#include "drivers/Clavier.h"
#include "sextant/sprite.h"

namespace {
	Clavier keyboard;
	int verticalVelocity = 0;
	const int gravity = 2;
	const int jumpImpulse = -20;
}

void update_mario_position(int& x, int& y, int screenWidth, int screenHeight, bool& isRight) {
	int groundY = screenHeight - SPRITE_HEIGHT;		// Ground level

	// Movement
	if (keyboard.testChar()) {
		char key = keyboard.getchar();
		if (key == 'q' || key == 'Q') {
			x -= 10;
			isRight = false;
		} else if (key == 'd' || key == 'D') {
			x += 10;
			isRight = true;
		} else if ((key == 'z' || key == 'Z') && y >= groundY) {
			verticalVelocity = jumpImpulse;
		}
	}

	// Prevent Mario from going more than the middle of the screen
	int maxX = screenWidth / 2 - SPRITE_WIDTH;
	if (x < 0) x = 0;
	if (x > maxX) x = maxX;

	// Vertical physics (jump + gravity)
	y += verticalVelocity;
	if (y < groundY) {
		verticalVelocity += gravity;
	} else {
		y = groundY;
		verticalVelocity = 0;
	} 
}
