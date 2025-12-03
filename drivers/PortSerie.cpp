/*
 * PortSerie.cpp
 *
 *  Created on: 4 ao�t 2008
 *      Author: jmenaud
 */

#include "PortSerie.h"
#include "Ecran.h"

void PortSerie::ecrireMot(const char *mot)
{
    int i = 0;
    while (mot[i] != '\0')
    {
        ecrireOctet(mot[i], 0x3F8);
        i++;
    }
}

void PortSerie::afficherCaractere(char c)
{
    ecrireOctet(c, 0x3F8);
}

void PortSerie::afficherBase(unsigned int entier, int base)
{
    char buff[64], *cur = buff;

    if (base <= 2 || base >= 30)
    {
        return;
    }

    if (entier == 0)
    {
        ecrireMot("0");
    }
    else
    {
        while (entier >= base)
        {
            *cur = int2char(entier % base, base);
            entier /= base;

            cur++;
        }
        *cur = int2char(entier, base);

        while (cur >= buff)
        {
            afficherCaractere(*cur);
            cur--;
        }
    }
}