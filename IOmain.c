/*
Projet-ISN

Fichier: IOmain.c

Contenu: Fonctions d'entrées sorties (chargements, sauvegardes de fichiers, ...)

Actions: C'est ici que se trouve les fonctions qui lisent ou écrivent sur le disques, qui gèrent le clavier et la souris.

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
#include <gtk\gtk.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "jeu.h"

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;
double Volume, Largeur, Hauteur;
extern InfoDeJeu infos;

int InitialisationSon(FMOD_SYSTEM **ppMoteurSon, FILE *pFichierErreur, Sons *pSons)
{
	FMOD_System_Create(ppMoteurSon);        //Création du moteur sonore

	if(*ppMoteurSon == NULL)
	{
		fprintf(pFichierErreur, "Erreur lors de la création du moteur sonore\n");
		exit(EXIT_FAILURE);
	}

	if(FMOD_System_Init(*ppMoteurSon, 32, FMOD_INIT_NORMAL, NULL) != FMOD_OK)         //Initialisation du moteur sonore
	{
		fprintf(pFichierErreur, "Erreur lors de l'initialisation du moteur sonore\n");
		exit(EXIT_FAILURE);
	}

	if (ChargementMusic(pSons, *ppMoteurSon))
	{
		fprintf(pFichierErreur, "Erreur lors de l'ouverture des fichiers sons\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}

int Initialisation(SDL_Renderer **ppMoteurRendu, FILE *pFichierErreur, SDL_Window **ppFenetre, Options *pOptions)
{
	/* Initialisations et vérifications */
	if(SDL_Init (SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_TIMER) != 0)
	{
		fprintf(pFichierErreur, "Erreur: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	if((IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF)&(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF)) != (IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF))
	{
		fprintf(pFichierErreur, "Erreur: %s\n", IMG_GetError());
		exit(EXIT_FAILURE);
	}

	if(TTF_Init() != 0)
	{
		fprintf(pFichierErreur, "Erreur: %s\n", TTF_GetError());
		exit(EXIT_FAILURE);
	}

	srand((unsigned int)time(NULL));      //Initialisation du moteur de nombre aléatoire

	if (pOptions->fullScreen)
	{
		*ppFenetre = SDL_CreateWindow("ProjetZ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, pOptions->largeur, pOptions->hauteur, SDL_WINDOW_SHOWN|SDL_WINDOW_FULLSCREEN_DESKTOP);//Ouverture de la fenêtre
	}
	else
	{
		*ppFenetre = SDL_CreateWindow("ProjetZ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, pOptions->largeur, pOptions->hauteur, SDL_WINDOW_SHOWN);//Ouverture de la fenêtre
	}

	*ppMoteurRendu = SDL_CreateRenderer(*ppFenetre, -1, SDL_RENDERER_ACCELERATED);      //Création du moteur de rendu

	if (*ppFenetre == NULL || *ppMoteurRendu == NULL)
	{
		fprintf(pFichierErreur, "Erreur lors de la création de la fenêtre ou du moteur de rendu.");
		exit(EXIT_FAILURE);
	}

	SDL_SetWindowIcon(*ppFenetre, IMG_Load("ressources/img/z.png"));     //Icône de la fenêtre

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");     //Activation du filtrage anisotropique
	SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "1");
	SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1");

	/* Définition d'une taille logique pour les calculs d'images (le GPU fera une mise à échelle au moment de l'affichage selon la résolution maximale supportée par la machine, le ratio sera conservé) */
	SDL_RenderSetLogicalSize(*ppMoteurRendu, pOptions->largeur, pOptions->hauteur);

	return 0;
}

int Chargement (sprite images[], SDL_Renderer *pMoteurRendu, TTF_Font *polices[], Animation anim[])
{
	if(ChargementTextures(pMoteurRendu ,images) == -1)
	{
		return 1;
	}

	if(ChargementPolices(polices) == -1)
	{
		return 2;
	}

	if (ChargementAnimation(pMoteurRendu, anim) == -1)
	{
		return 3;
	}

	return 0;
}

int ChargementTextures(SDL_Renderer *pMoteurRendu, sprite images[])
{
	SDL_Surface *sImages[50] = {NULL}, *pSurfFondTextes;  //Tableau de surfaces +
	int rmask, gmask, bmask, amask;
	int i=0;                          //Variable de compteur

	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;

	pSurfFondTextes = SDL_CreateRGBSurface(0, 100, 100, 32, rmask, gmask, bmask, amask);
	SDL_FillRect(pSurfFondTextes, NULL, SDL_MapRGBA(pSurfFondTextes->format, 0, 0, 0, 140));
	images[FOND_TEXTES].pTextures[0] = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfFondTextes);
	SDL_FreeSurface(pSurfFondTextes);


	sImages[CURSEUR] = IMG_Load("ressources/img/curseur.png");

	if(sImages[CURSEUR] == NULL)
	{
		return -1;
	}

	sImages[BOULE_BLEUE] = IMG_Load("ressources/img/boule_bleue.png");     //Chargement des images en surface et vérification

	if(sImages[BOULE_BLEUE] == NULL)
	{
		return -1;
	}

	sImages[BOULE_MAGENTA] = IMG_Load("ressources/img/boule_magenta.png");

	if(sImages[BOULE_MAGENTA] == NULL)
	{
		return -1;
	}

	sImages[BOULE_VERTE] = IMG_Load("ressources/img/boule_verte.png");

	if(sImages[BOULE_VERTE] == NULL)
	{
		return -1;
	}

	sImages[MISSILE] = IMG_Load("ressources/img/missile.png");

	if(sImages[MISSILE] == NULL)
	{
		return -1;
	}

	sImages[VORTEX_BLEU] = IMG_Load("ressources/img/vortexBleu.png");

	if (sImages[VORTEX_BLEU] == NULL)
	{
		return -1;
	}

	sImages[VORTEX_VERT] = IMG_Load("ressources/img/vortexVert.png");

	if (sImages[VORTEX_VERT] == NULL)
	{
		return -1;
	}

	sImages[GEMMES] = IMG_Load("ressources/img/gemmes.png");

	if (sImages[GEMMES] == NULL)
	{
		return -1;
	}

	sImages[VIE] = IMG_Load("ressources/img/heart_plein.png");

	if(sImages[VIE] == NULL)
	{
		return -1;
	}

	sImages[VIE+1] = IMG_Load("ressources/img/heart_vide.png");

	if(sImages[VIE+1] == NULL)
	{
		return -1;
	}

	for(i = 0; i <= VIE; i++)               //Conversion des surfaces en textures pour le moteur de rendu
	{
		images[i].pTextures[0] = SDL_CreateTextureFromSurface(pMoteurRendu, sImages[i]);
	}

	images[VIE].pTextures[1] = SDL_CreateTextureFromSurface(pMoteurRendu, sImages[VIE+1]);

	DestructionSurfaces(sImages);       //Libération de la mémoire utilisée par les surfaces, elles sont inutiles puisqu'on les a converties en textures

	return 0;
}

int GestionEvenements(ClavierSouris *entrees)
{
	SDL_Event evenement;        //Variable de type événement

	SDL_PollEvent(&evenement);        //Récupération du prochain événement en queue

	switch (evenement.type)        //On regarde le type d'évènement
	{
	case SDL_QUIT:
		entrees->fermeture = true;
		break;

	case SDL_KEYDOWN:
		switch (evenement.key.keysym.sym)      //On regarde quelle touche a été enfoncée
		{
		case SDLK_ESCAPE:
			entrees->clavier[ECHAP] = true;
			break;

		case SDLK_SPACE:
			entrees->clavier[ESPACE] = true;
			break;

		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			entrees->clavier[ENTREE] = true;
			break;

		case SDLK_RIGHT:
			entrees->clavier[DROITE] = true;
			break;

		case SDLK_LEFT:
			entrees->clavier[GAUCHE] = true;
			break;

		case SDLK_DOWN:
			entrees->clavier[BAS] = true;
			break;

		case SDLK_UP:
			entrees->clavier[HAUT] = true;
			break;

		case SDLK_z:
			entrees->clavier[Z] = true;
			break;

		case SDLK_s:
			entrees->clavier[S] = true;
			break;

		case SDLK_q:
			entrees->clavier[Q] = true;
			break;

		case SDLK_d:
			entrees->clavier[D] = true;
			break;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			entrees->clavier[SHIFT] = true;
			break;

		case SDLK_j:
			entrees->clavier[J] = true;
			break;

		case SDLK_l:
			entrees->clavier[L] = true;
			break;

		case SDLK_k:
			entrees->clavier[K] = true;
			break;

		case SDLK_i:
			entrees->clavier[I] = true;
			break;

		case SDLK_m:
			entrees->clavier[M] = true;
			break;

		case SDLK_p:
			entrees->clavier[P] = true;
			break;

		case SDLK_F5:
			entrees->clavier[F5] = true;
			break;

		default:
			break;
		}

		break;

	case SDL_KEYUP:
		switch (evenement.key.keysym.sym)      //On regarde quelle touche a été relâchée
		{
		case SDLK_ESCAPE:
			entrees->clavier[ECHAP] = false;
			break;

		case SDLK_SPACE:
			entrees->clavier[ESPACE] = false;
			break;

		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			entrees->clavier[ENTREE] = false;
			break;

		case SDLK_z:
			entrees->clavier[Z] = false;
			break;

		case SDLK_s:
			entrees->clavier[S] = false;
			break;

		case SDLK_q:
			entrees->clavier[Q] = false;
			break;

		case SDLK_d:
			entrees->clavier[D] = false;
			break;

		case SDLK_RIGHT:
			entrees->clavier[DROITE] = false;
			break;

		case SDLK_LEFT:
			entrees->clavier[GAUCHE] = false;
			break;

		case SDLK_DOWN:
			entrees->clavier[BAS] = false;
			break;

		case SDLK_UP:
			entrees->clavier[HAUT] = false;
			break;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			entrees->clavier[SHIFT] = false;

		case SDLK_j:
			entrees->clavier[J] = false;
			break;

		case SDLK_l:
			entrees->clavier[L] = false;
			break;

		case SDLK_k:
			entrees->clavier[K] = false;
			break;

		case SDLK_i:
			entrees->clavier[I] = false;
			break;

		case SDLK_m:
			entrees->clavier[M] = false;
			break;

		case SDLK_p:
			entrees->clavier[P] = false;
			break;

		case SDLK_F5:
			entrees->clavier[F5] = false;
			break;

		default:
			break;
		}

		break;

	case SDL_MOUSEBUTTONDOWN:
		switch(evenement.button.button)		//On regarde quel bouton a été pressé
		{
		case SDL_BUTTON_LEFT:
			entrees->souris.touches[C_GAUCHE] = true;
			break;

		case SDL_BUTTON_RIGHT:
			entrees->souris.touches[C_DROIT] = true;
			break;

		case SDL_BUTTON_MIDDLE:
			entrees->souris.touches[C_MOLETTE] = true;
			break;

		default:
			break;
		}

		break;

	case SDL_FINGERDOWN:
		if(evenement.tfinger.x > 0.4 && evenement.tfinger.x < 0.6)
		{
			entrees->souris.touches[C_MOLETTE] = true;
			entrees->souris.touches[C_GAUCHE] = false;
			entrees->souris.touches[C_DROIT] = false;
		}

		break;

	case SDL_MOUSEBUTTONUP:
		switch(evenement.button.button)		//On regarde quel bouton a été relaché
		{
		case SDL_BUTTON_LEFT:
			entrees->souris.touches[C_GAUCHE] = false;
			break;

		case SDL_BUTTON_RIGHT:
			entrees->souris.touches[C_DROIT] = false;
			break;

		case SDL_BUTTON_MIDDLE:
			entrees->souris.touches[C_MOLETTE] = false;
			break;

		default:
			break;
		}

		break;

	case SDL_FINGERUP:
		if(evenement.tfinger.x > 0.4 && evenement.tfinger.x < 0.6)
		{
			entrees->souris.touches[C_MOLETTE] = false;
		}

		break;

	case SDL_MOUSEMOTION:		//On prend les coordonnées de la souris si elle a bougé
		entrees->souris.position.x = evenement.motion.x;
		entrees->souris.position.y = evenement.motion.y;
		break;

	case SDL_MOUSEWHEEL:		//On regarde de combien a défilé la molette de la souris
		entrees->souris.scroll = evenement.wheel.y;
		break;

	default:
		break;
	}

	return 0;
}

int ChargementPolices(TTF_Font *polices[])
{
	/* On charge les polices dont on aura besoin en différentes tailles */

	polices[ARIAL] = TTF_OpenFont("ressources/fonts/ARIALUNI.ttf", 25);

	if(polices[ARIAL] == NULL)
	{
		return -1;
	}

	polices[ARIAL_MOYEN] = TTF_OpenFont("ressources/fonts/ARIALUNI.ttf", 50);

	if(polices[ARIAL_MOYEN] == NULL)
	{
		return -1;
	}

	polices[ARIAL_GRAND] = TTF_OpenFont("ressources/fonts/ARIALUNI.ttf", 80);

	if(polices[ARIAL_GRAND] == NULL)
	{
		return -1;
	}

	polices[SNICKY] = TTF_OpenFont("ressources/fonts/Snickles.ttf", 20);

	if(polices[SNICKY] == NULL)
	{
		return -1;
	}

	polices[SNICKY_MOYEN] = TTF_OpenFont("ressources/fonts/Snickles.ttf", 40);

	if(polices[SNICKY_MOYEN] == NULL)
	{
		return -1;
	}

	polices[SNICKY_GRAND] = TTF_OpenFont("ressources/fonts/Snickles.ttf", 60);

	if(polices[SNICKY_GRAND] == NULL)
	{
		return -1;
	}

	return 0;
}

int ChargementMusic (Sons *pSons, FMOD_SYSTEM *pMoteurSon)
{
	/* On charge les musiques et on active le mode "en boucle" pour celles de fond. */

	if(FMOD_System_CreateStream(pMoteurSon, "ressources/music/menu.mp3", FMOD_2D|FMOD_HARDWARE|FMOD_LOOP_NORMAL, NULL, &pSons->music[M_MENU]) != FMOD_OK)
	{
		return -1;
	}

	if(FMOD_System_CreateStream(pMoteurSon, "ressources/music/win.mp3", FMOD_2D|FMOD_HARDWARE, NULL, &pSons->music[M_GAGNE]) != FMOD_OK)
	{
		return -1;
	}

	if(FMOD_System_CreateStream(pMoteurSon, "ressources/music/credits.mp3", FMOD_2D|FMOD_HARDWARE, NULL, &pSons->music[M_CREDITS]) != FMOD_OK)
	{
		return -1;
	}

	if(FMOD_System_CreateStream(pMoteurSon, "ressources/music/jeu.mp3", FMOD_2D|FMOD_HARDWARE|FMOD_LOOP_NORMAL, NULL, &pSons->music[M_JEU]) != FMOD_OK)
	{
		return -1;
	}

	if(FMOD_System_CreateStream(pMoteurSon, "ressources/music/perdu.mp3", FMOD_2D|FMOD_HARDWARE|FMOD_LOOP_NORMAL, NULL, &pSons->music[M_PERDU]) != FMOD_OK)
	{
		return -1;
	}

	if(FMOD_System_CreateStream(pMoteurSon, "ressources/music/load.mp3", FMOD_2D|FMOD_HARDWARE, NULL, &pSons->music[M_LOAD]) != FMOD_OK)
	{
		return -1;
	}

	if(FMOD_System_CreateSound(pMoteurSon, "ressources/music/boule_boum.mp3", FMOD_2D|FMOD_HARDWARE, NULL, &pSons->bruits[S_BOULE_BOUM]) != FMOD_OK)
	{
		return -1;
	}

	if (FMOD_System_CreateSound(pMoteurSon, "ressources/music/gong.wav", FMOD_2D|FMOD_HARDWARE, NULL, &pSons->bruits[S_BOULE_BOULE]) != FMOD_OK)
	{
		return -1;
	}

	if (FMOD_System_CreateSound(pMoteurSon, "ressources/music/click.mp3", FMOD_2D|FMOD_HARDWARE, NULL, &pSons->bruits[S_CLICK]) != FMOD_OK)
	{
		return -1;
	}

	if (FMOD_System_CreateSound(pMoteurSon, "ressources/music/bonus.wav", FMOD_2D|FMOD_HARDWARE, NULL, &pSons->bruits[S_BONUS]) != FMOD_OK)
	{
		return -1;
	}

	if (FMOD_System_CreateSound(pMoteurSon, "ressources/music/slide.mp3", FMOD_2D|FMOD_HARDWARE, NULL, &pSons->bruits[S_SAVE]) != FMOD_OK)
	{
		return -1;
	}

	if (FMOD_System_CreateSound(pMoteurSon, "ressources/music/alert.wav", FMOD_2D|FMOD_HARDWARE|FMOD_LOOP_NORMAL, NULL, &pSons->bruits[S_ALARME]) != FMOD_OK)
	{
		return -1;
	}

	return 0;
}

int DestructionSurfaces(SDL_Surface *sImages[])
{
	int i=0;

		while (i++, sImages[i] != NULL)
		{
			SDL_FreeSurface(sImages[i]);
		}

	return 0;
}

Map* ChargementNiveau(SDL_Renderer *pMoteurRendu, char mode[], int level)
{
	int i=0, j=0, k=0;
	SDL_Surface *pSurface = NULL, *pSurfaceFond = NULL;
	FILE *pFichierNiveau = NULL, *pFichierErreur = NULL;
	char ligne[50] = {0};
	Map *pMap = NULL;

	pFichierErreur = fopen("ressources/ErreursLog.txt", "a");

	if(pFichierErreur == NULL)
	{
		return NULL;
	}

	sprintf(ligne, "ressources/fonds/map%d.jpg", ((rand()%12)+1));

	pSurface = IMG_Load("ressources/img/tile_sol.png");
	pSurfaceFond = IMG_Load(ligne);

	if(pSurface == NULL)
	{
		fprintf(pFichierErreur, "Erreur lors du chargement du fichier tile_sol.png\n");
		return NULL;
	}

	if(pSurfaceFond == NULL)
	{
		fprintf(pFichierErreur, "Erreur lors du chargement du fichier %s\n", ligne);
		return NULL;
	}

	pMap = malloc(sizeof(Map));

	if(pMap == NULL)
	{
		fprintf(pFichierErreur, "Erreur lors de l'allocation dynamique de la mémoire\n");
		return NULL;
	}

	pMap->tileset = SDL_CreateTextureFromSurface(pMoteurRendu, pSurface);
	pMap->fond = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfaceFond);

	pMap->HAUTEUR_TILE = 96;
	pMap->LARGEUR_TILE = 94;
	pMap->nbtilesX = 5;
	pMap->nbtilesY = 4;

	pMap->props = malloc(pMap->nbtilesX * pMap->nbtilesY * sizeof(TileProp));

	if(pMap->props == NULL)
	{
		fprintf(pFichierErreur, "Erreur lors de l'allocation dynamique de la mémoire\n");
		return NULL;
	}

	for(i=0; i< pMap->nbtilesY; i++)
	{
		for (j=0; j< pMap->nbtilesX; j++)
		{
			pMap->props[k].src.h = pMap->HAUTEUR_TILE;
			pMap->props[k].src.w = pMap->LARGEUR_TILE;
			pMap->props[k].src.x = j*100 +3;
			pMap->props[k].src.y = i*100 +2;
			k++;
		}
	}

	pFichierNiveau = fopen("ressources/level.lvl", "r");

	if (pFichierNiveau == NULL)
	{
		fprintf(pFichierErreur, "Erreur: le fichier level.lvl est introuvable\n");
		return NULL;
	}

	for (i=0; i<level-1; i++)
	{
		fgets(ligne, 50, pFichierNiveau);

		do
		{
			fgets(ligne, 50, pFichierNiveau);
		}
		while (strcmp(ligne, "##--##\n") != 0);
	}

	fgets(ligne, 50, pFichierNiveau);

	if(strcmp(ligne, "#taille\n") == 0)
	{
		fgets(ligne, 50, pFichierNiveau);
		pMap->nbtiles_hauteur_monde = Arrondir(strtol(ligne, NULL, 10));

		fgets(ligne, 50, pFichierNiveau);
		pMap->nbtiles_largeur_monde = Arrondir(strtol(ligne, NULL, 10));
	}
	else
	{
		fprintf(pFichierErreur, "Plus de niveau !\n");
		return NULL;
	}

	pMap->plan = malloc(pMap->nbtiles_largeur_monde * sizeof(int));
	pMap->planObjets = malloc(pMap->nbtiles_largeur_monde * sizeof(int));

	if(pMap->plan == NULL || pMap->planObjets == NULL)
	{
		fprintf(pFichierErreur, "Erreur d'allocation dynamique de la mémoire\n");
		return NULL;
	}

	for(i=0; i< pMap->nbtiles_largeur_monde; i++)
	{
		pMap->plan[i] = malloc(pMap->nbtiles_hauteur_monde * sizeof(int));
		pMap->planObjets[i] = malloc(pMap->nbtiles_hauteur_monde * sizeof(int));

		if(pMap->plan[i] == NULL || pMap->planObjets[i] == NULL)
		{
			fprintf(pFichierErreur, "Erreur d'allocation dynamique de la mémoire\n");
			return NULL;
		}
	}

	if (strcmp(mode, "jouer") == 0)
	{
		while(strcmp(ligne, "#map\n") != 0)
		{
			fgets(ligne, 50, pFichierNiveau);
		}

		for (i=0; i< pMap->nbtiles_largeur_monde; i++)
		{
			for (j=0; j< pMap->nbtiles_hauteur_monde; j++)
			{
				if (fgets(ligne, 50, pFichierNiveau) != NULL)
				{
					pMap->plan[i][j] = Arrondir(strtol(ligne, NULL, 10));
				}
				else
				{
					fprintf(pFichierErreur, "Erreur: level.lvl, fichier corrompu.\n");
					return NULL;
				}
			}
		}

		fgets(ligne, 50, pFichierNiveau);

		if (strcmp(ligne, "#mapObjets\n") != 0)
		{
			fprintf(pFichierErreur, "Erreur: level.lvl, fichier corrompu.\n");
			return NULL;
		}

		for (i=0; i< pMap->nbtiles_largeur_monde; i++)
		{
			for (j=0; j< pMap->nbtiles_hauteur_monde; j++)
			{
				if (fgets(ligne, 50, pFichierNiveau) != NULL)
				{
					pMap->planObjets[i][j] = Arrondir(strtol(ligne, NULL, 10));
				}
				else
				{
					fprintf(pFichierErreur, "Erreur: level.lvl, fichier corrompu.\n");
					return NULL;
				}
			}
		}

		fclose(pFichierNiveau);
	}
	else
	{
		for (i=0; i< pMap->nbtiles_largeur_monde; i++)
		{
			for (j=0; j< pMap->nbtiles_hauteur_monde; j++)
			{
				pMap->plan[i][j] = VIDE;
				pMap->planObjets[i][j] = AUCUN_BONUS;
			}
		}
	}

	fclose(pFichierErreur);
	SDL_FreeSurface(pSurface);
	SDL_FreeSurface(pSurfaceFond);

	return pMap;
}

int DestructionMap(Map *pMap)
{
	int i;		//Variable compteur

	/* On libère la mémoire */

	SDL_DestroyTexture(pMap->tileset);


	for(i=0; i< pMap->nbtiles_largeur_monde; i++)
	{
		free(pMap->plan[i]);
		free(pMap->planObjets[i]);
	}

	free(pMap->plan);
	free(pMap->planObjets);
	free(pMap->props);
	free(pMap);

	return 0;
}

int EntreesZero(ClavierSouris *pEntrees)
{
	int i=0;	//Variable compteur

	/* On initialise tous les champs à zéro */

	for(i=0; i< 200; i++)
	{
		pEntrees->clavier[i] = false;
	}

	for (i=0; i< 3; i++)
	{
		pEntrees->souris.touches[i] = false;
	}

	pEntrees->souris.position.x = 0;
	pEntrees->souris.position.y = 0;

	pEntrees->souris.scroll = 0;

	pEntrees->fermeture = false;

	return 0;
}

int ChargementAnimation(SDL_Renderer *pMoteurRendu, Animation anim[])
{
	int i=0, j=0;			//Compteur
	SDL_Surface *pSurface = NULL;	//Pointeur sur la surface
	char chaine[256];		//Chaine de caractères

	for (i=0; i< 200; i++)
	{
		sprintf(chaine, "ressources/anim/boum (%d).png", i+1);		//On choisi le fichier
		pSurface = IMG_Load(chaine);

		if (pSurface != NULL)
		{
			anim[ANIM_0].img[i] = SDL_CreateTextureFromSurface(pMoteurRendu, pSurface);		//On crée une texture tant qu'il y a des images
		}
		else
		{
			anim[ANIM_0].img[i] = NULL;
		}

		SDL_FreeSurface(pSurface);
	}

	anim[ANIM_0].animEnCours = true;

	for (i=1; i<10; i++)
	{
		for (j=0; j<200; j++)
		{
			anim[i].img[j] = NULL;
		}
	}

	return 0;
}


int MessageInformations(const char messageInfos[], TTF_Font *polices[], SDL_Renderer *pMoteurRendu, ClavierSouris *pEntrees)
{
	texte informations;
	SDL_Color blancOpaque= {255, 255, 255, SDL_ALPHA_OPAQUE};

	sprintf(informations.chaines[0], messageInfos);

	informations.surface = TTF_RenderText_Blended(polices[SNICKY_GRAND], informations.chaines[0], blancOpaque);
	TTF_SizeText(polices[SNICKY_GRAND], informations.chaines[0], &informations.positions[0].w, &informations.positions[0].h);

	informations.pTextures[0] = SDL_CreateTextureFromSurface(pMoteurRendu, informations.surface);

	SDL_FreeSurface(informations.surface);

	informations.surface = TTF_RenderText_Blended(polices[SNICKY_MOYEN], "Appuyez sur ENTREE ou sur ECHAP", blancOpaque);
	TTF_SizeText(polices[SNICKY_MOYEN], "Appuyez sur ENTREE ou sur ECHAP", &informations.positions[1].w, &informations.positions[1].h);

	informations.pTextures[1] = SDL_CreateTextureFromSurface(pMoteurRendu, informations.surface);

	SDL_FreeSurface(informations.surface);

	informations.positions[0].x = Largeur/2 - informations.positions[0].w/2;
	informations.positions[0].y = Hauteur/2 - informations.positions[0].h/2;

	informations.positions[1].x = Largeur/2 - informations.positions[1].w/2;
	informations.positions[1].y = Hauteur/2 - informations.positions[1].h/2 + informations.positions[0].h;


	while (!pEntrees->clavier[ENTREE] && !pEntrees->clavier[ECHAP])
	{
		SDL_SetRenderDrawColor(pMoteurRendu, 30, 30, 30, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(pMoteurRendu);

		SDL_RenderCopy(pMoteurRendu, informations.pTextures[0], NULL, &informations.positions[0]);
		SDL_RenderCopy(pMoteurRendu, informations.pTextures[1], NULL, &informations.positions[1]);

		SDL_RenderPresent(pMoteurRendu);

		GestionEvenements(pEntrees);
	}

	SDL_DestroyTexture(informations.pTextures[0]);


	if (pEntrees->clavier[ENTREE])
	{
		pEntrees->clavier[ENTREE]=false;
		return 1;
	}
	else
	{
		pEntrees->clavier[ECHAP]=false;
		return 0;
	}
}

//Fin du fichier IOmain.c
