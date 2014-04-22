/*
Projet-ISN

Fichier: editeur.h

Contenu: Prototypes des fonctions contenues dans editeur.c

Actions: Permet à l'ordinateur de connaître toutes les fonctions présentes dans le programme ainsi que leurs arguments.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef EDITEUR_H_INCLUDED
#define EDITEUR_H_INCLUDED

/* Prototypes des fonctions */
Map* InitialisationEditeur (SDL_Renderer *pMoteurRendu, sprite images[], TTF_Font *polices[]);
int AffichageEditeur(SDL_Renderer *pMoteurRendu, sprite images[], Map* pMap, ClavierSouris entrees, int diaPris);
int Editeur (SDL_Renderer *pMoteurRendu, sprite images[], FMOD_SYSTEM *pMoteurSon, Sons *pSons, TTF_Font *polices[]);
int MiseAJourMap (Map *pMap, sprite images[], ClavierSouris *pEntrees, FMOD_SYSTEM *pMoteurSon, Sons *pSons);
int VerifierEmplacements(sprite images[], Map *pMap);
void DeplacementObjetEditeur(FMOD_SYSTEM *pMoteurSon, Sons *pSons, sprite images[], ClavierSouris *pEntrees);
void MiseAjourMapEtBonusEditeur(ClavierSouris *pEntrees, FMOD_SYSTEM *pMoteurSon, Sons *pSons, Map *pMap, int *pDiaPris, int *pMissilePris);
void MiseAJourMapMissileEditeur(FMOD_SYSTEM *pMoteurSon, Sons *pSons, ClavierSouris *pEntrees, sprite images[], int *pMissilePris);
void AmeliorationMap(Map *pMap);
void AffichageBonusEditeur(SDL_Renderer *pMoteurRendu, sprite images[]);
void AffichageBoxEditeur(SDL_Renderer *pMoteurRendu, ClavierSouris *pEntrees);
void AffichageObjetCurseurEditeur(SDL_Renderer *pMoteurRendu, ClavierSouris *pEntrees, sprite images[], int objetPris);

#endif // EDITEUR_H_INCLUDED

//Fin du fichier editeur.h
