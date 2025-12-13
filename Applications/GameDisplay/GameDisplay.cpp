#include "GameDisplay.h"
#include <Applications/GameData.h>
#include <Applications/GameDisplay/Level_display_data.h>
#include <sextant/sprite.h> // for palette_vga
#include <Applications/GameDisplay/spritesText.h>

GameDisplay::GameDisplay(GameData *data) : display(720, 240, LEVEL_WIDTH, VBE_MODE::_8)
{
    g = data;
}

void GameDisplay::run()
{
    g->ps.ecrireMot("[GameDisplay] Starting ...\n");
    display.init();
    display.clear(0);
    display.set_palette(palette_vga);

    display.paint_picture(level_sprite_indices, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT);

    g->lock.P();
    g->ps.ecrireMot("[GameDisplay] Locked\n");
    int oldX = g->marioX;
    int oldY = g->marioY;
    int oldScrollX = g->scrollX;
    unsigned char *initSprite = g->marioSprite;
    g->lock.V();
    g->ps.ecrireMot("[GameDisplay] Unlocked\n");

    display.plot_sprite(initSprite,
                        MARIO_SPRITE_WIDTH, MARIO_SPRITE_HEIGHT,
                        oldX, oldY);

    display.set_offset(oldScrollX, 0);
    afficherHUD(true);

    while (true)
    {
        g->lock.P();
        g->ps.ecrireMot("[GameDisplay] Locked in loop\n");
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
        g->ps.ecrireMot("[GameDisplay] Unlocked in loop\n");
        thread_yield();
    }
}

void GameDisplay::afficherHUD(bool init)
{
    // Implémentation de l'affichage du HUD
    // Le HUD commence à la position (30, 30) de l'écran affiché (/= VRAM)
    // il affiche le mot "MARIO" puis le score sur 6 chiffres en dessous

    // Calcul de la position du HUD dans la VRAM en fonction du scrollX
    int hudX = 30 + g->scrollX;
    int hudY = 30;

    // créer le sprite marioText en concaténant les sprites des lettres M A R I O
    const unsigned char *marioText = createWordFromSpritesText(
        (const unsigned char *[]){
            spriteM,
            spriteA,
            spriteR,
            spriteI,
            spriteO},
        5);

    if (init)
    {
        g->ps.ecrireMot("[GameDisplay] Initializing HUD\n");

        // Affichage du mot "MARIO"
        display.plot_sprite((void *)marioText, SPRITE_TEXT_WIDTH * 5, SPRITE_TEXT_HEIGHT, hudX, hudY);
    }
}

const unsigned char *GameDisplay::createWordFromSpritesText(const unsigned char *letters[], int length)
{
    static unsigned char word[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 20];
    int index = 0;
    for (int y = 0; y < SPRITE_TEXT_HEIGHT; y++)
    {
        for (int i = 0; i < length; i++)
        {
            for (int x = 0; x < SPRITE_TEXT_WIDTH; x++)
            {
                word[index++] = letters[i][y * SPRITE_TEXT_WIDTH + x];
            }
        }
    }
    return (const unsigned char *)word;
}