#ifndef GAMEDISPLAY_H
#define GAMEDISPLAY_H

#include <drivers/EcranBochs.h>
#include <drivers/Clavier.h>
#include <sextant/types.h>
#include <drivers/timer.h>
#include <sextant/Activite/Threads.h>
#include <Applications/GameData.h>

/**
 * @file LevelDisplay.h
 * @class LevelDisplay
 * @brief Affiche le niveau actuel du jeu.
 */
class GameDisplay : public Threads
{
    GameData *g;
    EcranBochs display;

    void createWordFromSpritesText(unsigned char *buffer, const unsigned char *letters[], int length);
    void createNumberFromSpritesText(unsigned char *buffer, int number, int digits);
    void displayHUD(int oldscrollX);
    void displayGameOver();
    void displayWelcomeScreen();
    void removeWelcomeScreen();
    void displayCredits(int x, int y);

public:
    /**
     * @brief Constructeur de Level.
     * @param e écran pour l'affichage
     */
    GameDisplay(GameData *data);
    void run() override;
    
};

#endif
