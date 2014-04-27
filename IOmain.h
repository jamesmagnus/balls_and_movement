/*
Projet-ISN

Fichier: IOmain.h

Contenu: Prototypes des fonctions contenues dans IOmain.c

Actions: Permet à l'ordinateur de connaître toutes les fonctions présentes dans le programme ainsi que leurs arguments.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef IOMAIN_H_INCLUDED
#define IOMAIN_H_INCLUDED	//Protection contre les inclusons infinies

/* Structures */
typedef struct Souris       //Structure pour gérer les événements de la souris
{
	char touches[3];
	SDL_Rect position;
	int scroll;
} Souris;

typedef struct ClavierSouris        //Structure pour gérer les événements du clavier
{
	int fermeture;
	char clavier[200];
	Souris souris;
} ClavierSouris;

typedef struct texte        //Structure pour afficher du texte avec la SDL
{
	SDL_Texture *pTextures[100];
	SDL_Surface *surface;
	SDL_Rect positions[100];
	char chaines[100][200];
} texte;

typedef struct sprite           //Structure pour les images à afficher
{
	SDL_Texture *pTextures[20];
	SDL_Rect position[20];
} sprite;

typedef struct TileProp		//Structure pour les propriétés des tiles du décor
{
	SDL_Rect src;
} TileProp;

typedef struct Map		//Structure pour la map d'un niveau
{
	int LARGEUR_TILE, HAUTEUR_TILE;
	int nbtilesX,nbtilesY;
	SDL_Texture *tileset, *fond;
	TileProp *props;
	int nbtiles_largeur_monde, nbtiles_hauteur_monde;
	int **plan;
	int **planObjets;
} Map;

typedef struct Sons		//Structure pour les sons du jeu
{
	FMOD_SOUND *music[10];
	FMOD_SOUND *bruits[20];
} Sons;

typedef struct Animation		//Structure pour les images qui forment une animation
{
	SDL_Texture *img[200];
	SDL_Rect pos;
	int animEnCours;
} Animation;

typedef struct Collision		//Structure pour gérer les collisions
{
	unsigned int etatColl;
	unsigned char numMissile;
} Collision;

/* Prototypes des fonctions */
int Initialisation(SDL_Renderer **ppMoteurRendu, FILE *pFichierErreur, SDL_Window **ppFenetre, Options *pOptions);
int Chargement (sprite images[], SDL_Renderer *pMoteurRendu, TTF_Font *polices[], Animation anim[]);
int GestionEvenements(ClavierSouris *entrees);
int ChargementTextures(SDL_Renderer *pMoteurRendu, sprite images[]);
int DestructionSurfaces(SDL_Surface *sImages[]);
int ChargementPolices(TTF_Font *polices[]);
int ChargementMusic (Sons *pSons, FMOD_SYSTEM *pMoteurSon);
int InitialisationSon(FMOD_SYSTEM **ppMoteurSon, FILE *pFichierErreur, Sons *pSons);
Map* ChargementNiveau(SDL_Renderer *pMoteurRendu, char mode[], int level);
int DestructionMap(Map *pMap);
int EntreesZero(ClavierSouris *pEntrees);
int ChargementAnimation(SDL_Renderer *pMoteurRendu, Animation anim[]);
int MessageInformations(const char messageInfos[], TTF_Font *polices[], SDL_Renderer *pMoteurRendu, ClavierSouris *pEntrees);

#endif // IOMAIN_H_INCLUDED

//Fin du fichier IOmain.h
