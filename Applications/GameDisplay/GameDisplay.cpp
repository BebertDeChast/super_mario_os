#include "GameDisplay.h"
#include <Applications/GameData.h>
#include <sprites/Level_display_data.h>
#include <sprites/palette.h> // for palette_vga
#include <sprites/MarioSprites.h>
#include <sprites/spritesText.h>
#include <sprites/GoombaSprite.h>
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
    displayWelcomeScreen();

    g->lock.P();
    int oldX = g->marioX;
    int oldY = g->marioY;
    int oldScrollX = g->scrollX;
    unsigned char *initSprite = g->marioSprite;

    int oldGoombaX[MAX_GOOMBAS];
    int oldGoombaY[MAX_GOOMBAS];
    unsigned char *oldGoombaSprite[MAX_GOOMBAS];
    bool wasGoombaActive[MAX_GOOMBAS];

    // Initialize local tracking arrays
    for (int i = 0; i < MAX_GOOMBAS; i++)
    {
        oldGoombaX[i] = -100;
        oldGoombaY[i] = -100;
        oldGoombaSprite[i] = g->goombas[i].sprite;
        wasGoombaActive[i] = false;
    }
    g->lock.V();

    display.set_offset(oldScrollX, 0);

    while (g->showWelcomeScreen)
    {
        thread_yield();
    }
    removeWelcomeScreen();

    while (!g->gameOver)
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
            // render HUD if needed
            displayHUD(oldScrollX);
            oldScrollX = g->scrollX;
        }
        // Render Goombas
        for(int i=0; i<MAX_GOOMBAS; i++) {
            if (g->goombas[i].active) {
                if (oldGoombaX[i] != g->goombas[i].x || oldGoombaY[i] != g->goombas[i].y || oldGoombaSprite[i] != g->goombas[i].sprite) {
                    display.plot_moving_sprite(g->goombas[i].sprite,
                                               GOOMBA_WIDTH, GOOMBA_HEIGHT,
                                               g->goombas[i].x, g->goombas[i].y,
                                               oldGoombaX[i], oldGoombaY[i],
                                               level_sprite_indices);
                    oldGoombaX[i] = g->goombas[i].x;
                    oldGoombaY[i] = g->goombas[i].y;
                    oldGoombaSprite[i] = g->goombas[i].sprite;
                }
                wasGoombaActive[i] = true;
            } else if (wasGoombaActive[i]) {
                // Erase if it just became inactive
                display.plot_moving_sprite(oldGoombaSprite[i],
                                           GOOMBA_WIDTH, GOOMBA_HEIGHT,
                                           -100, -100,
                                           oldGoombaX[i], oldGoombaY[i],
                                           level_sprite_indices);
                oldGoombaX[i] = -100;
                oldGoombaY[i] = -100;
                wasGoombaActive[i] = false;
            }
        }

        g->lock.V();
        thread_yield();
    }

    if (g->gameOver)
    {
        displayGameOver();
        g->ps.ecrireMot("[GameDisplay] Game Over detected, exiting display thread.\n");
        thread_exit();
    }
}

void GameDisplay::displayHUD(int oldscrollX)
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

void GameDisplay::displayGameOver()
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
                                  spriteSPACE,
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
    // Score en dessous
    static unsigned char scoreText[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 6];
    createNumberFromSpritesText(scoreText, g->score, 6);
    display.plot_sprite((void *)scoreText, SPRITE_TEXT_WIDTH * 6, SPRITE_TEXT_HEIGHT,
                        centerX, centerY + SPRITE_TEXT_HEIGHT + 5);

    // Crédit en haut à gauche
    displayCredits(10, 10);
}

void GameDisplay::displayWelcomeScreen()
{
    // Implémentation de l'affichage de l'écran de bienvenue
    // calcul de la position pour centrer le sprite
    int centerX = (720 - TITLE_SCREEN_WIDTH) / 2;
    int centerY = (240 - TITLE_SCREEN_HEIGHT) / 2;
    display.plot_sprite((void *)title_screen_sprite, TITLE_SCREEN_WIDTH, TITLE_SCREEN_HEIGHT,
                        centerX, centerY);

    // Affichage d'un message "Press any key to start" en haut de l'écran
    static unsigned char pressKeyText[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 22];
    createWordFromSpritesText(pressKeyText,
                              (const unsigned char *[]){
                                  spriteP,
                                  spriteR,
                                  spriteE,
                                  spriteS,
                                  spriteS,
                                  spriteSPACE,
                                  spriteA,
                                  spriteN,
                                  spriteY,
                                  spriteSPACE,
                                  spriteK,
                                  spriteE,
                                  spriteY,
                                  spriteSPACE,
                                  spriteT,
                                  spriteO,
                                  spriteSPACE,
                                  spriteS,
                                  spriteT,
                                  spriteA,
                                  spriteR,
                                  spriteT},
                              22);
    int textX = (720 - SPRITE_TEXT_WIDTH * 22) / 2;
    int textY = 10;
    display.plot_sprite((void *)pressKeyText, SPRITE_TEXT_WIDTH * 22, SPRITE_TEXT_HEIGHT,
                        textX, textY);

    // Affichage des crédits à droite du titre
    displayCredits(centerX + TITLE_SCREEN_WIDTH + 10, centerY);
}

void GameDisplay::removeWelcomeScreen()
{
    // Clear the welcome screen by redrawing the background area
    int centerX = (720 - TITLE_SCREEN_WIDTH) / 2;
    int centerY = (240 - TITLE_SCREEN_HEIGHT) / 2;
    display.redraw_background_area(centerX, centerY, TITLE_SCREEN_WIDTH, TITLE_SCREEN_HEIGHT, level_sprite_indices);
    // Clear the "Press any key to start" text
    int textX = (720 - SPRITE_TEXT_WIDTH * 22) / 2;
    int textY = 10;
    display.redraw_background_area(textX, textY, SPRITE_TEXT_WIDTH * 22, SPRITE_TEXT_HEIGHT, level_sprite_indices);
    // Clear the credits
    display.redraw_background_area(centerX + TITLE_SCREEN_WIDTH + 10, centerY,
                                   SPRITE_TEXT_WIDTH * 21, SPRITE_TEXT_HEIGHT * 4,
                                   level_sprite_indices);
}

void GameDisplay::displayCredits(int x, int y)
{
    // Humbert de Chastellux
    // Valentin Chaud
    // Jordan Baumard
    static unsigned char creditLineHumbert[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 21];
    createWordFromSpritesText(creditLineHumbert,
                              (const unsigned char *[]){
                                  spriteH,
                                  spriteU,
                                  spriteM,
                                  spriteB,
                                  spriteE,
                                  spriteR,
                                  spriteT,
                                  spriteSPACE,
                                  spriteD,
                                  spriteE,
                                  spriteSPACE,
                                  spriteC,
                                  spriteH,
                                  spriteA,
                                  spriteS,
                                  spriteT,
                                  spriteE,
                                  spriteL,
                                  spriteL,
                                  spriteU,
                                  spriteX},
                              21);

    static unsigned char creditLineValentin[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 14];
    createWordFromSpritesText(creditLineValentin,
                              (const unsigned char *[]){
                                  spriteV,
                                  spriteA,
                                  spriteL,
                                  spriteE,
                                  spriteN,
                                  spriteT,
                                  spriteI,
                                  spriteN,
                                  spriteSPACE,
                                  spriteC,
                                  spriteH,
                                  spriteA,
                                  spriteU,
                                  spriteD},
                              14);
    static unsigned char creditLineJordan[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 14];
    createWordFromSpritesText(creditLineJordan,
                              (const unsigned char *[]){
                                  spriteJ,
                                  spriteO,
                                  spriteR,
                                  spriteD,
                                  spriteA,
                                  spriteN,
                                  spriteSPACE,
                                  spriteB,
                                  spriteA,
                                  spriteU,
                                  spriteM,
                                  spriteA,
                                  spriteR,
                                  spriteD},
                              14);

    display.plot_sprite((void *)creditLineValentin, SPRITE_TEXT_WIDTH * 14, SPRITE_TEXT_HEIGHT,
                        x, y);
    display.plot_sprite((void *)creditLineHumbert, SPRITE_TEXT_WIDTH * 21, SPRITE_TEXT_HEIGHT,
                        x, y + SPRITE_TEXT_HEIGHT + 2);

    display.plot_sprite((void *)creditLineJordan, SPRITE_TEXT_WIDTH * 14, SPRITE_TEXT_HEIGHT,
                        x, y + 2 * (SPRITE_TEXT_HEIGHT + 2));
}