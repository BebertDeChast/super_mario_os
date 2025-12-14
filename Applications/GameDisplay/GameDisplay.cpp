#include "GameDisplay.h"
#include <Applications/GameData.h>
#include <Applications/GameDisplay/Level_display_data.h>
#include <sextant/sprite.h> // for palette_vga
#include <Applications/GameDisplay/spritesText.h>
#include <Applications/MarioBros/GoombaSprite.h>
#include <sprites/TitleScreenSprite.h>

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

    display.paint_picture(level_sprite_indices, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT); // draw level background
    afficherWelcomeScreen();

    g->lock.P();
    int oldX = g->marioX;
    int oldY = g->marioY;
    int oldScrollX = g->scrollX;
    unsigned char *initSprite = g->marioSprite;
    int oldGoombaX = g->goombaX;
    int oldGoombaY = g->goombaY;
    g->lock.V();

    display.set_offset(oldScrollX, 0);

    while (g->showWelcomeScreen)
    {
        thread_yield();
    }
    removeWelcomeScreen();

    while (true)
    {
        g->lock.P();

        if (g->gameOver)
        {
            g->lock.V();
            afficherGameOver();
            continue;
        }

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
            // render HUD if needed
            afficherHUD(oldScrollX);
            oldScrollX = g->scrollX;
        }
        // Render Goomba
        if (g->goombaActive)
        {
            if (oldGoombaX != g->goombaX || oldGoombaY != g->goombaY)
            {
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
        thread_yield();
    }
}

void GameDisplay::afficherHUD(int oldscrollX)
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

    // Buffers for sprites
    static unsigned char marioText[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 5];
    static unsigned char scoreText[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 6];
    static unsigned char livesText[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 5];
    static unsigned char livesNumberText[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 2];

    // Création du sprite "MARIO"
    createWordFromSpritesText(marioText,
                              (const unsigned char *[]){
                                  spriteM,
                                  spriteA,
                                  spriteR,
                                  spriteI,
                                  spriteO},
                              5);

    // Création du sprite du score sur 6 chiffres
    createNumberFromSpritesText(scoreText, g->score, 6);

    // Création du spite "LIVES"
    createWordFromSpritesText(livesText,
                              (const unsigned char *[]){
                                  spriteL,
                                  spriteI,
                                  spriteV,
                                  spriteE,
                                  spriteS},
                              5);

    // Création du sprite du nombre de vies sur 2 chiffres
    createNumberFromSpritesText(livesNumberText, g->lives, 2);

    // Affichage du HUD

    // Affichage du mot "MARIO" et du score en dessous à gauche
    display.plot_moving_sprite((void *)marioText, SPRITE_TEXT_WIDTH * 5, SPRITE_TEXT_HEIGHT,
                               hudX, hudY,
                               oldHudX, hudY,
                               level_sprite_indices);
    display.plot_moving_sprite((void *)scoreText, SPRITE_TEXT_WIDTH * 6, SPRITE_TEXT_HEIGHT,
                               hudX, hudY + SPRITE_TEXT_HEIGHT,
                               oldHudX, hudY + SPRITE_TEXT_HEIGHT,
                               level_sprite_indices);

    // Affichage du mot "LIVES" et du nombre de vies en dessous à droite
    display.plot_moving_sprite((void *)livesText, SPRITE_TEXT_WIDTH * 5, SPRITE_TEXT_HEIGHT,
                               hudRightX - SPRITE_TEXT_WIDTH * 5, hudY,
                               oldHudRightX - SPRITE_TEXT_WIDTH * 5, hudY,
                               level_sprite_indices);
    display.plot_moving_sprite((void *)livesNumberText, SPRITE_TEXT_WIDTH * 2, SPRITE_TEXT_HEIGHT,
                               hudRightX - SPRITE_TEXT_WIDTH * 2, hudY + SPRITE_TEXT_HEIGHT,
                               oldHudRightX - SPRITE_TEXT_WIDTH * 2, hudY + SPRITE_TEXT_HEIGHT,
                               level_sprite_indices);
}

/* Creates a sprite representing a word by concatenating the corresponding letter sprites.
    The function takes an array of letter sprites and the length of the word.
    The resulting sprite is stored in the provided buffer.
*/
void GameDisplay::createWordFromSpritesText(unsigned char *buffer, const unsigned char *letters[], int length)
{
    int index = 0;
    for (int y = 0; y < SPRITE_TEXT_HEIGHT; y++)
    {
        for (int i = 0; i < length; i++)
        {
            for (int x = 0; x < SPRITE_TEXT_WIDTH; x++)
            {
                buffer[index++] = letters[i][y * SPRITE_TEXT_WIDTH + x];
            }
        }
    }
}

/* Creates a sprite representing a number with a fixed number of digits
    by concatenating the corresponding digit sprites.
    If the number has fewer digits than specified, it is padded with leading zeros.
    For example, createNumberFromSpritesText(buffer, 42, 5) will create a sprite for "00042".
    The function uses the digit sprites defined in spritesText.h.
    The resulting sprite is stored in the provided buffer.
*/
void GameDisplay::createNumberFromSpritesText(unsigned char *buffer, int number, int digits)
{
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
                buffer[index++] = digitSprites[digit][y * SPRITE_TEXT_WIDTH + x];
            }
        }
    }
}

void GameDisplay::afficherGameOver()
{
    // Implémentation de l'affichage de l'écran de Game Over
    // Ecran noir avec le texte "GAME OVER" au centre de l'écran
    display.clear(0); // Effacer l'écran avec la couleur noire (index 0)
    // Création du sprite "GAME OVER"
    static unsigned char gameOverText[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 10];
    createWordFromSpritesText(gameOverText,
                              (const unsigned char *[]){
                                  spriteG,
                                  spriteA,
                                  spriteM,
                                  spriteE,
                                  spriteMINUS,
                                  spriteO,
                                  spriteV,
                                  spriteE,
                                  spriteR,
                                  spriteWARNING},
                              10);
    // Affichage du sprite "GAME OVER" au centre de l'écran
    int centerX = (720 - SPRITE_TEXT_WIDTH * 10) / 2;
    int centerY = (240 - SPRITE_TEXT_HEIGHT) / 2;
    display.plot_sprite((void *)gameOverText, SPRITE_TEXT_WIDTH * 10, SPRITE_TEXT_HEIGHT,
                        centerX, centerY);
}

void GameDisplay::afficherWelcomeScreen()
{
    // Implémentation de l'affichage de l'écran de bienvenue
    // calcul de la position pour centrer le sprite
    int centerX = (720 - TITLE_SCREEN_WIDTH) / 2;
    int centerY = (240 - TITLE_SCREEN_HEIGHT) / 2;
    display.plot_sprite((void *)title_screen_sprite, TITLE_SCREEN_WIDTH, TITLE_SCREEN_HEIGHT,
                        centerX, centerY);
}

void GameDisplay::removeWelcomeScreen()
{
    // Clear the welcome screen by redrawing the background area
    int centerX = (720 - TITLE_SCREEN_WIDTH) / 2;
    int centerY = (240 - TITLE_SCREEN_HEIGHT) / 2;
    display.redraw_background_area(centerX, centerY, TITLE_SCREEN_WIDTH, TITLE_SCREEN_HEIGHT, level_sprite_indices);
}