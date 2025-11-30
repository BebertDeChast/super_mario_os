#include "Applications/MarioBros/Movement.h"
#include "drivers/Clavier.h"
#include "sextant/sprite.h"

namespace {
	Clavier keyboard;
	int verticalVelocity = 0;
	const int gravity = 2;
	const int jumpImpulse = -20;
}

void update_mario_position(int& x, int& y, int& scrollX, int& scrollY, int screenWidth, int screenHeight, bool& isRight) {
	int groundY = screenHeight - SPRITE_HEIGHT;		// Ground level
	int middleScreenX = scrollX + (screenWidth / 2 - SPRITE_WIDTH);

	// Movement
	if (keyboard.testChar()) {
		char key = keyboard.getchar();
		if (key == 'q' || key == 'Q') {
			x -= 10;
			isRight = false;
		} else if (key == 'd' || key == 'D') {
			x += 10;
			if (x > middleScreenX) {
				scrollX += 10;
			}
			isRight = true;
		} else if ((key == 'z' || key == 'Z') && y >= groundY) {
			verticalVelocity = jumpImpulse;
		}
	}

	// Prevent Mario from going out of the level
	if (x < scrollX) x = scrollX;

	// Vertical physics (jump + gravity)
	y += verticalVelocity;
	if (y < groundY) {
		verticalVelocity += gravity;
	} else {
		y = groundY;
		verticalVelocity = 0;
	} 
}
