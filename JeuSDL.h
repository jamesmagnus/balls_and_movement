/*
Projet-ISN

Fichier: jeuSDL.h

Contenu: Prototypes des fonctions contenues dans jeuSDL.c

Actions: Permet à l'ordinateur de connaître toutes les fonctions présentes dans le programme ainsi que leurs arguments.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef JEU_SDL_H_INCLUDED		//Protection contre les inclusons infinies
#define JEU_SDL_H_INCLUDED

/* Prototypes des fonctions */
int LancerJeu(FMOD_SYSTEM *pMoteurSon, Sons *pSons, const char mode[], Joueur *pJoueur);
void LibererMemoire(SDL_Renderer *pMoteurRendu, sprite images[], Animation anim[], TTF_Font *polices[], SDL_Window *pFenetre, Options *pOptions);
int SauverMySql(Joueur *pJoueur);
void InitialiserInfos(Options *pOptions, Joueur *pJoueur);

#endif // JEU_SDL_H_INCLUDED

//Fin du fichier jeuSDL.h
