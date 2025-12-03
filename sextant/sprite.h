#ifndef SPRITE_H
#define SPRITE_H

// Généré par png_to_c_with_palette.py à partir de 'pixel-32x32.png'
#define MARIO_SPRITE_WIDTH  32
#define MARIO_SPRITE_HEIGHT 32

extern unsigned char palette_vga[256][3];
extern unsigned char marioSpriteData[MARIO_SPRITE_WIDTH*MARIO_SPRITE_HEIGHT];
extern unsigned char marioSpriteDataReversed[MARIO_SPRITE_WIDTH*MARIO_SPRITE_HEIGHT];

#endif