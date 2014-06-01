/*
Projet-ISN

Fichier: interfaceMain.h

Contenu: Prototypes des fonctions contenues dans interfaceMain.c.

Actions: Permet à l'ordinateur de connaître toutes les fonctions présentes dans le programme ainsi que leurs arguments. Ici se trouvent les fonctions qui sont directement utilisées par main() et qui construisent l'interface du menu principal.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef DEF_INTERFACE_MAIN_H	//Protection contre les inclusions infinies
#define DEF_INTERFACE_MAIN_H

/* Prototypes des fonctions */
void AffectationVariableGlobale(Options *pOptions);
void AjoutElementsListe(GSList **pListeElements, FMOD_SYSTEM *pMoteurSon, Sons *pSons, Options *pOptions, Joueur *pJoueur);
void AjoutTimeOut(GtkWidget *pZoneDessin, GtkWidget *pZoneDessinV1, GtkWidget *pZoneDessinV2);
void ChargementAnimMAIN(FILE *pFichierErreur, GdkImages *pPix);
void CreerBoites(GtkWidget **pWindow, GtkWidget **pBoutonConnexion, GtkWidget **pBoutonJouer, GtkWidget **pBoutonOptions, GtkWidget **pBoutonEditeur, GtkWidget **pBoutonCredits, GtkWidget **pBoutonQuitter, GtkWidget **pZoneDessinV1, GtkWidget **pZoneDessinV2, GtkWidget **pTitre, GtkWidget **pZoneDessin);
void CreerBoutons(GtkWidget **pBoutonConnexion, GtkWidget **pBoutonJouer, GtkWidget **pBoutonCredits, GtkWidget **pBoutonOptions, GtkWidget **pBoutonEditeur, GtkWidget **pBoutonQuitter, GSList **pListeElements);
void CreerFenetre(GtkWidget **pWindow);
void CreerInterfaceMAIN(GSList **pListeElements, GtkWidget **pWindow, GtkWidget **pBoutonConnexion, GtkWidget **pBoutonJouer, GtkWidget **pBoutonCredits, GtkWidget **pBoutonOptions, GtkWidget **pBoutonEditeur, GtkWidget **pBoutonQuitter, GtkWidget **pTitre, GtkWidget **pZoneDessinV1, GtkWidget **pZoneDessinV2, GtkWidget **pZoneDessin, GdkImages *pix);
void CreerTitre(GtkWidget **pTitre);
void CreerZonesDessin(GtkWidget **pZoneDessin, GtkWidget **pZoneDessinV1, GtkWidget **pZoneDessinV2, GdkImages *pix);
void LiberationMemoireMain(GSList *pListeElements, Options *pOptions, FILE *pFichierErreur, Sons *pSons, FMOD_SYSTEM *pMoteurSon, GdkImages *pPix);

#endif

//Fin du fichier interfaceMain.h