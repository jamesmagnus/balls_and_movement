/*
Projet-ISN

Fichier: editeur.h

Contenu: Prototypes des fonctions contenues dans editeur.c

Actions: Permet à l'ordinateur de connaître toutes les fonctions présentes dans le programme ainsi que leurs arguments.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef EDITEUR_H_INCLUDED	//Protection contre les inclusions infinies
#define EDITEUR_H_INCLUDED

/* Prototypes des fonctions */
void AffichageBonusEditeur(SDL_Renderer *pMoteurRendu, Sprite images[]);
void AffichageBoxEditeur(SDL_Renderer *pMoteurRendu, ClavierSouris *pEntrees);
int AffichageEditeur(SDL_Renderer *pMoteurRendu, Sprite images[], Map* pMap, ClavierSouris entrees, int diaPris);
void AffichageObjetCurseurEditeur(SDL_Renderer *pMoteurRendu, ClavierSouris *pEntrees, Sprite images[], int objetPris);
void AmeliorationMap(Map *pMap);
void DeplacementObjetEditeur(FMOD_SYSTEM *pMoteurSon, Sons *pSons, Sprite images[], ClavierSouris *pEntrees);
int Editeur (SDL_Renderer *pMoteurRendu, Sprite images[], FMOD_SYSTEM *pMoteurSon, Sons *pSons, TTF_Font *polices[], Joueur *pJoueur, gpointer *pData);
Map* InitialisationEditeur (SDL_Renderer *pMoteurRendu, Sprite images[], TTF_Font *polices[], Joueur *pJoueur, int *etat);
int MiseAJourMap (Map *pMap, Sprite images[], ClavierSouris *pEntrees, FMOD_SYSTEM *pMoteurSon, Sons *pSons);
void MiseAjourMapEtBonusEditeur(ClavierSouris *pEntrees, FMOD_SYSTEM *pMoteurSon, Sons *pSons, Map *pMap, int *pDiaPris, int *pMissilePris);
void MiseAJourMapMissileEditeur(FMOD_SYSTEM *pMoteurSon, Sons *pSons, ClavierSouris *pEntrees, Sprite images[], int *pMissilePris);
int VerifierEmplacements(Sprite images[], Map *pMap);

#endif // EDITEUR_H_INCLUDED

//Fin du fichier editeur.h
