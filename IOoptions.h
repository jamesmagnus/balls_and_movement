/*
Projet-ISN

Fichier: IOoptions.h

Contenu: Prototypes des fonctions contenues dans IOoptions.c

Actions: Permet à l'ordinateur de connaître toutes les fonctions présentes dans le programme ainsi que leurs arguments.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef IOOPTIONS_H_INCLUDED		//Protection contre les inclusons infinies
#define IOOPTIONS_H_INCLUDED

/* Structures */
typedef struct Options		//Structure pour stocker les options
{
	int fullScreen;
	int musique;
	int nbLigne;
	int sons;
	int largeur;
	int hauteur;
	char vies;
	float volume;
} Options;

/* Prototypes des fonctions */
int LectureOptions(char options[][50]);
Options* DecouperOptions(char options[][50]);
int ValiderChangement(char options[][50]);

#endif // IOOPTIONS_H_INCLUDED

//Fin du fichier IOoptions.h
