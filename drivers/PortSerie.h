/*
 * PortSerie.h
 *
 *  Created on: 4 ao�t 2008
 *      Author: jmenaud
 */

#ifndef PORTSERIE_H_
#define PORTSERIE_H_

#include <hal/fonctionsES.h>

class PortSerie {
public :


	void ecrireMot(const char*);
	void afficherBase(unsigned int entier, int base);
	void afficherCaractere(char c);


};
#endif /* PORTSERIE_H_ */
