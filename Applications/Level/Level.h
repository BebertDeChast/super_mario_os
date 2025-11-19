#define LEVEL_H

#include <drivers/EcranBochs.h>
#include <drivers/Clavier.h>
#include <sextant/types.h>
#include <drivers/timer.h>

/**
 * @file LevelDisplay.h
 * @class LevelDisplay
 * @brief Affiche le niveau actuel du jeu.
 */
class Level
{
    EcranBochs *e;

public:
    /**
     * @brief Constructeur de Level.
     * @param e écran pour l'affichage
     */
    Level(EcranBochs *e);
    void afficheNiveau();
};
