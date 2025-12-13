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

    const unsigned char *createWordFromSpritesText(const unsigned char *letters[], int length);

public:
    /**
     * @brief Constructeur de Level.
     * @param e écran pour l'affichage
     */
    GameDisplay(GameData *data);
    void run() override;
    void afficherHUD(int oldscrollX, bool init = false);
};

#endif
