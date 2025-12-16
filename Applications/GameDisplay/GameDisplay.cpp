#include "GameDisplay.h"
#include <Applications/GameData.h>
#include <sprites/Level_display_data.h>
#include <sprites/palette.h> // for palette_vga
#include <sprites/MarioSprites.h>
#include <sprites/spritesText.h>
#include <sprites/GoombaSprite.h>
#include <sprites/TitleScreenSprite.h>

/**
 * @brief Construct a new Game Display:: Game Display object
 *
 * @param data a pointer to the shared GameData object
 */
GameDisplay::GameDisplay(GameData *data) : display(720, 240, LEVEL_WIDTH, VBE_MODE::_8)
{
    g = data;
}

/**
 * @brief Main loop for the game display thread.
 * This function initializes the display, shows the welcome screen,
 * and then enters a loop to render the game world until the game is over.
 */
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
    displayHUD(oldScrollX);

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
            if (oldScrollX != g->scrollX)
                displayHUD(oldScrollX);
            oldScrollX = g->scrollX;
        }
        // Render Goombas
        for (int i = 0; i < MAX_GOOMBAS; i++)
        {
            if (g->goombas[i].active)
            {
                if (oldGoombaX[i] != g->goombas[i].x || oldGoombaY[i] != g->goombas[i].y || oldGoombaSprite[i] != g->goombas[i].sprite)
                {
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
            }
            else if (wasGoombaActive[i])
            {
                // Erase if it just became inactive
                display.redraw_background_area(oldGoombaX[i], oldGoombaY[i], GOOMBA_WIDTH, GOOMBA_HEIGHT, level_sprite_indices);
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

/**
 * @brief Displays the Heads-Up Display (HUD) on the screen.
 * The HUD shows "MARIO" with the score below it on the left,
 * and "LIVES" with the number of lives below it on the right.
 * The HUD is positioned relative to the screen, not the VRAM, and moves with the screen scroll.
 * @param oldscrollX The previous scroll position, used to erase the old HUD.
 */
void GameDisplay::displayHUD(int oldscrollX)
{
    // Implementation of the HUD display
    // The HUD starts at position (25, 15) of the displayed screen (not VRAM)
    // It displays the word "MARIO" then the 6-digit score below it
    int Xmargin = 25;
    int Ymargin = 15;

    // Calculate the position of the left HUD in VRAM based on scrollX
    int hudX = Xmargin + g->scrollX;
    int hudY = Ymargin;
    int oldHudX = Xmargin + oldscrollX;

    // Calculate the position of the right HUD in VRAM based on scrollX
    int hudRightX = 720 - Xmargin + g->scrollX;
    int oldHudRightX = 720 - Xmargin + oldscrollX;

    // Buffers for sprites
    static unsigned char marioText[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 5];
    static unsigned char scoreText[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 6];
    static unsigned char livesText[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 5];
    static unsigned char livesNumberText[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 2];

    // Create the "MARIO" sprite
    createWordFromSpritesText(marioText,
                              (const unsigned char *[]){
                                  spriteM,
                                  spriteA,
                                  spriteR,
                                  spriteI,
                                  spriteO},
                              5);

    // Create the 6-digit score sprite
    createNumberFromSpritesText(scoreText, g->score, 6);

    // Create the "LIVES" sprite
    createWordFromSpritesText(livesText,
                              (const unsigned char *[]){
                                  spriteL,
                                  spriteI,
                                  spriteV,
                                  spriteE,
                                  spriteS},
                              5);

    // Create the 2-digit lives number sprite
    createNumberFromSpritesText(livesNumberText, g->lives, 2);

    // Display the HUD

    // Display "MARIO" and the score below it on the left
    display.plot_moving_sprite((void *)marioText, SPRITE_TEXT_WIDTH * 5, SPRITE_TEXT_HEIGHT,
                               hudX, hudY,
                               oldHudX, hudY,
                               level_sprite_indices);
    display.plot_moving_sprite((void *)scoreText, SPRITE_TEXT_WIDTH * 6, SPRITE_TEXT_HEIGHT,
                               hudX, hudY + SPRITE_TEXT_HEIGHT,
                               oldHudX, hudY + SPRITE_TEXT_HEIGHT,
                               level_sprite_indices);

    // Display "LIVES" and the number of lives below it on the right
    display.plot_moving_sprite((void *)livesText, SPRITE_TEXT_WIDTH * 5, SPRITE_TEXT_HEIGHT,
                               hudRightX - SPRITE_TEXT_WIDTH * 5, hudY,
                               oldHudRightX - SPRITE_TEXT_WIDTH * 5, hudY,
                               level_sprite_indices);
    display.plot_moving_sprite((void *)livesNumberText, SPRITE_TEXT_WIDTH * 2, SPRITE_TEXT_HEIGHT,
                               hudRightX - SPRITE_TEXT_WIDTH * 2, hudY + SPRITE_TEXT_HEIGHT,
                               oldHudRightX - SPRITE_TEXT_WIDTH * 2, hudY + SPRITE_TEXT_HEIGHT,
                               level_sprite_indices);
}

/**
 * @brief Creates a sprite representing a word by concatenating the corresponding letter sprites.
 * The function takes an array of letter sprites and the length of the word.
 * The resulting sprite is stored in the provided buffer.
 * @param buffer The buffer to store the resulting word sprite.
 * @param letters An array of pointers to the individual letter sprites.
 * @param length The number of letters in the word.
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

/**
 * @brief Creates a sprite representing a number with a fixed number of digits
 * by concatenating the corresponding digit sprites.
 * If the number has fewer digits than specified, it is padded with leading zeros.
 * For example, createNumberFromSpritesText(buffer, 42, 5) will create a sprite for "00042".
 * The function uses the digit sprites defined in spritesText.h.
 * The resulting sprite is stored in the provided buffer.
 * @param buffer The buffer to store the resulting number sprite.
 * @param number The number to convert into a sprite.
 * @param digits The fixed number of digits for the sprite (with leading zeros if necessary).
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

/**
 * @brief Displays the "GAME OVER" screen.
 * This function clears the screen to black, then displays the text "GAME OVER"
 * in the center, followed by the final score and credits.
 */
void GameDisplay::displayGameOver()
{
    // Implementation of the Game Over screen display
    // Black screen with "GAME OVER" text in the center
    display.clear(0); // Clear the screen with black color (index 0)
    display.set_offset(0, 0);
    // Create the "GAME OVER" sprite
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
    // Display the "GAME OVER" sprite in the center of the screen
    int centerX = (720 - SPRITE_TEXT_WIDTH * 10) / 2;
    int centerY = (240 - SPRITE_TEXT_HEIGHT) / 2;
    display.plot_sprite((void *)gameOverText, SPRITE_TEXT_WIDTH * 10, SPRITE_TEXT_HEIGHT,
                        centerX, centerY);
    // Score below
    static unsigned char scoreText[SPRITE_TEXT_WIDTH * SPRITE_TEXT_HEIGHT * 6];
    createNumberFromSpritesText(scoreText, g->score, 6);
    display.plot_sprite((void *)scoreText, SPRITE_TEXT_WIDTH * 6, SPRITE_TEXT_HEIGHT,
                        centerX, centerY + SPRITE_TEXT_HEIGHT + 5);

    // Credits in the top left
    displayCredits(10, 10);
}

/**
 * @brief Displays the welcome screen.
 * This includes the game title sprite centered, a "Press any key to start"
 * message, and the game credits.
 */
void GameDisplay::displayWelcomeScreen()
{
    // Implementation of the welcome screen display
    // Calculate position to center the sprite
    int centerX = (720 - TITLE_SCREEN_WIDTH) / 2;
    int centerY = (240 - TITLE_SCREEN_HEIGHT) / 2;
    display.plot_sprite((void *)title_screen_sprite, TITLE_SCREEN_WIDTH, TITLE_SCREEN_HEIGHT,
                        centerX, centerY);

    // Display a "Press any key to start" message at the top of the screen
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

    // Display credits to the right of the title
    displayCredits(centerX + TITLE_SCREEN_WIDTH + 10, centerY);
}

/**
 * @brief Removes the welcome screen elements to prepare for the game start.
 * It redraws the background over the title, the "press key" text, and the credits.
 */
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

/**
 * @brief Displays the game credits at a specified location.
 * @param x The x-coordinate to start drawing the credits.
 * @param y The y-coordinate to start drawing the credits.
 */
void GameDisplay::displayCredits(int x, int y)
{
    // Credits:
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