#ifndef SEXTANT_RESSOURCES_VIA_MEM_TP3_ECRAN_MODE_TEXTE_H
#define SEXTANT_RESSOURCES_VIA_MEM_TP3_ECRAN_MODE_TEXTE_H

/**
 * @file Ecran.h
 * @class Ecran
 * @brief Affichage � l'�cran en couleur. Le caract�re '\\n' est pris en charge. Si l'�cran est plein, un d�filement
 * vers le bas est provoqu� lors de la cr�tion d'une nouvelle ligne.
 */

#define CRT_REG_INDEX 0x3d4
#define CRT_REG_DATA  0x3d5
#define VIDEO   	0xb8000
#define LIGNES 		25 // 25 lignes maximun pour un �cran
#define COLONNES	80 // 80 colonnes maximun pour un �cran
#define CURSEUR		'>' // caract�re pour le curseur
#define CLIGN_OUI		1 // Clignotement
#define CLIGN_NON		0 // Clignotement

#include <sextant/types.h>
#include <hal/fonctionsES.h>

/**
 * @struct caseEcran
 * @brief Une case m�moire de la m�moire vidéo en mode texte.
 */
struct caseEcran {
	unsigned char caractere;
	unsigned char couleurs;
} __attribute__ ((packed));
//__attribute__ ((packed)) : compile la structure en m�moire telle qu'elle est d�finie.


/**
 * Affiche le curseur à la position ('ligne','colonne').
 */
class Ecran {
	/**
	 * Couleur de l'arri�re-plan.
	 */
	Couleur arrierePlan;
	/**
	 * Ligne courante.
	 */
	int ligne;
	/**
	 * Colonne courante.
	 */
	int colonne;
	/**
	 * Adresse de la premi�re case de la m�moire vid�o.
	 *
	 * volatile : signifie que l'�criture et la lecture se font toujours
	 * en m�moire (certaines optimisations du compilateur permettent
	 * d'�viter la lecture ou l'�criture en m�moire en pla�ant
	 * des variables dans des registres).
	 */
	static volatile caseEcran* origine;

	int getLigne();

	int getColonne();


	void setLigne(int);

	void setColonne(int);

	int positionCourrante();

	void avancerPositionCourrante();

	/***
	 * Modifie la position courante de la sortie sur l'�cran
	 ***/
	void modifierPosition(int,int);
	/*
	 * Fonction de traduction : couleur -> code
	 */
	unsigned char coderCouleur(Couleur);

	/*
	 * Fonction de traduction : clignotement + couleurs -> attribut
	 */
	unsigned char coderAttribut(Couleur, Couleur);

	unsigned char coderAttribut(Couleur , Couleur ,int);

	/**
	 * Affiche le curseur � la position ('ligne','colonne').
	 */
	void afficherCurseur();
	/**
	 * G�re le d�filement.
	 * Lorsque le curseur sort de l'�cran, le texte est d�cal� d'une ligne vers le haut,
	 * lib�rant la derni�re ligne.
	 * Le curseur se positionne en bas � gauche.
	 */
	void defilement(int nline);
public :

	void sautDeLigne();


	/**
	 * Efface l'�cran en le coloriant avec la couleur indiqu�e.
	 */
	void effacerEcran(Couleur arrierePlan);

	/**
	 * Ecrit un caract�re sur l'�cran (caract�re) � la position donn�e (l,c)
	 * avec la couleur (prPlan) et l'attribut donn� (arPlan).
	 */
	void afficherCaractereSeul(int l,int c,Couleur prPlan,Couleur arPlan,const char caractere);

	void afficherCaractere(Couleur prPlan,Couleur arPlan,const char caractere);

	/**
	 * Ecrit sur l'�cran le caract�re � la position donn�e  avec l'attribut donn�.
	 * Ne g�re ni le caract�re de passage � la ligne ('\n')
	 * ni le d�filement.
	 * Corrige les erreurs de position.
	 */
	void afficherCaractere(int ligne, int colonne,Couleur premierPlan, Couleur arrierePlan,const char caractere);

	/**
	 * Affiche la cha�ne de  caract�res sur l'�cran � la position courante
	 * avec la couleur donn�e.
	 */
	void afficherMot(const char *mot,Couleur premierPlan=BLANC);
	void afficherMot(int l,int c,const char *mot,Couleur premierPlan=BLANC);

	/**
	 * Affiche un entier � la position courante de couleur prPlan (blanc par defaut)
	 * Dans la base choisie (maximum 16). Par d�faut, l'entier est affich� dans la base d�cimale en blanc.
	 */
	void afficherBase(unsigned int entier,int base=10,Couleur prPlan=BLANC);

	void afficherChiffre( const int valeur);
	void afficherChiffre(int l,int c, const int valeur);

};

char int2char(unsigned int n, int base);
#endif
