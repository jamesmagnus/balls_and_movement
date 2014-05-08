/*
Projet-ISN

Fichier: main.h

Contenu: Prototypes des fonctions contenues dans main.c, les structures et les flags.

Actions: Permet à l'ordinateur de connaître toutes les fonctions présentes dans le programme ainsi que leurs arguments.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef DEF_MAIN_H	//Protection contre les inclusions infinies
#define DEF_MAIN_H

/* Définitions */
#define true 1
#define false 0
#define T_FPS 15
#define T_ANIM 35
#define _G_ 9.80665
#define MODE_CAMPAGNE 1
#define MODE_PERSO 2
#define MODE_EDITEUR 3

/* Flags pour les bonus */
#define AUCUN_BONUS 0x000
#define BONUS_SCORE_FAIBLE 0x001
#define BONUS_SCORE_FORT 0x002
#define BONUS_VIE 0x004
#define BONUS_SAUT_VERTE_FAIBLE 0x008
#define BONUS_SAUT_VERTE_FORT 0x010
#define BONUS_SAUT_BLEUE_FAIBLE 0x020
#define BONUS_SAUT_BLEUE_FORT 0x040
#define BONUS_VITESSE_BLEUE_FAIBLE 0x080
#define BONUS_VITESSE_BLEUE_FORT 0x100
#define BONUS_VITESSE_VERTE_FAIBLE 0x200
#define BONUS_VITESSE_VERTE_FORT 0x400

/* Flags pour les collisions */
#define COLL_NONE 0x000
#define COLL_VORTEX_BLEU 0x001
#define COLL_VORTEX_VERT 0x002
#define COLL_BORD_HAUT 0x004
#define COLL_BORD_BAS 0x008
#define COLL_BORD_GAUCHE 0x010
#define COLL_BORD_DROIT 0x020
#define COLL_BOULE_BLEUE 0x040
#define COLL_BOULE_VERTE 0x080
#define COLL_BOULE_MAGENTA 0x100
#define COLL_DECOR 0x200
#define COLL_MISSILE 0x400

/* Structures */
typedef struct Coordo	//Coordonnées pour les animations du menu
{
	double x1, x2, x3, x4;
	double y1, y2, y3, y4;
	char sens1, sens2, sens3, sens4;
} Coordo;

typedef struct GdkImages	//Structure qui contient les 4 pixbufs, c'est à dire les quatres images des animations du menu
{
	GdkPixbuf *img[4];
} GdkImages;

/* Enumérations pour ne pas avoir à retenir les indices des tableaux */
enum {BOULE_BLEUE, BOULE_MAGENTA, BOULE_VERTE, MISSILE, VORTEX_BLEU, VORTEX_VERT, GEMMES, CURSEUR, VIE, FOND_TEXTES, AJOUTER_MISSILE_H, AJOUTER_MISSILE_V};

enum{DIA_VERTCLAIR=1, DIA_NOIR, DIA_TURQUOISE, DIA_JAUNE, DIA_ROSE, DIA_ROUGE, DIA_ORANGE, DIA_BLEUCLAIR, DIA_BLEUMARINE, DIA_VERT, DIA_MARRON, DIA_BLEU, DIA_ROSECLAIR, DIA_VIOLET, DIA_FUSHIA, DIA_MARRONCLAIR, DIA_GRIS, DIA_BLEUFONCE};

enum {SOL_PETITE_PENTE_D, SOL_GRANDE_PENTE_D, VIDE, SOL_GRANDE_PENTE_G, SOL_PETITE_PENTE_G, SOL_COIN_D_1, SOL_COIN_D_2, SOL_NORMAL, SOL_COIN_G_1, SOL_COIN_G_2, SOL_PLEIN_1, SOL_PLEIN_2, SOL_PLEIN_3, SOL_PLEIN_4, SOL_PLEIN_5, SOL_FIN_G, SOL_FIN_D, SOL_PLEIN_UNI};

enum {POLICE_ARIAL, POLICE_ARIAL_MOYEN, POLICE_ARIAL_GRAND, POLICE_SNICKY, POLICE_SNICKY_MOYEN, POLICE_SNICKY_GRAND};

enum {HAUT, BAS, GAUCHE, DROITE, Q, D, S, ECHAP, ESPACE, ENTREE, SHIFT, J, L, F5};

enum {C_GAUCHE, C_DROIT, C_MOLETTE};

enum {ANIM_0, ANIM_1, ANIM_2, ANIM_3, ANIM_4};

enum {M_MENU, M_JEU, M_PERDU, M_CREDITS, M_GAGNE, M_LOAD};

enum {S_BOULE_BOUM, S_BOULE_BOULE, S_SORTIE, S_BONUS, S_CLICK, S_SAVE, S_ALARME};

enum {CHARGEMENT_ERREUR=-1, CHARGEMENT_REUSSI=0, CHARGEMENT_GAGNE=1, CHARGEMENT_FICHIER_CORROMPU=2};

enum {JEU_FIN_ERREUR_CHARGEMENT=-1, JEU_FIN=0, JEU_EN_COURS=1};

enum {MORT_BORDURE=-3, GAGNE=-2, RIEN=-1};

/* Prototypes des fonctions */
void CreerBoutons(GtkWidget **pBoutonConnexion, GtkWidget **pBoutonJouer, GtkWidget **pBoutonCredits, GtkWidget **pBoutonOptions, GtkWidget **pBoutonEditeur, GtkWidget **pBoutonQuitter, GSList *pListeElements);
void CreerFenetre(GtkWidget **pWindow);
void CreerTitre(GtkWidget **pTitre);
void CreerZonesDessin(GtkWidget **pZoneDessin, GtkWidget **pZoneDessinV1, GtkWidget **pZoneDessinV2, GdkImages *pix);
void CreerBoites(GtkWidget **pWindow, GtkWidget **pBoutonConnexion, GtkWidget **pBoutonJouer, GtkWidget **pBoutonOptions, GtkWidget **pBoutonEditeur, GtkWidget **pBoutonCredits, GtkWidget **pBoutonQuitter, GtkWidget **pZoneDessinV1, GtkWidget **pZoneDessinV2, GtkWidget **pTitre, GtkWidget **pZoneDessin);
void InitialiserJoueur(Joueur *pJoueur);
void LiberationMemoireMain(GSList *pListeElements, Options *pOptions, FILE *pFichierErreur, Sons *pSons, FMOD_SYSTEM *pMoteurSon, GdkImages *pPix);

#endif //DEF_MAIN_H

//Fin du fichier main.h
