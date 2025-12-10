#define GAMEDISPLAY_H

#include <drivers/EcranBochs.h>
#include <drivers/Clavier.h>
#include <sextant/types.h>
#include <drivers/timer.h>

/**
 * @file LevelDisplay.h
 * @class LevelDisplay
 * @brief Affiche le niveau actuel du jeu.
 */
class GameDisplay
{
    GameData *g;

public:
    /**
     * @brief Constructeur de Level.
     * @param e écran pour l'affichage
     */
    GameDisplay(GameData *data);
    void run();
};
