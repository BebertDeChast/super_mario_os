#include "Level.h"

Level::Level(EcranBochs *ecr)
{
    e = ecr;
}

void Level::afficheNiveau()
{
    // affiche un carré au milieu de l'écran
    int x = (e->getWidth() - 100) / 2;
    int y = (e->getHeight() - 100) / 2;
    e->plot_square(x, y, e->getWidth() / 10, 10); 
}