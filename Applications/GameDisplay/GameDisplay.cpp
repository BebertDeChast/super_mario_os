#include "GameDisplay.h"
#include <Applications/GameData.h>
#include <Applications/GameDisplay/Level_display_data.h>
#include <sextant/sprite.h> // for palette_vga
#include <Applications/GameDisplay/spritesText.h>
#include <Applications/MarioBros/GoombaSprite.h>

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
    // g->ps.ecrireMot("[GameDisplay] Locked\n");
    int oldX = g->marioX;
    int oldY = g->marioY;
    int oldScrollX = g->scrollX;
    unsigned char *initSprite = g->marioSprite;
    int oldGoombaX = g->goombaX;
    int oldGoombaY = g->goombaY;

    g->lock.V();
    // g->ps.ecrireMot("[GameDisplay] Unlocked\n");

    display.plot_sprite(initSprite,
                        MARIO_SPRITE_WIDTH, MARIO_SPRITE_HEIGHT,
                        oldX, oldY);

    display.set_offset(oldScrollX, 0);
    afficherHUD(oldScrollX, true);

    while (true)
    {
        g->lock.P();
        // g->ps.ecrireMot("[GameDisplay] Locked in loop\n");
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
            afficherHUD(oldScrollX, false);
            oldScrollX = g->scrollX;
        }
        // Render Goomba
        if (g->goombaActive) {
            if (oldGoombaX != g->goombaX || oldGoombaY != g->goombaY) {
                display.plot_moving_sprite(goombaSpriteData,
                                           GOOMBA_WIDTH, GOOMBA_HEIGHT,
                                           g->goombaX, g->goombaY,
                                           oldGoombaX, oldGoombaY,
                                           level_sprite_indices);
                oldGoombaX = g->goombaX;
                oldGoombaY = g->goombaY;
            }
        }

        g->lock.V();
        // g->ps.ecrireMot("[GameDisplay] Unlocked in loop\n");
        thread_yield();
    }
}

void GameDisplay::afficherHUD(int oldscrollX, bool init)
{
    // Implémentation de l'affichage du HUD
    // Le HUD commence à la position (25, 15) de l'écran affiché (/= VRAM)
    // il affiche le mot "MARIO" puis le score sur 6 chiffres en dessous
    int Xmargin = 25;
    int Ymargin = 15;

    // Calcul de la position du HUD de gauche dans la VRAM en fonction du scrollX
    int hudX = Xmargin + g->scrollX;
    int hudY = Ymargin;
    int oldHudX = Xmargin + oldscrollX;

    // Calcul de la position du HUD de droite dans la VRAM en fonction du scrollX
    int hudRightX = 720 - Xmargin + g->scrollX;
    int oldHudRightX = 720 - Xmargin + oldscrollX;

    // Création du sprite "MARIO"
    const unsigned char *marioText = createWordFromSpritesText(
        (const unsigned char *[]){
            spriteM,
            spriteA,
            spriteR,
            spriteI,
            spriteO},
        5);

    // Création du sprite du score sur 6 chiffres
    const unsigned char *scoreText = createNumberFromSpritesText(g->score, 6);

    // Création du spite "LIVES"
    const unsigned char *timeText = createWordFromSpritesText(
        (const unsigned char *[]){
            spriteL,
            spriteI,
            spriteV,
            spriteE,
            spriteS},
        5);

    // Création du sprite du nombre de vies sur 2 chiffres
    const unsigned char *livesText = createNumberFromSpritesText(g->lives, 2);

    // Affichage du HUD

    if (init)
    {
        g->ps.ecrireMot("[GameDisplay] Initializing HUD\n");

        // Affichage du mot "MARIO"
        display.plot_sprite((void *)marioText, SPRITE_TEXT_WIDTH * 5, SPRITE_TEXT_HEIGHT, hudX, hudY);
        // Affichage du score en dessous de "MARIO"
        display.plot_sprite((void *)scoreText, SPRITE_TEXT_WIDTH * 6, SPRITE_TEXT_HEIGHT, hudX, hudY + SPRITE_TEXT_HEIGHT);
        // Affichage du mot "LIVES" à droite de l'écran
        display.plot_sprite((void *)timeText, SPRITE_TEXT_WIDTH * 5, SPRITE_TEXT_HEIGHT, hudRightX - SPRITE_TEXT_WIDTH * 5, hudY);
        // Affichage du nombre de vies en dessous de "LIVES"
        display.plot_sprite((void *)livesText, SPRITE_TEXT_WIDTH * 2, SPRITE_TEXT_HEIGHT, hudRightX - SPRITE_TEXT_WIDTH * 2, hudY + SPRITE_TEXT_HEIGHT);
    }
    else
    {
        // g->ps.ecrireMot("[GameDisplay] Updating HUD\n");
        display.plot_moving_sprite((void *)marioText, SPRITE_TEXT_WIDTH * 5, SPRITE_TEXT_HEIGHT,
                                   hudX, hudY,
                                   oldHudX, hudY,
                                   level_sprite_indices);
        display.plot_moving_sprite((void *)scoreText, SPRITE_TEXT_WIDTH * 6, SPRITE_TEXT_HEIGHT,
                                   hudX, hudY + SPRITE_TEXT_HEIGHT,
                                   oldHudX, hudY + SPRITE_TEXT_HEIGHT,
                                   level_sprite_indices);
        display.plot_moving_sprite((void *)timeText, SPRITE_TEXT_WIDTH * 5, SPRITE_TEXT_HEIGHT,
                                   hudRightX - SPRITE_TEXT_WIDTH * 5, hudY,
                                   oldHudRightX - SPRITE_TEXT_WIDTH * 5, hudY,
                                   level_sprite_indices);
        display.plot_moving_sprite((void *)livesText, SPRITE_TEXT_WIDTH * 2, SPRITE_TEXT_HEIGHT,
                                   hudRightX - SPRITE_TEXT_WIDTH * 2, hudY + SPRITE_TEXT_HEIGHT,
                                   oldHudRightX - SPRITE_TEXT_WIDTH * 2, hudY + SPRITE_TEXT_HEIGHT,
                                   level_sprite_indices);
    }
}

/* Creates a sprite representing a word by concatenating the corresponding letter sprites.
    The function takes an array of letter sprites and the length of the word.
    The resulting sprite is stored in a static array and returned.
*/
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

/* Creates a sprite representing a number with a fixed number of digits
    by concatenating the corresponding digit sprites.
    If the number has fewer digits than specified, it is padded with leading zeros.
    For example, createNumberFromSpritesText(42, 5) will create a sprite for "00042".
    The function uses the digit sprites defined in spritesText.h.
    The resulting sprite is stored in a static array and returned.
*/
const unsigned char *GameDisplay::createNumberFromSpritesText(int number, int digits)
{
    static unsigned char numberSprite[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 20]; // Max 20 digits
    const unsigned char *digitSprites[10] = {
        sprite0, sprite1, sprite2, sprite3, sprite4,
        sprite5, sprite6, sprite7, sprite8, sprite9};

    // Convert number to string with leading zeros
    char numStr[20];
    for (int i = digits - 1; i >= 0; i--)
    {
        numStr[i] = (number % 10) + '0';
        number /= 10;
    }

    int index = 0;
    for (int y = 0; y < SPRITE_TEXT_HEIGHT; y++)
    {
        for (int i = 0; i < digits; i++)
        {
            int digit = numStr[i] - '0'; // Convert char to int
            for (int x = 0; x < SPRITE_TEXT_WIDTH; x++)
            {
                numberSprite[index++] = digitSprites[digit][y * SPRITE_TEXT_WIDTH + x];
            }
        }
    }
    return (const unsigned char *)numberSprite;
}