/*
Projet-ISN

Fichier: IOoptions.c

Contenu: Fonctions d'entrées sorties (chargements, sauvegardes de fichiers, ...)

Actions: C'est ici que se trouve les fonctions qui lisent ou écrivent sur le disque et qui concernent les options.

Bibliothèques utilisées: Bibliothèques standards, SDL, SDL_image, SDL_ttf, FMOD, GTK

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <fmod.h>
#include <gtk/gtk.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"

int LectureOptions(char options[][50])
{
	FILE *pFichierOptions = NULL;       //Pointeur sur le fichier d'options
	int i=0, j=1;
	char caractere = 0;     //Variable pour retenir le caractère actuel

	pFichierOptions = fopen("ressources/settings.ini", "r");       //Ouverture du fichier en mode lecture

	if(pFichierOptions == NULL) //On vérifie si le fichier existe
	{
		return -1;
	}

	do
	{
		do
		{
			caractere = fgetc(pFichierOptions); //On lit un caractère
			options[j][i] = caractere;  //On le met dans le tableau
			i++;    //Caractère suivant dans le tableau
		}while (caractere != '\n' && caractere != EOF);  //Tant qu'il n'y a pas de retour à la ligne ou de fin de fichier

		options[j][i-1] = '\0';     //On place le caractère de fin de chaîne à la place du retour à la ligne
		j++;      //Ligne suivante dans le tableau
		i=0;	//On revient au début de la ligne dans le tableau
	}while (caractere != EOF);       //Tant qu'on est pas à la fin du fichier

	fclose(pFichierOptions);        //On ferme le fichier

	options[0][0]= (char)j-1;	//Sur le premier octet on met le nombre de lignes total

	return 0;
}

int ValiderChangement(char options[][50])
{
	int i=0;	//Compteur
	FILE *pFichierOptions = NULL;	//Pointeur sur le fichier des options

	pFichierOptions = fopen("ressources/settings.ini", "w");      //On ouvre le fichier et on l'efface

	if(pFichierOptions == NULL)     //On vérifie l'ouverture
	{
		return -1;
	}

	for(i=1; i<options[0][0] ; i++)
	{
		fputs(options[i], pFichierOptions);   //On écrit chaque ligne
		fputc('\n', pFichierOptions);       //On place un retour à la ligne
	}

	fclose(pFichierOptions);        //On ferme le fichier

	return 0;
}

Options* DecouperOptions(char options[][50])
{
	/* Cette fonction alloue une structure Options qu'elle remplit avec le tableau lu par la fonction précédente */

	Options *pOptions = malloc(sizeof(Options));	//On alloue une structure Options
	char *c=NULL;	//Pointeur sur un caractère pour faire des recherches dans les chaînes

	if(pOptions == NULL)	//On vérifie l'allocation de la structure
	{
		return NULL;
	}

	c = strstr(options[1], "=");	//On cherche le '=', on place son adresse dans le pointeur
	pOptions->musique = strtol((c+1), NULL, 10);	//On convertit en nombre décimal le contenu de la chaîne à partir du caractère suivant le '=' d'où (c+1)

	c = strstr(options[2], "=");
	pOptions->sons = strtol((c+1), NULL, 10);

	c = strstr(options[3], "=");
	pOptions->largeur = strtol((c+1), NULL, 10);

	c = strstr(options[3], "x");	//Ici c'est le 'x' entre les deux valeurs de la résolution que l'on recherche
	pOptions->hauteur = strtol((c+1), NULL, 10);

	c = strstr(options[4], "=");
	pOptions->vies = strtol((c+1), NULL, 10);

	c = strstr(options[5], "=");
	pOptions->volume = strtod((c+1), NULL);

	c = strstr(options[6], "=");
	pOptions->fullScreen = strtol((c+1), NULL, 10);

	pOptions->nbLigne = options[0][0];	//On stocke le nombre de ligne dans la structure également

	return pOptions;	//On retourne l'adresse de la structure qu'on a allouée
}

//Fin du fichier IOoptions.c