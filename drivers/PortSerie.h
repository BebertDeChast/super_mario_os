/*
 * PortSerie.h
 *
 *  Created on: 4 ao�t 2008
 *      Author: jmenaud
 */

#ifndef PORTSERIE_H_
#define PORTSERIE_H_

#include <hal/fonctionsES.h>

struct GameData;

class PortSerie
{
public:
	void ecrireMot(const char *);
	void afficherBase(unsigned int entier, int base);
	void afficherCaractere(char c);
	void afficherGameData(GameData *data);
};
#endif /* PORTSERIE_H_ */
