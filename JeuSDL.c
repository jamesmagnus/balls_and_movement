/*
Projet-ISN

Fichier: jeuSDL.c

Contenu: Fonctions d'initialisation du jeu.

Actions: C'est ici que sont effectuées les tâches de chargement du niveau, d'ouverture de la fenêtre, ...

Bibliothèques utilisées: Bibliothèques standards, SDL, SDL_image, SDL_ttf, FMOD, GTK

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock.h>
#include <mysql.h>
#include <SDL.h>
#include <math.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <fmod.h>
#include <gtk\gtk.h>
#include <SDL2_gfxPrimitives.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "editeur.h"
#include "jeu.h"
#include "JeuSDL.h"

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;
extern double Volume, Hauteur, Largeur;
extern InfoDeJeu infos;

int LancerJeu(FMOD_SYSTEM *pMoteurSon, Sons *pSons, const char mode[], Joueur *pJoueur)
{
	SDL_Renderer *pMoteurRendu = NULL;  //Pointeurs sur le moteur de rendu
	SDL_Window *pFenetre = NULL;        //Pointeur sur la fenêtre
	FMOD_CHANNEL *channelEnCours = NULL;    //Pour le contrôle des différents canaux audios

	sprite images[50] = {{NULL}, {0}};   //Tableau des images
	TTF_Font *polices[10] = {NULL};		//Tableau des polices

	Options *pOptions = NULL;
	char options[50][50] = {{""}};

	FILE *pFichierErreur = fopen("ressources/ErreursLog.txt", "a");        //Pointeur sur le fichier d'erreurs

	SDL_Surface *surf = NULL;
	SDL_Texture *pEcranChargement = NULL;

	Animation anim[10];

	int erreur=0;

	LectureOptions(options);
	pOptions = DecouperOptions(options);

	Initialisation(&pMoteurRendu, pFichierErreur, &pFenetre, pOptions);     //Initialisation des principaux éléments


	if(BMusique)
	{
		FMOD_System_GetChannel(pMoteurSon, M_MENU, &channelEnCours);
		FMOD_Channel_SetPaused(channelEnCours, true);

		FMOD_System_PlaySound(pMoteurSon, M_LOAD, pSons->music[M_LOAD], true, NULL);        // On lit la musique
		FMOD_System_GetChannel(pMoteurSon, M_LOAD, &channelEnCours);
		FMOD_Channel_SetVolume(channelEnCours, Volume/100.0);
		FMOD_Channel_SetPaused(channelEnCours, false);
	}

	surf = IMG_Load("ressources/img/load.png");
	pEcranChargement = SDL_CreateTextureFromSurface(pMoteurRendu, surf);

	SDL_ShowCursor(false);

	SDL_SetRenderDrawColor(pMoteurRendu, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(pMoteurRendu);
	SDL_RenderCopy(pMoteurRendu, pEcranChargement, NULL, NULL);
	SDL_RenderPresent(pMoteurRendu);


	SDL_EventState(SDL_TEXTEDITING, SDL_DISABLE);   //Désactivation du traitement des événements dont on a pas besoin.
	SDL_EventState(SDL_TEXTINPUT, SDL_DISABLE);

	SDL_DisableScreenSaver();       //Désactivation de l'écran de veille.

	erreur = Chargement(images, pMoteurRendu, polices, anim);

	if(erreur == 1)     //Chargement des images, des polices, des sons, ...
	{
		fprintf(pFichierErreur, "Erreur lors du chargement des images. Veuillez vérifier leur présence dans le dossier de l'exécutable.\n");
		exit(EXIT_FAILURE);
	}
	else if (erreur == 2)
	{
		fprintf(pFichierErreur, "Erreur lors du chargement des polices. Veuillez vérifier leur présence dans le dossier de l'exécutable.\n");
		exit(EXIT_FAILURE);
	}

	if (BMusique)
	{
		FMOD_System_GetChannel(pMoteurSon, M_LOAD, &channelEnCours);
		FMOD_Channel_SetPaused(channelEnCours, true);

		FMOD_Sound_SetLoopCount(pSons->music[M_JEU], -1);      // On active la lecture en boucle

		FMOD_System_PlaySound(pMoteurSon, M_JEU, pSons->music[M_JEU], true, NULL);        // On lit la musique
		FMOD_System_GetChannel(pMoteurSon, M_JEU, &channelEnCours);
		FMOD_Channel_SetVolume(channelEnCours, Volume/100.0);
		FMOD_Channel_SetPaused(channelEnCours, false);
	}


	if (strcmp(mode, "jouer") == 0)
	{
		InitialiserInfos(pOptions);
		Boucle_principale(pJoueur, images, anim, pMoteurRendu, pMoteurSon, pSons, polices);       //Boucle du jeu

		if (pJoueur->connexion == 1)
		{
			SauverMySql(pJoueur);
		}
	}
	else if (strcmp(mode, "editeur") == 0)
	{
		Editeur(pMoteurRendu, images, pMoteurSon, pSons, polices);
	}

	/*Libération de la mémoire*/
	LibererMemoire(pMoteurRendu, images, anim, polices, pFenetre, pOptions);

	fclose(pFichierErreur);

	return 0;
}

void LibererMemoire(SDL_Renderer *pMoteurRendu, sprite images[], Animation anim[], TTF_Font *polices[], SDL_Window *pFenetre, Options *pOptions)
{
	int i, j;

	SDL_DestroyRenderer(pMoteurRendu);
	SDL_DestroyWindow(pFenetre);

	for(i=0; i<10; i++)
	{
		SDL_DestroyTexture(images[i].pTextures[0]);

		j=0;

		while(anim[i].img[j] !=NULL)
		{
			SDL_DestroyTexture(anim[i].img[j]);
			j++;
		}
	}

	SDL_DestroyTexture(images[VIE].pTextures[1]);

	for(i=0; i<=SNICKY_GRAND; i++)
	{
		TTF_CloseFont(polices[i]);
	}

	free(pOptions);

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

int SauverMySql(Joueur *pJoueur)
{
	MYSQL *mysql = mysql_init(NULL);
	char requete[255] = "";

	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "default");

	if(mysql_real_connect(mysql, "mysql1.alwaysdata.com", "89504_beaussart", "beaussart62", "ballsandmovement_players", 3306, NULL, 0))
	{
		sprintf(requete, "UPDATE projetz SET score_max = %ld, niveau_max = %d, autre = '%s' WHERE pseudo = '%s'", infos.score, infos.niveau, pJoueur->pseudo, pJoueur->autre);
		mysql_query(mysql, requete);

		mysql_close(mysql);
	}

	return 0;
}

void InitialiserInfos(Options *pOptions)
{
	infos.niveau = 1;
	infos.score = 1000;
	infos.vies = pOptions->vies;
	infos.viesInitiales = pOptions->vies;
	infos.compteurTemps = 0;
	infos.bonus &= AUCUN_BONUS;
}


//Fin du fichier JeuSDL.c
