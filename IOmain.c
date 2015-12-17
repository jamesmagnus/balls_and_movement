/*
Projet-ISN

Fichier: IOmain.c

Contenu: Fonctions d'initialisation et de chargement des fichiers pour le jeu.

Actions: C'est ici que se trouve les fonctions qui lisent sur le disques, qui g�rent le clavier et la souris ou qui initialisent des structures.

Biblioth�ques utilis�es: Biblioth�ques standards, SDL, SDL_image, SDL_ttf, FMOD, GTK

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <md5.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <fmod.h>
#include <gtk/gtk.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "jeu.h"

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;		//Lien vers les variables globales d�clar�es dans main.c
extern double Volume, Largeur, Hauteur;
extern InfoDeJeu infos;

int InitialisationSon(FMOD_SYSTEM **ppMoteurSon, FILE *pFichierErreur, Sons *pSons)
{
    /* Cette fonction initialise et charge tout ce qu'il faut pour lire la musique et les sons, elle est appel�e au lancement du programme */

    FMOD_System_Create(ppMoteurSon);        //Cr�ation du moteur sonore via un pointeur sur un pointeur

    if(*ppMoteurSon == NULL)	//V�rification
    {
        fprintf(pFichierErreur, "Erreur lors de la cr�ation du moteur sonore\n");
        exit(EXIT_FAILURE);	//On quitte
    }

    if(FMOD_System_Init(*ppMoteurSon, 32, FMOD_INIT_NORMAL, NULL) != FMOD_OK)         //Initialisation du moteur sonore et v�rification
    {
        fprintf(pFichierErreur, "Erreur lors de l'initialisation du moteur sonore\n");
        exit(EXIT_FAILURE);
    }

    if (ChargementMusic(pSons, *ppMoteurSon))	//On charge les fichiers sons et on quitte s'il en manque un
    {
        fprintf(pFichierErreur, "Erreur lors de l'ouverture des fichiers sons\n");
        exit(EXIT_FAILURE);
    }

    return 0;	//Tout c'est bien pass�
}

int Initialisation(SDL_Renderer **ppMoteurRendu, FILE *pFichierErreur, SDL_Window **ppFenetre, Options *pOptions)
{
    /* Cette fonction initialise la SDL lors du lancement du jeu ou de l'�diteur */

    /* Initialisations et v�rifications de la SDL et de ses extensions */
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

    srand((unsigned int)time(NULL));      //Initialisation du moteur de nombre al�atoire avec la date et l'heure comme base de travail

    if (pOptions->fullScreen)
    {
        *ppFenetre = SDL_CreateWindow("ProjetZ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, pOptions->largeur, pOptions->hauteur, SDL_WINDOW_SHOWN|SDL_WINDOW_FULLSCREEN_DESKTOP);	//Ouverture de la fen�tre en mode plein �cran
    }
    else
    {
        /* Ouverture de la fen�tre normalement sinon */
        *ppFenetre = SDL_CreateWindow("ProjetZ", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, pOptions->largeur, pOptions->hauteur, SDL_WINDOW_SHOWN);
    }

    *ppMoteurRendu = SDL_CreateRenderer(*ppFenetre, -1, SDL_RENDERER_ACCELERATED);      //Cr�ation du moteur de rendu

    if (*ppFenetre == NULL || *ppMoteurRendu == NULL)	//On v�rifie si la fen�tre et le moteur sont bien cr��s
    {
        fprintf(pFichierErreur, "Erreur lors de la cr�ation de la fen�tre ou du moteur de rendu.");
        exit(EXIT_FAILURE);
    }

    SDL_SetWindowIcon(*ppFenetre, IMG_Load("ressources/img/z.png"));     //Ic�ne de la fen�tre

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");     //Activation du filtrage anisotropique
    SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "1");
    SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1");

    /* D�finition d'une taille logique pour les calculs d'images (le GPU fera une mise � �chelle au moment de l'affichage selon la r�solution maximale support�e par la machine pour le mode plein �cran, le ratio sera conserv�) */
    SDL_RenderSetLogicalSize(*ppMoteurRendu, pOptions->largeur, pOptions->hauteur);

    return 0;
}

int Chargements(Sprite images[], SDL_Renderer *pMoteurRendu, TTF_Font *polices[], Animation anim[])
{
    if(ChargementTextures(pMoteurRendu ,images) == -1)	//On charge les images (boules, missile et vortexs)
    {
        return 1;
    }

    if(ChargementPolices(polices) == -1)	//On charge les fichiers de police TTF pour �crire avec la SDL
    {
        return 2;
    }

    if (ChargementAnimations(pMoteurRendu, anim) == -1)	//On charge les images pour les animations
    {
        return 3;
    }

    return 0;	//On renvoie 0 si tout c'est bien pass�
}

int ChargementTextures(SDL_Renderer *pMoteurRendu, Sprite images[])
{
    /* Cette fonction charge les images et les convertie en textures */

    SDL_Surface *sImages[50] = {NULL}, *pSurfFondTextes;  //Tableau de surfaces + un pointeur pour une autre surface
    int rmask, gmask, bmask, amask;		//Masques RVB pour cr�er des couleurs avec la SDL
    int i=0;                          //Compteur

    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;

    /* On cr�e un fond gris semi-opaque pour que le temps eet le score se voient mieux */
    pSurfFondTextes = SDL_CreateRGBSurface(0, 100, 100, 32, rmask, gmask, bmask, amask);
    SDL_FillRect(pSurfFondTextes, NULL, SDL_MapRGBA(pSurfFondTextes->format, 0, 0, 0, 140));
    images[FOND_TEXTES].pTextures[0] = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfFondTextes);	//On convertie en texture
    SDL_FreeSurface(pSurfFondTextes);	//On lib�re la surface

    /* On charge toutes les images comme des surfaces et on v�rifie que le chargement a r�ussi */
    sImages[CURSEUR] = IMG_Load("ressources/img/curseur.png");

    if(sImages[CURSEUR] == NULL)
    {
        return -1;
    }

    sImages[BOULE_BLEUE] = IMG_Load("ressources/img/boule_bleue.png");

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

    /* On charge le coeur vide un indice au dessus de celui plein car il n'existe pas dans l'�num�ration */
    sImages[VIE+1] = IMG_Load("ressources/img/heart_vide.png");

    if(sImages[VIE+1] == NULL)
    {
        return -1;
    }

    for(i = 0; i <= VIE; i++)   //Conversion des surfaces en textures pour le moteur de rendu
    {
        images[i].pTextures[0] = SDL_CreateTextureFromSurface(pMoteurRendu, sImages[i]);
    }

    /* Conversion du coeur vide, on le place dans le tableau des textures du Sprite 'VIE' */
    images[VIE].pTextures[1] = SDL_CreateTextureFromSurface(pMoteurRendu, sImages[VIE+1]);

    DestructionSurfaces(sImages);       //Lib�ration de la m�moire utilis�e par les surfaces, elles sont inutiles puisqu'on les a converties en textures

    return 0;
}

int GestionEvenements(ClavierSouris *entrees)
{
    /* Cette fonction tr�s longue prend le prochain �v�nement en file d'attente et met � jour la structure qui repr�sente l'�tat du clavier et de la souris */

    SDL_Event evenement;        //Structure de type event

    SDL_PollEvent(&evenement);        //R�cup�ration du prochain �v�nement en file

    switch (evenement.type)        //On regarde le type d'�v�nement
    {
    case SDL_QUIT:		//Demande de fermeture (Alt+F4 ou croix)
        entrees->fermeture = true;
        break;

    case SDL_KEYDOWN:
        switch (evenement.key.keysym.sym)      //On regarde quelle touche a �t� enfonc�e
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

        case SDLK_p:
            entrees->clavier[POLICE_ARIAL] = true;
            break;

        case SDLK_F5:
            entrees->clavier[F5] = true;
            break;

        case SDLK_a:
            entrees->souris.touches[C_MOLETTE] = true;
            break;
        }

        break;

    case SDL_KEYUP:
        switch (evenement.key.keysym.sym)      //On regarde quelle touche a �t� rel�ch�e
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

        case SDLK_p:
            entrees->clavier[POLICE_ARIAL] = false;
            break;

        case SDLK_F5:
            entrees->clavier[F5] = false;
            break;

        case SDLK_a:
            entrees->souris.touches[C_MOLETTE] = false;
            break;
        }

        break;

    case SDL_MOUSEBUTTONDOWN:
        switch(evenement.button.button)		//On regarde quel bouton a �t� press�
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
        }

        break;

    case SDL_FINGERDOWN:		//Pour les touchpad (ou les souris mac), on consid�re un clic de molette si c'est un clic dans la zone du milieu
        if(evenement.tfinger.x > 0.4 && evenement.tfinger.x < 0.6)
        {
            entrees->souris.touches[C_MOLETTE] = true;
            entrees->souris.touches[C_GAUCHE] = false;
            entrees->souris.touches[C_DROIT] = false;
        }

        break;

    case SDL_MOUSEBUTTONUP:
        switch(evenement.button.button)		//On regarde quel bouton a �t� relach�
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
        }

        break;

    case SDL_FINGERUP:
        entrees->souris.touches[C_MOLETTE] = false;
        break;

    case SDL_MOUSEMOTION:		//On prend les coordonn�es de la souris si elle a boug�
        entrees->souris.position.x = evenement.motion.x;
        entrees->souris.position.y = evenement.motion.y;
        break;

    case SDL_MOUSEWHEEL:		//On regarde de combien a d�fil� la molette de la souris
        entrees->souris.scroll = evenement.wheel.y;
        break;
    }

    return 0;
}

int ChargementPolices(TTF_Font *polices[])
{
    /* On charge les polices dont on aura besoin en diff�rentes tailles et on v�rifie que �a a march� */

    polices[POLICE_ARIAL] = TTF_OpenFont("ressources/fonts/ARIALUNI.ttf", 25);

    if(polices[POLICE_ARIAL] == NULL)
    {
        return -1;
    }

    polices[POLICE_ARIAL_MOYEN] = TTF_OpenFont("ressources/fonts/ARIALUNI.ttf", 50);

    if(polices[POLICE_ARIAL_MOYEN] == NULL)
    {
        return -1;
    }

    polices[POLICE_ARIAL_GRAND] = TTF_OpenFont("ressources/fonts/ARIALUNI.ttf", 80);

    if(polices[POLICE_ARIAL_GRAND] == NULL)
    {
        return -1;
    }

    polices[POLICE_SNICKY] = TTF_OpenFont("ressources/fonts/Snickles.ttf", 20);

    if(polices[POLICE_SNICKY] == NULL)
    {
        return -1;
    }

    polices[POLICE_SNICKY_MOYEN] = TTF_OpenFont("ressources/fonts/Snickles.ttf", 40);

    if(polices[POLICE_SNICKY_MOYEN] == NULL)
    {
        return -1;
    }

    polices[POLICE_SNICKY_GRAND] = TTF_OpenFont("ressources/fonts/Snickles.ttf", 60);

    if(polices[POLICE_SNICKY_GRAND] == NULL)
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

    if(FMOD_System_CreateStream(pMoteurSon, "ressources/music/perdu.mp3", FMOD_2D|FMOD_HARDWARE, NULL, &pSons->music[M_PERDU]) != FMOD_OK)
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

    if (FMOD_System_CreateSound(pMoteurSon, "ressources/music/gling.wav", FMOD_2D|FMOD_HARDWARE, NULL, &pSons->bruits[S_BOULE_BOULE]) != FMOD_OK)
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

    if (FMOD_System_CreateSound(pMoteurSon, "ressources/music/fall.wav", FMOD_2D|FMOD_HARDWARE, NULL, &pSons->bruits[S_TOMBE]) != FMOD_OK)
    {
        return -1;
    }

    if (FMOD_System_CreateSound(pMoteurSon, "ressources/music/levelup.wav", FMOD_2D|FMOD_HARDWARE, NULL, &pSons->bruits[S_SORTIE]) != FMOD_OK)
    {
        return -1;
    }

    return 0;
}

int DestructionSurfaces(SDL_Surface *sImages[])
{
    int i=0;

    /* Tant que le pointeur n'est pas NULL c'est qu'il reste des surfaces */
    while (i++, sImages[i] != NULL)
    {
        SDL_FreeSurface(sImages[i]);
    }

    return 0;
}

Map* ChargementNiveau(SDL_Renderer *pMoteurRendu, Joueur *pJoueur, int level, int *pEtatNiveau)
{
    int i=0, j=0, k=0;	//Compteur
    SDL_Surface *pSurface = NULL, *pSurfaceFond = NULL;	//Deux pointeurs sur une surface
    FILE *pFichierNiveau = NULL, *pFichierErreur = NULL;	//Deux pointeur sur des fichiers
    char ligne[50] = "";	//Une cha�ne pour lire une ligne du fichier
    Map *pMap = NULL;		//Un pointeur vers une structure Map que l'on va allouer et remplir avec le fichier
    char *c=NULL;

    pFichierErreur = fopen("ressources/ErreursLog.txt", "a");	//On ouvre le fichier d'erreur pour �crire d'�ventuelles erreurs

    if(pFichierErreur == NULL)
    {
        *pEtatNiveau = CHARGEMENT_ERREUR;	//On place un code d'erreur dans la variable
        return NULL;	//On arr�te si on a pas r�ussi � ouvrir le fichier
    }

    sprintf(ligne, "ressources/fonds/map%d.jpg", ((rand()%12)+1));	//On �crit dans la cha�ne le chemin d'une image de fond choisie al�atoirement
    pSurfaceFond = IMG_Load(ligne);	//On charge cette image

    pSurface = IMG_Load("ressources/img/tile_sol.png");	//On charge l'image avec tous les blocs de base pour faire un niveau

    /* On v�rifie si les deux images ont bien �t� charg�es */
    if(pSurface == NULL)
    {
        fprintf(pFichierErreur, "Erreur lors du chargement du fichier tile_sol.png\n");
        *pEtatNiveau = CHARGEMENT_ERREUR;	//On place un code d'erreur dans la variable
        return NULL;	//On arr�te si on a pas r�ussi � ouvrir le fichier
    }

    if(pSurfaceFond == NULL)
    {
        fprintf(pFichierErreur, "Erreur lors du chargement du fichier %s\n", ligne);
        *pEtatNiveau = CHARGEMENT_ERREUR;	//On place un code d'erreur dans la variable
        return NULL;	//On arr�te si on a pas r�ussi � ouvrir le fichier
    }

    /* On alloue la m�moire n�cessaire pour une structure Map */
    pMap = malloc(sizeof(Map));

    if(pMap == NULL)	//On v�rifie l'allocation
    {
        fprintf(pFichierErreur, "Erreur lors de l'allocation dynamique de la m�moire\n");
        *pEtatNiveau = CHARGEMENT_ERREUR;	//On place un code d'erreur dans la variable
        return NULL;	//On arr�te si on a pas r�ussi � allouer la m�moire
    }

    /* On convertie les surfaces en textures et on place les adresses dans les pointeurs fra�chements allou�s de la structure Map */
    pMap->tileset = SDL_CreateTextureFromSurface(pMoteurRendu, pSurface);
    pMap->fond = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfaceFond);

    /* On d�finie les valeurs de tailles et de nombres de tiles sur la texture avec tous les blocs */
    pMap->HAUTEUR_TILE = 96;
    pMap->LARGEUR_TILE = 94;
    pMap->nbtilesX = 5;
    pMap->nbtilesY = 4;

    /* On alloue de la m�moire pour une structure TileProp pour chaque bloc de terrain */
    pMap->props = malloc(pMap->nbtilesX * pMap->nbtilesY * sizeof(TileProp));

    if(pMap->props == NULL)	//On v�rifie
    {
        fprintf(pFichierErreur, "Erreur lors de l'allocation dynamique de la m�moire\n");
        *pEtatNiveau = CHARGEMENT_ERREUR;	//On place un code d'erreur dans la variable
        return NULL;	//On arr�te si on a pas r�ussi � allouer la m�moire
    }

    /* On parcourt un par un tous les blocs de la texture et on d�finie les masques pour afficher chaque bloc (taille d'un bloc et position sur la texture) */
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

    /* Enfin ouvre le fichier de niveau selon le mode jeu ou �diteur */
    /* On ouvre un fichier correct pour le mode �diteur car il faut les tailles de map par d�faut */
    if(pJoueur->mode == MODE_CAMPAGNE || (pJoueur->mode == MODE_EDITEUR && level == -1))
    {
        if(VerificationMD5("9D656D35ACFF9BDC6829986F9EC14A41", "ressources/level.lvl"))	//On v�rifie l'empreinte md5 du fichier
        {
            pFichierNiveau = fopen("ressources/level.lvl", "r");
        }
        else
        {
            *pEtatNiveau = CHARGEMENT_FICHIER_CORROMPU;	//On place un code d'erreur dans la variable
            return NULL;	//On arr�te car le fichier fourni n'est pas le bon
        }
    }
    else if (pJoueur->mode == MODE_PERSO || (pJoueur->mode == MODE_EDITEUR && level != -1))	//On ouvre le fichier personnel si c'est pour une �dition
    {
        pFichierNiveau = fopen("ressources/levelUser.lvl", "r");
    }

    if (pFichierNiveau == NULL)	//On v�rifie
    {
        fprintf(pFichierErreur, "Erreur: le fichier level.lvl ou levelUser.lvl est introuvable\n");
        *pEtatNiveau = CHARGEMENT_ERREUR;	//On place un code d'erreur dans la variable
        return NULL;	//On arr�te si on a pas r�ussi � ouvrir le fichier
    }

    /* On parcourt autant de niveau qu'il faut pour atteindre celui qu'il faut charger */
    for (i=0; i<level-1; i++)
    {
        do
        {
            fgets(ligne, 50, pFichierNiveau);
        }
        while (strcmp(ligne, "##--##\n") != 0);
    }

    /* On lit la premi�re ligne du niveau suivant (celui que l'on veut charger) */
    fgets(ligne, 50, pFichierNiveau);

    if(strcmp(ligne, "#titre\n") == 0)	//Si elle ne contient pas #titre\n c'est qu'on est arriv� � la fin du fichier
    {
        fgets(ligne, 50, pFichierNiveau);	//On lit le titre
        c = strstr(ligne, "\n");	//On recherche le retour � la ligne

        if(c != NULL)
        {
            *c = '\0';	//On le fait dispara�tre
        }

        sprintf(pMap->titre, ligne);	//On copie le titre dans la structure

        fgets(ligne, 50, pFichierNiveau);	//'#taille\n'

        /* S'il y a un niveau � charger on commence par lire sa taille */
        /* On convertie la cha�ne en nombre */
        fgets(ligne, 50, pFichierNiveau);
        pMap->nbtiles_hauteur_monde = strtol(ligne, NULL, 10);

        fgets(ligne, 50, pFichierNiveau);
        pMap->nbtiles_largeur_monde = strtol(ligne, NULL, 10);
    }
    else	//Fin du fichier, plus de niveau, on a gagn�
    {
        *pEtatNiveau = CHARGEMENT_GAGNE;	//On place un code d'erreur dans la variable
        return NULL;	//On retourne NULL comme il n'y a plus rien � charger
    }

    /* On alloue de la m�moire pour chaque colonne la map des blocs et chaque colonne de celle des bonus */
    pMap->plan = malloc(pMap->nbtiles_largeur_monde * sizeof(int*));
    pMap->planObjets = malloc(pMap->nbtiles_largeur_monde * sizeof(int*));

    if(pMap->plan == NULL || pMap->planObjets == NULL)	//On v�rifie
    {
        fprintf(pFichierErreur, "Erreur d'allocation dynamique de la m�moire\n");
        *pEtatNiveau = CHARGEMENT_ERREUR;	//On place un code d'erreur dans la variable
        return NULL;	//On arr�te si on a pas r�ussi � allouer la m�moire
    }

    /* Ensuite dans chacune de ces colonnes on va allouer de la m�moire pour chaque ligne */
    for(i=0; i< pMap->nbtiles_largeur_monde; i++)
    {
        pMap->plan[i] = malloc(pMap->nbtiles_hauteur_monde * sizeof(int));
        pMap->planObjets[i] = malloc(pMap->nbtiles_hauteur_monde * sizeof(int));

        if(pMap->plan[i] == NULL || pMap->planObjets[i] == NULL)	//On v�rifie � chaque fois
        {
            fprintf(pFichierErreur, "Erreur d'allocation dynamique de la m�moire\n");
            *pEtatNiveau = CHARGEMENT_ERREUR;	//On place un code d'erreur dans la variable
            return NULL;	//On arr�te si on a pas r�ussi � allouer la m�moire
        }
    }

    /* Maintenant on initialise les tailles et les positions de toutes les images selon si on est en mode jeu ou en mode �diteur */
    if (pJoueur->mode == MODE_CAMPAGNE || pJoueur->mode == MODE_PERSO || (pJoueur->mode == MODE_EDITEUR && level != -1))
    {
        while(strcmp(ligne, "#map\n") != 0)	//On avance dans le fichier tant qu'on a pas atteint le d�but de la map
        {
            fgets(ligne, 50, pFichierNiveau);
        }

        /* On parcourt la map � l'aide d'une double boucle */
        for (i=0; i< pMap->nbtiles_largeur_monde; i++)
        {
            for (j=0; j< pMap->nbtiles_hauteur_monde; j++)
            {
                /* On place le nombre lu dans la map */
                if (fgets(ligne, 50, pFichierNiveau) != NULL)
                {
                    pMap->plan[i][j] = strtol(ligne, NULL, 10);
                }
                else	//Si on a pas r�ussi � lire quelque chose c'est que le fichier est corrompu, il n'y a pas assez de bloc
                {
                    fprintf(pFichierErreur, "Erreur: level.lvl, fichier corrompu.\n");
                    *pEtatNiveau = CHARGEMENT_FICHIER_CORROMPU;	//On place un code d'erreur dans la variable
                    return NULL;	//On arr�te car le fichier est corrompu
                }
            }
        }

        /* On lit la ligne suivante, on v�rifie que c'est bien le d�but des bonus */
        fgets(ligne, 50, pFichierNiveau);

        if (strcmp(ligne, "#mapObjets\n") != 0)
        {
            fprintf(pFichierErreur, "Erreur: level.lvl, fichier corrompu.\n");
            *pEtatNiveau = CHARGEMENT_FICHIER_CORROMPU;	//On place un code d'erreur dans la variable
            return NULL;	//On arr�te car le fichier est corrompu
        }

        /* On parcourt la map des bonus avec notre double boucle */
        for (i=0; i< pMap->nbtiles_largeur_monde; i++)
        {
            for (j=0; j< pMap->nbtiles_hauteur_monde; j++)
            {
                /* On place les bonus dans la map */
                if (fgets(ligne, 50, pFichierNiveau) != NULL)
                {
                    pMap->planObjets[i][j] = strtol(ligne, NULL, 10);
                }
                else	//S'il manque des lignes le fichier est corrompu
                {
                    fprintf(pFichierErreur, "Erreur: level.lvl, fichier corrompu.\n");
                    *pEtatNiveau = CHARGEMENT_FICHIER_CORROMPU;	//On place un code d'erreur dans la variable
                    return NULL;	//On arr�te car le fichier est corrompu
                }
            }
        }

        fclose(pFichierNiveau);	//On ferme le fichier de niveau
    }
    else if(pJoueur->mode == MODE_EDITEUR && level == -1)	//Si on est en mode �diteur et qu'on ajoute un niveau
    {
        for (i=0; i< pMap->nbtiles_largeur_monde; i++)
        {
            for (j=0; j< pMap->nbtiles_hauteur_monde; j++)
            {
                pMap->plan[i][j] = VIDE;		//On fait une map vierge
                pMap->planObjets[i][j] = AUCUN_BONUS;
            }
        }

        sprintf(pMap->titre, "default");	//Titre par d�faut
    }

    fclose(pFichierErreur);	//On ferme le fichier d'erreur
    SDL_FreeSurface(pSurface);
    SDL_FreeSurface(pSurfaceFond);	//On d�truit les deux surfaces

    return pMap;	//On renvoie l'adresse de la structure Map que l'on a cr��e et charg�e
}

int DestructionMap(Map *pMap)
{
    int i;	//Compteur

    /* On lib�re la m�moire prise par les malloc de la map et par les textures*/

    SDL_DestroyTexture(pMap->tileset);
    SDL_DestroyTexture(pMap->fond);

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

void EntreesZero(ClavierSouris *pEntrees)
{
    int i=0;	//Compteur

    /* On initialise tous les champs � z�ro ou false */

    for(i=0; i< 200; i++)
    {
        pEntrees->clavier[i] = false;		//Clavier
    }

    for (i=0; i< 3; i++)
    {
        pEntrees->souris.touches[i] = false;	//Souris
    }

    pEntrees->souris.position.x = 0;	//Position souris
    pEntrees->souris.position.y = 0;

    pEntrees->souris.scroll = 0;	//Scroll de la molette

    pEntrees->fermeture = false;	//�v�nement de fermeture
}

int ChargementAnimations(SDL_Renderer *pMoteurRendu, Animation anim[])
{
    int i=0, j=0;			//Compteurs
    SDL_Surface *pSurface = NULL;	//Pointeur sur une surface
    char chaine[256];		//Cha�ne de caract�res

    for (i=0; i<200; i++)
    {
        sprintf(chaine, "ressources/anim/boum (%d).png", i+1);		//On �crit le nom de chaque fichier un par un
        pSurface = IMG_Load(chaine);	//On charge

        if (pSurface != NULL)
        {
            anim[ANIM_0].img[i] = SDL_CreateTextureFromSurface(pMoteurRendu, pSurface);		//On cr�e une texture tant qu'il y a des images � charger
        }
        else	//Sinon c'est que l'animation ne comporte pas plus d'image, on met tous les pointeurs suivants � NULL
        {
            anim[ANIM_0].img[i] = NULL;
        }

        SDL_FreeSurface(pSurface);	//On lib�re chaque surface
    }

    /* On met tous les autres pointeurs de texture de toutes les autres animations � NULL */
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
    /* Cette fonction permet d'afficher un message dans la fen�tre de la SDL, sur fond noir, il faut ensuite appuyer sur entr�e ou sur echap pour effacer le message, on peut savoir si on a appuy� sur l'une ou sur l'autre touche */

    Texte informations;	//Structure avec un tableau de cha�nes, un tableau de textures, un tableau de positions et une surface
    SDL_Color blancOpaque= {255, 255, 255, SDL_ALPHA_OPAQUE};
    ClavierSouris entrees;	//Structure pour les entr�es si jamais on n'en a pas fournie en param�tre

    if(pEntrees == NULL)
    {
        /* Pas de pointeur fourni en param�tre donc on initialise la structure locale et on met son adresse dans le pointeur pour ne pas avoir � modifier le reste de la fonction */
        EntreesZero(&entrees);
        pEntrees = &entrees;
    }

    sprintf(informations.chaines[0], messageInfos);	//On copie le message re�u en param�tre dans la premi�re cha�ne

    /* On cr�e une surface � partir de cette cha�ne et on enregistre la taille qu'elle va faire */
    informations.surface = TTF_RenderText_Blended(polices[POLICE_SNICKY_GRAND], informations.chaines[0], blancOpaque);
    TTF_SizeText(polices[POLICE_SNICKY_GRAND], informations.chaines[0], &informations.positions[0].w, &informations.positions[0].h);

    /* Si jamais la largeur du texte d�passe celle de la fen�tre, alors on la r�duit jusqu'� ce qu'elle rentre dedans */
    while(informations.positions[0].w > Largeur)
    {
        informations.positions[0].w -= 10;
    }

    /* On convertie la surface en texture */
    informations.pTextures[0] = SDL_CreateTextureFromSurface(pMoteurRendu, informations.surface);

    SDL_FreeSurface(informations.surface);	//On lib�re la surface

    /* On fait de m�me avec le message qui indique sur quelles touches il faut appuyer, et qui est toujours pr�sent */
    informations.surface = TTF_RenderText_Blended(polices[POLICE_SNICKY_MOYEN], "Appuyez sur ENTREE ou sur ECHAP", blancOpaque);
    TTF_SizeText(polices[POLICE_SNICKY_MOYEN], "Appuyez sur ENTREE ou sur ECHAP", &informations.positions[1].w, &informations.positions[1].h);

    informations.pTextures[1] = SDL_CreateTextureFromSurface(pMoteurRendu, informations.surface);

    SDL_FreeSurface(informations.surface);

    /* On centre le message principal */
    informations.positions[0].x = Arrondir(Largeur/2.0 - informations.positions[0].w/2.0);
    informations.positions[0].y = Arrondir(Hauteur/2.0 - informations.positions[0].h/2.0);

    /* On centre le message secondaire sous le principal */
    informations.positions[1].x = Arrondir(Largeur/2.0 - informations.positions[1].w/2.0);
    informations.positions[1].y = Arrondir(Hauteur/2.0 - informations.positions[1].h/2.0 + informations.positions[0].h);

    /* On affiche le message jusqu'� ce qu'il y ait appui sur une des deux touches */
    while (!(pEntrees->clavier[ENTREE] || pEntrees->clavier[ECHAP]))
    {
        SDL_SetRenderDrawColor(pMoteurRendu, 30, 30, 30, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(pMoteurRendu);

        SDL_RenderCopy(pMoteurRendu, informations.pTextures[0], NULL, &informations.positions[0]);
        SDL_RenderCopy(pMoteurRendu, informations.pTextures[1], NULL, &informations.positions[1]);

        SDL_RenderPresent(pMoteurRendu);

        GestionEvenements(pEntrees);
    }

    /* On lib�re la m�moire prise par les textures */
    SDL_DestroyTexture(informations.pTextures[0]);
    SDL_DestroyTexture(informations.pTextures[1]);

    /* On regarde sur quelle touche on a appuy� pour stopper le message et on revoie 'true' pour entr�e et 'false' pour echap */
    if (pEntrees->clavier[ENTREE])
    {
        pEntrees->clavier[ENTREE]=false;	//On remet la touche entr�e � false et on renvoie 'true'
        return true;
    }
    else	//Sinon c'est sur echap qu'on a appuy�
    {
        pEntrees->clavier[ECHAP]=false;	//On remet la touche echap � false et on renvoie 'false'
        return false;
    }
}

int VerificationMD5(char empreinte[], char nomFichier[])
{
    /* Cette fonction permet de comparer l'empreinte md5 d'un fichier avec une connue pour v�rifier qu'il n'a pas �t� corrompu par quelques personnes malveillantes (n'est-ce pas Julien) */

    FILE *pFichierATester = fopen(nomFichier, "rb");	//On ouvre le fichier en mode lecture binaire
    md5_state_t etat;	//Structure pour le calcul md5
    md5_byte_t md5Calcule[17]="";
    char md5CalculeHEXA[33]="";	//Cha�nes pour le calcul du md5 et pour la conversion en h�xad�cimal
    char *buffer= malloc(32768);	//On alloue un buffer de lecture de 32 768 octets (2^15)
    int i=0;	//Compteur
    size_t nbOctetsLus=0;	//Pour stocker le nombre d'octets lus � chaque tour de boucle ('size_t' est un typedef de 'unsigned int')

    if(pFichierATester == NULL)	//On v�rifie que l'ouverture du fichier � fonctionn�e
    {
        return -1;
    }

    md5_init(&etat);	//Initialisation du calculateur md5

    do
    {
        nbOctetsLus = fread(buffer, 1, 32768, pFichierATester);	//On lit 1*32768 octets dans le fichier que l'on place dans le buffer
        /* On ajoute le buffer au md5, on pr�cise la taille des donn�es pour le calcul (32 768 sauf au dernier tour de boucle) */
        md5_append(&etat, (md5_byte_t*)buffer, nbOctetsLus);
    }
    while(nbOctetsLus == 32768);	//Lorsqu'on a lu un nombre d'octet diff�rent de 32 768 c'est que c'�tait la derni�re partie du fichier

    md5_finish(&etat, md5Calcule);	//On termine le calcul md5
    md5Calcule[16] = '\0';	//On ajoute le caract�re de fin de cha�ne

    while (md5Calcule[i] != '\0')	//Tant qu'on est pas arriv� au bout
    {
        sprintf(md5CalculeHEXA+(2*i), "%02X", md5Calcule[i]);	//On transcrit chaque octet du message md5 en h�xa sur 2 chiffres
        i++;
    }

    if(strcmp(empreinte, md5CalculeHEXA) == 0)
    {
        return true;	//On renvoie 'true' si les deux empreintes ne sont pas identiques
    }

    return false;
}
//Fin du fichier IOmain.c
