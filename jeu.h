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
int Affichage(SDL_Renderer *pMoteurRendu, Sprite images[], TTF_Font *polices[], unsigned char descente[], Map* pMap, Animation anim[], unsigned char *ajoutAnim);
int AffichageBonus(SDL_Renderer *pMoteurRendu, Map *pMap, Sprite images[]);
int AffichageImages(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], unsigned char descente[], unsigned char *pAjoutAnim);
int AffichageMap(SDL_Renderer *pMoteurRendu, Map *pMap);
int AffichageTextes(SDL_Renderer *pMoteurRendu, TTF_Font *polices[], SDL_Texture *pTextureFond);
int AffichageVies(SDL_Renderer *pMoteurRendu, Sprite images[]);
int BouclePrincipale(Joueur *pJoueur, Sprite images[], Animation anim[], SDL_Renderer *pMoteurRendu, FMOD_SYSTEM *pMoteurSon, Sons *pSons, TTF_Font *polices[]);
void Chrono();
int DeplacementBoules(Sprite images[], Map *pMap, ClavierSouris *pEntrees, FMOD_SYSTEM *pMoteurSon, Sons *pSons);
int DeplacementMissiles(Sprite images[], unsigned char descente[]);
void DetectionBonus (Sprite images[], int indiceImage, Map* pMap);
int Gagne(SDL_Renderer *pMoteurRendu, Sprite images[], Map *pMap, TTF_Font *polices[]);
int GagneAffichage(SDL_Renderer *pMoteurRendu, Sprite images[], SDL_Texture *pTextureFondGagne, Map *pMap, Texte *information);
int Gravite(Sprite images[], Map *pMap, unsigned char sautEnCoursBleue, unsigned char sautEnCoursVerte);
int InitialisationPositions(Sprite images[], Joueur *pJoueur, int level);
int LectureAnimation(SDL_Renderer *pMoteurRendu, Animation anim[], int animNB);
int MiseAJourCoordonnees(ClavierSouris entrees, Sprite images[], unsigned char descente[], Map *pMap, FMOD_SYSTEM *pMoteurSon, Sons *pSons);
int Perdu(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], Map* pMap, TTF_Font *polices[], unsigned char *pAjoutAnim);
int PerduAffichage(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], SDL_Texture *pFondPerdu, Map *pMap, Texte *pInformation, unsigned char *pAjoutAnim);
int SautBleue(SDL_Rect *pPosition, unsigned char *pSautEnCours);
int Sauts(Sprite images[], Map *pMap, ClavierSouris *pEntrees, unsigned char *pSautEnCoursBleue, unsigned char *pSautEnCoursVerte);
int SautVerte(SDL_Rect *pPosition, unsigned char *pSautEnCours);
void TraitementBonus(FMOD_SYSTEM *pMoteurSon, Sons *pSons);
int TraitementEtatDuNiveau(SDL_Renderer *pMoteurRendu, FMOD_SYSTEM *pMoteurSon, Sons *pSons, Map **ppMap, Joueur *pJoueur, Sprite images[], TTF_Font *polices[], Animation anim[], ClavierSouris *pEntrees, unsigned char descente[], int *pEtat, unsigned char *pAjoutAnim, int *pControl);
char VerifierMortOUGagne(Sprite images[], Map *pMap, FMOD_SYSTEM *pMoteurSon, Sons *pSons);

#endif // JEU_H_INCLUDED

//Fin du fichier jeu.h
