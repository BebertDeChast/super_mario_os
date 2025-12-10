#include "GameDisplay.h"
#include <Applications/GameData.h>
#include <Applications/Level/Level_display_data.h>
#include <sextant/sprite.h> // for palette_vga

GameDisplay::GameDisplay(GameData *data)
{
    g = data;
}

void GameDisplay::run()
{
    EcranBochs display(720, 240, LEVEL_WIDTH, VBE_MODE::_8);
    display.init();
    display.clear(0);
    display.set_palette(palette_vga);

    display.paint_picture(level_sprite_indices, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT);

    g->lock.P();
    int oldX = g->marioX;
    int oldY = g->marioY;
    int oldScrollX = g->scrollX;
    g->lock.V();

    display.set_offset(oldScrollX, 0);

    while (true)
    {
        g->lock.P();
        if (oldX != g->marioX || oldY != g->marioY)
        {
            display.plot_moving_sprite(g->marioSprite,
                                       MARIO_SPRITE_WIDTH, MARIO_SPRITE_HEIGHT,
                                       g->marioX, g->marioY,
                                       oldX, oldY,
                                       level_sprite_indices);
            oldX = g->marioX;
            oldY = g->marioY;
            display.set_offset(g->scrollX, 0);
            oldScrollX = g->scrollX;
        }
        g->lock.V();
    }
}
