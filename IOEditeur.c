/*
Projet-ISN

Fichier: IOEditeur.c

Contenu: Fonctions relatives aux chargements et aux écritures de l'éditeur de niveau.

Actions: C'est ici que se trouve les fonctions qui gèrent l'éditeur de niveau.

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
#include <gtk/gtk.h>
#include <fmod.h>
#include <SDL2_gfxPrimitives.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "jeu.h"
#include "IOediteur.h"
#include "editeur.h"

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;
double Volume, Largeur, Hauteur;

int SauvegardeNiveau(Map* pMap, sprite images[])
{
	FILE *pFichierNiveau = NULL;
	char tile[5] = "2\n";
	char chaine[20] = "";
	char *c = NULL;
	int i=0, j=0;

	pFichierNiveau = fopen("ressources/level.lvl", "a");

	if (pFichierNiveau == NULL)
	{
		return -1;
	}

	fputs("#taille\n", pFichierNiveau);

	sprintf(chaine, "%d\n", pMap->nbtiles_hauteur_monde);
	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%d\n", pMap->nbtiles_largeur_monde);
	fputs(chaine, pFichierNiveau);

	fputs("#missileV\n", pFichierNiveau);

	for(i=0; i<5; i++)
	{
		if(images[MISSILE].position[i].x < 0)
		{
			fputs("-1.0\n", pFichierNiveau);
			fputs("-1.0\n", pFichierNiveau);
			continue;
		}

		sprintf(chaine, "%f\n", (float)images[MISSILE].position[i].x/(float)Largeur);
		c = strstr(chaine, ",");

		if(c != NULL)
		{
			*c = '.';
		}

		fputs(chaine, pFichierNiveau);

		sprintf(chaine, "%f\n", (float)images[MISSILE].position[i].y/(float)Hauteur);
		c = strstr(chaine, ",");

		if(c != NULL)
		{
			*c = '.';
		}

		fputs(chaine, pFichierNiveau);
	}


	fputs("#missileH\n", pFichierNiveau);

	for(i=5; i<10; i++)
	{
		if(images[MISSILE].position[i].x < 0)
		{
			fputs("-1.0\n", pFichierNiveau);
			fputs("-1.0\n", pFichierNiveau);
			continue;
		}

		sprintf(chaine, "%f\n", (float)images[MISSILE].position[i].x/(float)Largeur);
		c = strstr(chaine, ",");

		if(c != NULL)
		{
			*c = '.';
		}

		fputs(chaine, pFichierNiveau);

		sprintf(chaine, "%f\n", (float)images[MISSILE].position[i].y/(float)Hauteur);
		c = strstr(chaine, ",");

		if(c != NULL)
		{
			*c = '.';
		}

		fputs(chaine, pFichierNiveau);
	}

	fputs("#vortex\n", pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[VORTEX_BLEU].position[0].x/(float)Largeur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[VORTEX_BLEU].position[0].y/(float)Hauteur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[VORTEX_VERT].position[0].x/(float)Largeur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[VORTEX_VERT].position[0].y/(float)Hauteur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	fputs("#boules\n", pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[BOULE_BLEUE].position[0].x/(float)Largeur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[BOULE_BLEUE].position[0].y/(float)Hauteur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[BOULE_MAGENTA].position[0].x/(float)Largeur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[BOULE_MAGENTA].position[0].y/(float)Hauteur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[BOULE_VERTE].position[0].x/(float)Largeur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	sprintf(chaine, "%f\n", (float)images[BOULE_VERTE].position[0].y/(float)Hauteur);
	c = strstr(chaine, ",");

	if(c != NULL)
	{
		*c = '.';
	}

	fputs(chaine, pFichierNiveau);

	fputs("#map\n", pFichierNiveau);

	for (i=0; i< pMap->nbtiles_largeur_monde; i++)
	{
		for(j=0; j< pMap->nbtiles_hauteur_monde; j++)
		{
			sprintf(tile, "%d\n", pMap->plan[i][j]);
			fputs(tile, pFichierNiveau);
		}
	}

	fputs("#mapObjets\n", pFichierNiveau);

	for (i=0; i< pMap->nbtiles_largeur_monde; i++)
	{
		for(j=0; j< pMap->nbtiles_hauteur_monde; j++)
		{
			sprintf(tile, "%d\n", pMap->planObjets[i][j]);
			fputs(tile, pFichierNiveau);
		}
	}

	fputs("##--##\n", pFichierNiveau);

	fclose(pFichierNiveau);

	return 0;
}

//Fin du fichier IOEditeur.c
