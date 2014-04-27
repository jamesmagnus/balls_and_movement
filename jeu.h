/*
Projet-ISN

Fichier: jeu.h

Contenu: Prototypes des fonctions contenues dans jeu.c

Actions: Permet à l'ordinateur de connaître toutes les fonctions présentes dans le programme ainsi que leurs arguments.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef JEU_H_INCLUDED		//Protection contre les inclusons infinies
#define JEU_H_INCLUDED

/* Prototypes des fonctions */
int MiseAJourCoordonnees(ClavierSouris entrees, sprite images[], unsigned char descente[], Map *pMap, FMOD_SYSTEM *pMoteurSon, Sons *pSons);
int DeplacementBoules(sprite images[], Map *pMap, ClavierSouris *pEntrees);
void CollisionDetect(sprite images[], unsigned char indiceImage, Map *pMap, Collision *pCollision);
unsigned int CollisionBordure (sprite images[], unsigned char indiceImage);
unsigned int CollisionImage (sprite images[], unsigned char indiceImage, Collision *pCollision);
unsigned int CollisionDecor (sprite images[], unsigned char indiceImage, Map* pMap);
void CollisionBonus (sprite images[], unsigned char indiceImage, Map* pMap);
int Affichage(SDL_Renderer *pMoteurRendu, sprite images[], TTF_Font *polices[], unsigned char descente[], Map* pMap, Animation anim[], int *ajoutAnim);
int AffichageVies(SDL_Renderer *pMoteurRendu, sprite images[]);
int AffichageMap(SDL_Renderer *pMoteurRendu, Map *pMap);
int AffichageBonus(SDL_Renderer *pMoteurRendu, Map *pMap, sprite images[]);
int AffichageImages(SDL_Renderer *pMoteurRendu, sprite images[], Animation anim[], unsigned char descente[], int *pAjoutAnim);
int AffichageTextes(SDL_Renderer *pMoteurRendu, TTF_Font *polices[], SDL_Texture *pTextureFond);
int InitialisationPositions(sprite images[], char mode[], int level);
int SautBleue(SDL_Rect *pPosition, unsigned char *pSautEnCours);
int SautVerte(SDL_Rect *pPosition, unsigned char *pSautEnCours);
int Boucle_principale(Joueur *pJoueur, sprite images[], Animation anim[], SDL_Renderer *pMoteurRendu, FMOD_SYSTEM *pMoteurSon, Sons *pSons, TTF_Font *polices[]);
int Arrondir(double nombre);
int Perdu(SDL_Renderer *pMoteurRendu, sprite images[], Animation anim[], Map* pMap, TTF_Font *polices[]);
int PerduAffichage(SDL_Renderer *pMoteurRendu, sprite images[], Animation anim[], SDL_Texture *pFondPerdu, Map *pMap, texte *pInformation);
int LectureAnimation(SDL_Renderer *pMoteurRendu, Animation anim[], int animNB);
int VerifierMortOUGagne(sprite images[], Map *pMap, FMOD_SYSTEM *pMoteurSon, Sons *pSons);
int Gagne(SDL_Renderer *pMoteurRendu, sprite images[], Map *pMap, TTF_Font *polices[]);
int GagneAffichage(SDL_Renderer *pMoteurRendu, sprite images[], SDL_Texture *pTextureFondGagne, Map *pMap, texte *information);

#endif // JEU_H_INCLUDED

//Fin du fichier jeu.h
