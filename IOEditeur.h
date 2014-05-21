/*
Projet-ISN

Fichier: IOediteur.h

Contenu: Prototypes des fonctions contenues dans IOediteur.c

Actions: Permet à l'ordinateur de connaître toutes les fonctions présentes dans le programme ainsi que leurs arguments.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef IOEDITEUR_H_INCLUDED
#define IOEDITEUR_H_INCLUDED	//Protection contre les inclusons infinies

/* Prototypes des fonctions */
gboolean DemandeNomNiveau(gpointer *pData);
void RenvoiContenuEntry(GtkWidget *pWidget, gpointer *pData);
int SauvegardeNiveau(Map* pMap, Sprite images[], Joueur *pJoueur);

#endif // IOEDITEUR_H_INCLUDED

//Fin du fichier IOediteur.h
