/*
Projet-ISN

Fichier: callback.c

Contenu: Fonctions de callback pour l'interface du menu

Actions: C'est ici que se trouve les fonctions pour l'interface du menu, les actions lorsque l'on clique sur un bouton.

Biblioth�ques utilis�es: Biblioth�ques standards, SDL, SDL_image, SDL_ttf, FMOD, GTK

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <md5.h>
#include <winsock.h>
#include <mysql.h>
#include <gtk\gtk.h>
#include <time.h>
#include <SDL.h>
#include <math.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <fmod.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "callback.h"
#include "jeu.h"
#include "JeuSDL.h"
#include "IOcredits.h"

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;		//Lien vers les variables globales d�clar�es dans main.c
extern double Volume, Largeur, Hauteur;

/* D�claration et affection d'une variable globale au fichier */
Coordo coordonnees =
{
	100, 400, 800, 750,	//x
	0, 50, 120, 2000,	//y
	false, true, false, false	//Sens: mont�e ou descente
};


int Redessiner(gpointer pData)
{
	gtk_widget_queue_draw(pData);	//On demande � redessiner le widget

	return true;	//On retourne 'true' pour que la r�p�tition de la fonction continue
}


void DestructionFenetre(GtkWidget *pWidget, gpointer pData)
{
	GtkWidget *pFenetreDemande, *pBoutonOUI, *pBoutonNON;
	GdkRGBA couleurFond= {0.610, 0.805, 0.920, 1}, couleurBoutons= {0.650, 0.850, 0.925, 1}, couleurBoutonsEnfonce= {0.550, 0.655, 7.000, 1};

	pFenetreDemande = gtk_dialog_new();		//On cr�e une fen�tre de dialogue

	/* On d�finit quelques caract�ristiques de cette fen�tre */
	gtk_window_set_position(GTK_WINDOW(pFenetreDemande), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(pFenetreDemande), "Vraiment ?");
	gtk_window_set_icon_from_file(GTK_WINDOW(pFenetreDemande), "ressources/img/z.png", NULL);
	gtk_window_set_resizable(GTK_WINDOW(pFenetreDemande), false);
	gtk_widget_override_background_color(pFenetreDemande, GTK_STATE_FLAG_NORMAL, &couleurFond);

	/* On cr�e les deux boutons et on les associe avec une r�ponse */
	pBoutonNON = gtk_button_new_with_label("Non");
	gtk_widget_override_background_color(pBoutonNON, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pBoutonNON, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);

	pBoutonOUI = gtk_button_new_with_label("Oui");
	gtk_widget_override_background_color(pBoutonOUI, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pBoutonOUI, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);

	gtk_dialog_add_action_widget(GTK_DIALOG(pFenetreDemande), pBoutonNON, GTK_RESPONSE_NO);
	gtk_dialog_add_action_widget(GTK_DIALOG(pFenetreDemande), pBoutonOUI, GTK_RESPONSE_YES);

	gtk_widget_show_all(pFenetreDemande);	//On affiche la fen�tre


	/* On lance le dialogue et on regarde la r�ponse dans un switch */
	switch(gtk_dialog_run(GTK_DIALOG(pFenetreDemande)))
	{
	case GTK_RESPONSE_YES:
		{
			/* Arr�t de la boucle �v�nementielle, le programme se ferme */
			gtk_main_quit();
			break;
		}

	case GTK_RESPONSE_NO:
		{
			/* On ne fait rien */
			break;
		}
	}

	gtk_widget_destroy(pFenetreDemande);	// On d�truit la fen�tre avant de sortir de la fonction
}

void DemandeModeJeu(GtkWidget *pWidget, gpointer pData)
{
	GtkWidget *pWindow, *pBoutonCampagne, *pBoutonPerso, *pLabelBoutonCampagne, *pLabelBoutonPerso, *pHBox;
	GdkRGBA couleurFond= {0.610, 0.805, 0.920, 1}, couleurBoutons= {0.650, 0.850, 0.925, 1}, couleurBoutonsEnfonce= {0.550, 0.655, 7.000, 1};
	Joueur *pJoueur = (Joueur *)g_slist_nth_data((GSList*)pData, 6);	//On r�cup�re le pointeur vers la structure Joueur dans la liste cha�n�e

	/* On cr�e la fen�tre et on d�finit quelques caract�ristiques */
	pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(pWindow), GTK_WIN_POS_CENTER);
	gtk_window_set_icon_from_file(GTK_WINDOW(pWindow), "ressources/img/z.png", NULL);
	gtk_window_set_title(GTK_WINDOW(pWindow), "Mode de jeu");
	gtk_widget_set_size_request(pWindow, 600, 100);
	gtk_window_set_resizable(GTK_WINDOW(pWindow), false);
	gtk_widget_override_background_color(pWindow, GTK_STATE_FLAG_NORMAL, &couleurFond);

	/* On cr�e le bouton Campagne */
	pBoutonCampagne = gtk_button_new();
	gtk_widget_set_margin_left(pBoutonCampagne, 50);
	gtk_widget_set_margin_top(pBoutonCampagne, 30);
	gtk_widget_set_margin_bottom(pBoutonCampagne, 30);
	g_signal_connect(G_OBJECT(pBoutonCampagne), "clicked", G_CALLBACK(LancerJeuModeCampagne), pData);//On connecte avec la fonction correspondante

	/* Si le joueur n'est pas connect� on d�sactive la campagne */
	if(pJoueur->connexion != 1)
	{
		gtk_widget_set_sensitive(pBoutonCampagne, false);
	}

	/* On cr�e le bouton Niveaux Perso */
	pBoutonPerso = gtk_button_new();
	gtk_widget_set_margin_right(pBoutonPerso, 50);
	gtk_widget_set_margin_top(pBoutonPerso, 30);
	gtk_widget_set_margin_bottom(pBoutonPerso, 30);
	g_signal_connect(G_OBJECT(pBoutonPerso), "clicked", G_CALLBACK(LancerJeuModePerso), pData);	//On connecte avec la fonction correspondante

	/* On cr�e le texte du bouton Campagne */
	pLabelBoutonCampagne = gtk_label_new("<span size=\"15000\"><b>Campagne</b></span>");
	gtk_label_set_use_markup(GTK_LABEL(pLabelBoutonCampagne), true);
	gtk_label_set_justify(GTK_LABEL(pLabelBoutonCampagne), GTK_JUSTIFY_CENTER);
	gtk_widget_override_background_color(pLabelBoutonCampagne, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pLabelBoutonCampagne, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
	gtk_widget_set_size_request(pLabelBoutonCampagne, 180, 40);

	/* On cr�e le texte du bouton Niveaux Perso */
	pLabelBoutonPerso = gtk_label_new("<span size=\"15000\"><b>Niveaux Perso</b></span>");
	gtk_label_set_use_markup(GTK_LABEL(pLabelBoutonPerso), true);
	gtk_label_set_justify(GTK_LABEL(pLabelBoutonPerso), GTK_JUSTIFY_CENTER);
	gtk_widget_override_background_color(pLabelBoutonPerso, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pLabelBoutonPerso, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
	gtk_widget_set_size_request(pLabelBoutonPerso, 180, 40);

	/* On ajoute les textes dans les boutons */
	gtk_container_add(GTK_CONTAINER(pBoutonCampagne), pLabelBoutonCampagne);
	gtk_container_add(GTK_CONTAINER(pBoutonPerso), pLabelBoutonPerso);

	/* On cr�e la bo�te horizontale et on y met les 2 boutons */
	pHBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(pHBox), pBoutonCampagne, false, false, 0);
	gtk_box_pack_end(GTK_BOX(pHBox), pBoutonPerso, false, false, 0);

	/* On met la bo�te dans la fen�tre */
	gtk_container_add(GTK_CONTAINER(pWindow), pHBox);

	gtk_widget_show_all(pWindow);	//On affiche la fen�tre
}


void LancerJeuModeCampagne(GtkWidget *pWidget, gpointer pData)
{
	Joueur *pJoueur = (Joueur *)g_slist_nth_data((GSList*)pData, 6);	//On r�cup�re le pointeur vers la structure Joueur dans la liste cha�n�e
	Sons *pSons = (Sons*)g_slist_nth_data((GSList*)pData, 4);	//De m�me avec celui vers la structure Sons
	FMOD_SYSTEM *pMoteurSon = (FMOD_SYSTEM *)g_slist_nth_data((GSList*)pData, 3);	//De m�me avec celui vers la structure FMOD_SYSTEM

	pJoueur->mode = MODE_CAMPAGNE;	//On d�finit le mode de jeu qui va �tre utilis�

	gtk_widget_destroy(gtk_widget_get_toplevel(pWidget));	//On d�truit la fen�tre de choix de mode de jeu

	/* On lance le jeu */
	gtk_widget_hide(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));	//On cache la fen�tre du menu
	LancerJeu(pMoteurSon, pSons, pJoueur);
	gtk_widget_show_all(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));	//On affiche � nouveau la fen�tre du menu
}


void LancerJeuModePerso(GtkWidget *pWidget, gpointer pData)
{
	Joueur *pJoueur = (Joueur *)g_slist_nth_data((GSList*)pData, 6);	//On r�cup�re le pointeur vers la structure Joueur dans la liste cha�n�e
	Sons *pSons = (Sons*)g_slist_nth_data((GSList*)pData, 4);
	FMOD_SYSTEM *pMoteurSon = (FMOD_SYSTEM *)g_slist_nth_data((GSList*)pData, 3);

	pJoueur->mode = MODE_PERSO;		//On d�finit le mode de jeu qui va �tre utilis�

	gtk_widget_destroy(gtk_widget_get_toplevel(pWidget));	//On d�truit la fen�tre de choix de mode de jeu

	/* On lance le jeu */
	gtk_widget_hide(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));	//On cache la fen�tre du menu
	LancerJeu(pMoteurSon, pSons, pJoueur);
	gtk_widget_show_all(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));	//On affiche � nouveau la fen�tre du menu
}


void LancerEditeur(GtkWidget *pWidget, gpointer pData)
{
	Joueur *pJoueur = (Joueur *)g_slist_nth_data((GSList*)pData, 6);	//On r�cup�re le pointeur vers la structure Joueur dans la liste cha�n�e
	Sons *pSons = (Sons*)g_slist_nth_data((GSList*)pData, 4);		//De m�me avec celui vers la structure Sons
	FMOD_SYSTEM *pMoteurSon = (FMOD_SYSTEM *)g_slist_nth_data((GSList*)pData, 3);	//De m�me avec celui vers la structure FMOD_SYSTEM

	pJoueur->mode = MODE_EDITEUR;	//On d�finit le mode de jeu qui va �tre utilis�

	/* On lance l'�diteur */
	gtk_widget_hide(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));	//On cache la fen�tre du menu
	LancerJeu(pMoteurSon, pSons, pJoueur);
	gtk_widget_show_all(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));	//On affiche � nouveau la fen�tre du menu
}


void LancementCredits(GtkWidget *pWidget, gpointer pData)
{
	GtkWidget *lignesCredits[100] = {NULL};	//Tableau de 100 pointeurs pour chaque ligne de cr�dit
	GtkWidget *pVBox=NULL, *pWindow=NULL;
	FMOD_CHANNEL *pChannelEnCours=NULL;
	GdkRGBA couleurFond= {0.610, 0.805, 0.920, 1};	//Couleur du fond de la fen�tre
	char credits[50][100];	//Tableau de 50 cha�nes de caract�res
	int nbLignes = ChargementCredits(credits), i=0;

	/* On arr�te la musique du menu */
	FMOD_System_GetChannel((FMOD_SYSTEM *)g_slist_nth_data(pData, 3), M_MENU, &pChannelEnCours);
	FMOD_Channel_SetPaused(pChannelEnCours, true);

	FMOD_Sound_SetLoopCount(((Sons *)g_slist_nth_data(pData, 4))->music[M_CREDITS], -1);      // On active la lecture en boucle

	/* On lit la musique des cr�dits */
	FMOD_System_PlaySound((FMOD_SYSTEM *)g_slist_nth_data(pData, 3), M_CREDITS, ((Sons *)g_slist_nth_data(pData, 4))->music[M_CREDITS], true, NULL);
	FMOD_System_GetChannel((FMOD_SYSTEM *)g_slist_nth_data(pData, 3), M_CREDITS, &pChannelEnCours);
	FMOD_Channel_SetVolume(pChannelEnCours, (float)(((Options*)g_slist_nth_data(pData, 5))->volume/100.0));
	FMOD_Channel_SetPaused(pChannelEnCours, false);

	/* On cr�e le titre Cr�dits */
	lignesCredits[0] = gtk_label_new(g_locale_to_utf8("<span underline=\"single\" font-family=\"Snickles\" size=\"55000\">Cr�dits</span>", -1, NULL, NULL, NULL));

	gtk_label_set_use_markup(GTK_LABEL(lignesCredits[0]), true);

	/* On cr�e toutes les lignes de cr�dits et on leur donne une marge au dessus pour que la fen�tre soit remplie */
	for (i=1; i<nbLignes; i++)
	{
		lignesCredits[i] = gtk_label_new(g_markup_printf_escaped("<span font-family=\"Snickles\" size=\"20000\">%s</span>", g_locale_to_utf8(credits[i], -1, NULL, NULL, NULL)));
		gtk_label_set_use_markup(GTK_LABEL(lignesCredits[i]), true);
		gtk_widget_set_margin_top(lignesCredits[i], 200/nbLignes);
	}

	pVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);	//Bo�te pour contenir toutes les lignes

	/* On cr�e la fen�tre des cr�dits et on la connecte � la fonction qui la ferme quand on clique sur la croix */
	pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(pWindow), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_widget_set_size_request(pWindow, 800, 600);
	gtk_window_set_icon_from_file(GTK_WINDOW(pWindow), "ressources/img/z.png", NULL);
	gtk_widget_override_background_color(pWindow, GTK_STATE_FLAG_NORMAL, &couleurFond);
	g_signal_connect(pWindow, "delete-event", G_CALLBACK(FermerCredit), pData);

	/* Un peu de marge au-dessus et en-dessous et on met le titre dans la bo�te */
	g_object_set(lignesCredits[0], "margin-top", 10, NULL);
	g_object_set(lignesCredits[0], "margin-bottom", 10, NULL);
	gtk_box_pack_start(GTK_BOX(pVBox), lignesCredits[0], false, false, 0);

	/* On met les lignes dans la bo�te */
	for (i=1; i<nbLignes; i++)
	{
		gtk_box_pack_start(GTK_BOX(pVBox), lignesCredits[i], false, false, 0);
	}

	/* On ajoute la bo�te � la fen�tre */
	gtk_container_add(GTK_CONTAINER(pWindow), pVBox);

	/* On masque la fen�tre du menu et on affiche celle des cr�dits */
	gtk_widget_hide(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));

	gtk_widget_show_all(pWindow);
}


void LancementOptions(GtkWidget *pWidget, gpointer pData)
{
	GtkWidget *pWindow, *pHbox1, *pHbox2, *pVbox1, *pVbox2, *pVbox3, *pTitre, *pCase1, *pCase2, *pCase3, *pBoutonValider, *pBoutonAnnuler, *pListeResolution, *pCurseur1, *pCurseur2, *pResolution, *pNBVies, *pVolume, *pSeparateur;
	char options[50][50] = {{""}};
	Options *pOptions = NULL;
	GdkRGBA couleurFond= {0.610, 0.805, 0.920, 1};

	/* On lit les options dans le fichier et on remplit la structure Options avec */
	LectureOptions(options);
	pOptions = DecouperOptions(options);

	/* On cr�e la fen�tre, on lui donne ses caract�ristiques et on la connecte avec la fonction d'affichage de la fen�tre du menu quand elle est d�truite */
	pWindow = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_title(GTK_WINDOW(pWindow), "Options");
	gtk_window_set_position(GTK_WINDOW(pWindow), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_widget_set_size_request(pWindow, 800, 600);
	gtk_widget_override_background_color(pWindow, GTK_STATE_FLAG_NORMAL, &couleurFond);
	g_signal_connect(pWindow, "destroy", G_CALLBACK(AfficherMenu), pData);

	/* On cr�e le titre */
	pTitre = gtk_label_new("<span underline=\"single\" font-family=\"Snickles\" size=\"46080\">Options</span>");
	gtk_label_set_use_markup(GTK_LABEL(pTitre), true);
	gtk_widget_set_margin_bottom(pTitre, 40);
	gtk_widget_set_margin_top(pTitre, 20);

	/* On cr�e les labels pour les diff�rentes options */
	pResolution = gtk_label_new(g_locale_to_utf8("<span underline=\"single\">R�solution:</span>", -1, NULL, NULL, NULL));
	gtk_label_set_use_markup(GTK_LABEL(pResolution), true);
	gtk_widget_set_margin_bottom(pResolution, 10);
	pNBVies = gtk_label_new("<span underline=\"single\">Vies:</span>");
	gtk_label_set_use_markup(GTK_LABEL(pNBVies), true);
	pVolume = gtk_label_new("<span underline=\"single\">Volume:</span>");
	gtk_label_set_use_markup(GTK_LABEL(pVolume), true);

	/* On cr�e un s�parateur pour s�parer la fen�tre en deux */
	pSeparateur = gtk_frame_new(NULL);
	gtk_widget_set_margin_left(pSeparateur, 70);
	gtk_widget_set_margin_right(pSeparateur, 70);

	/* On cr�e la premi�re bo�te horizontale qui va contenir 3 cases � cocher */
	pHbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_margin_top(pHbox1, 20);

	/* Les 3 cases en question, une fonction est appel�e pour mettre � jour la structure Options quand on clique dessus */
	pCase1 = gtk_check_button_new_with_label("Musique");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pCase1), pOptions->musique);
	gtk_widget_set_margin_left(pCase1, 160);
	g_signal_connect(pCase1, "toggled", G_CALLBACK(ModifierOptionsToggleButton1), pOptions);

	pCase2 = gtk_check_button_new_with_label("Son");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pCase2), pOptions->sons);
	gtk_widget_set_margin_right(pCase2, 160);
	g_signal_connect(pCase2, "toggled", G_CALLBACK(ModifierOptionsToggleButton2), pOptions);

	pCase3 = gtk_check_button_new_with_label("Plein Ecran");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pCase3), pOptions->fullScreen);
	gtk_widget_set_margin_left(pCase3, 50);
	g_signal_connect(pCase3, "toggled", G_CALLBACK(ModifierOptionsToggleButton3), pOptions);

	/* On ajoute ces 3 cases � cocher dans la bo�te H1*/
	gtk_box_pack_start(GTK_BOX(pHbox1), pCase1, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pHbox1), pCase3, false, false, 0);
	gtk_box_pack_end(GTK_BOX(pHbox1), pCase2, false, false, 0);

	/* On cr�e la premi�re bo�te verticale qui va contenir les curseurs de volume et de vie ainsi que la liste d�roulante des r�solutions */
	pVbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_margin_top(pVbox1, 20);

	/* La liste d�roulante des r�solutions avec ses marges et ses choix */
	pListeResolution = gtk_combo_box_text_new();
	gtk_widget_set_margin_left(pListeResolution, 250);
	gtk_widget_set_margin_right(pListeResolution, 250);
	gtk_widget_set_margin_bottom(pListeResolution, 40);

	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pListeResolution), "800 x 500");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pListeResolution), "1280 x 800");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pListeResolution), "1440 x 900");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pListeResolution), "1680 x 1050");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pListeResolution), "1920 x 1200");
	/* Lorsque l'on change la valeur de la liste la fonction est appel�e pour mettre � jour la structure Options */
	g_signal_connect(pListeResolution, "changed", G_CALLBACK(ModifierOptionsListe), pOptions);


	/* On met la liste sur la r�solution actuelle*/
	switch(pOptions->largeur)
	{
	case 800:
		gtk_combo_box_set_active(GTK_COMBO_BOX(pListeResolution), 0);
		break;

	case 1280:
		gtk_combo_box_set_active(GTK_COMBO_BOX(pListeResolution), 1);
		break;

	case 1440:
		gtk_combo_box_set_active(GTK_COMBO_BOX(pListeResolution), 2);
		break;

	case 1680:
		gtk_combo_box_set_active(GTK_COMBO_BOX(pListeResolution), 3);
		break;

	case 1920:
		gtk_combo_box_set_active(GTK_COMBO_BOX(pListeResolution), 4);
		break;

	default:
		gtk_combo_box_set_active(GTK_COMBO_BOX(pListeResolution), 0);
		break;
	}

	/* On cr�e les 2 curseurs pour la vie et pour le volume*/
	pCurseur1 = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 5, 1); // Entre 1 et 5 avec un pas de 1
	gtk_range_set_value(GTK_RANGE(pCurseur1), pOptions->vies);
	gtk_widget_set_margin_left(pCurseur1, 200);
	gtk_widget_set_margin_right(pCurseur1, 200);
	gtk_widget_set_margin_bottom(pCurseur1, 50);
	g_signal_connect(pCurseur1, "change-value", G_CALLBACK(ModifierOptionsRange1), pOptions);

	pCurseur2 = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);	//Entre 0 et 100 avec un pas de 1
	gtk_range_set_value(GTK_RANGE(pCurseur2), pOptions->volume);
	gtk_widget_set_margin_left(pCurseur2, 80);
	gtk_widget_set_margin_right(pCurseur2, 80);
	gtk_widget_set_margin_bottom(pCurseur2, 30);
	g_signal_connect(pCurseur2, "change-value", G_CALLBACK(ModifierOptionsRange2), pOptions);

	/* On met dans la bo�te V1 tous les widgets et leurs labels explicatifs*/
	gtk_box_pack_start(GTK_BOX(pVbox1), pResolution, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox1), pListeResolution, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox1), pNBVies, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox1), pCurseur1, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox1), pVolume, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox1), pCurseur2, false, false, 0);

	/* On cr�e une seconde bo�te verticale pour mettre les 2 autres bo�tes dedans*/
	pVbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	gtk_box_pack_start(GTK_BOX(pVbox2), pHbox1, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox2), pVbox1, false, false, 0);

	/* On met cette seconde bo�te verticale et tous ce qu'on a cr��e dans le s�parateur qu'il y est une bordure autour */
	gtk_container_add(GTK_CONTAINER(pSeparateur), pVbox2);

	/* On cr�e une seconde bo�te horizontale pour mettre les boutons Valider et Annuler */
	pHbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_margin_top(pHbox2, 40);

	/* On cr�e ces fameux boutons que l'on connecte aux fonctions correspondantes, pour fermer la fen�tre ou pour sauvegarder puis fermer la fen�tre */
	pBoutonAnnuler = gtk_button_new_with_label("ANNULER");
	gtk_widget_set_margin_left(pBoutonAnnuler, 120);
	g_signal_connect(pBoutonAnnuler, "clicked", G_CALLBACK(FermerFenetre), pData);

	pBoutonValider = gtk_button_new_with_label("VALIDER");
	gtk_widget_set_margin_right(pBoutonValider, 120);
	g_signal_connect(pBoutonValider, "clicked", G_CALLBACK(SauverOptions), pOptions);

	/* On ajoute les boutons dans la bo�te H2 */
	gtk_box_pack_start(GTK_BOX(pHbox2), pBoutonAnnuler, false, false, 0);
	gtk_box_pack_end(GTK_BOX(pHbox2), pBoutonValider, false, false, 0);

	/* On cr�e enfin une derni�re bo�te verticale pour tout mettre dedans */
	pVbox3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	/* On met le titre, le s�parateur avec tous les widgets de r�glage et enfin la bo�te H2 avec les boutons de validation */
	gtk_box_pack_start(GTK_BOX(pVbox3), pTitre, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox3), pSeparateur, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox3), pHbox2, false, false, 0);

	/* On termine en mettant cette bo�te V3 dans la fen�tre */
	gtk_container_add(GTK_CONTAINER(pWindow), pVbox3);

	/* On cache la fen�tre du menu et on affiche celle des options */
	gtk_widget_hide(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));
	gtk_widget_show_all(pWindow);
}


void Peindre(GtkWidget *pWidget, cairo_t *cr, gpointer pData)
{
	/* Cette fonction permet de "peindre" les images de l'animation des boules dans le menu, 'cr' est le contexte cairo g�r� automatiquement par gtk */
	gdk_cairo_set_source_pixbuf(cr, ((GdkImages*)pData)->img[0], coordonnees.x1, coordonnees.y1);
	cairo_paint(cr);

	gdk_cairo_set_source_pixbuf(cr, ((GdkImages*)pData)->img[1], coordonnees.x2, coordonnees.y2);
	cairo_paint(cr);

	gdk_cairo_set_source_pixbuf(cr, ((GdkImages*)pData)->img[2], coordonnees.x3, coordonnees.y3);
	cairo_paint(cr);
}


void PeindreV1(GtkWidget *pWidget, cairo_t *cr, gpointer pData)
{
	/* Cette fonction "peint" le missile de gauche */

	cairo_matrix_t mat;	//Matrice de transformation de l'image

	/* On initialise la matrice, on fait une mise � l'�chelle de l'image et lie la matrice au contexte de cairo */
	cairo_matrix_init_identity (&mat);
	cairo_matrix_scale (&mat, 0.18, 0.18);
	cairo_set_matrix (cr, &mat);

	/* On d�finit l'image � "peindre" (le missile), et on "peint" */
	gdk_cairo_set_source_pixbuf(cr, ((GdkImages*)pData)->img[3], coordonnees.x4, coordonnees.y4);
	cairo_paint(cr);
}


void PeindreV2(GtkWidget *pWidget, cairo_t *cr, gpointer pData)
{
	/* Cette fonction "peint" le missile de droite */

	cairo_matrix_t mat;	//Matrice de transformation de l'image

	/* On initialise la matrice, on fait une mise � l'�chelle de l'image et lie la matrice au contexte de cairo */
	cairo_matrix_init_identity (&mat);
	cairo_matrix_scale (&mat, 0.18, 0.18);
	cairo_set_matrix (cr, &mat);

	/* On d�finit l'image � "peindre" (le missile), et on "peint" */
	gdk_cairo_set_source_pixbuf(cr, ((GdkImages*)pData)->img[3], coordonnees.x4, coordonnees.y4);
	cairo_paint(cr);
}


int Avancer()
{
	/* Ici on va mettre � jour les coordonn�es des images des animations du menu dans la structure globale Coordonnees */

	/* Premi�re boule en x */
	if (coordonnees.x1 <= 800)
	{
		coordonnees.x1 += 3;
	}
	else
	{
		coordonnees.x1 = 0;
	}

	/* Deuxi�me boule en x */
	if (coordonnees.x2 <= 800)
	{
		coordonnees.x2 += 4;
	}
	else
	{
		coordonnees.x2 = 0;
	}

	/* Troisi�me boule en x */
	if (coordonnees.x3 <= 800)
	{
		coordonnees.x3 += 5;
	}
	else
	{
		coordonnees.x3 = 0;
	}

	/* Premi�re boule en y */
	if(coordonnees.y1 < 120 && coordonnees.sens1)
	{
		coordonnees.y1 += 4;
	}
	else if (coordonnees.y1 >= 120 && coordonnees.sens1)
	{
		coordonnees.sens1 = false;
	}

	if (coordonnees.y1 > 0 && !coordonnees.sens1)
	{
		coordonnees.y1 -=2;
	}
	else if (coordonnees.y1 <= 0 && !coordonnees.sens1)
	{
		coordonnees.sens1 = true;
	}

	/* Deuxi�me boule en y */
	if(coordonnees.y2 < 120 && coordonnees.sens2)
	{
		coordonnees.y2 += 4;
	}
	else if (coordonnees.y2 >= 120 && coordonnees.sens2)
	{
		coordonnees.sens2 = false;
	}

	if (coordonnees.y2 > 0 && !coordonnees.sens2)
	{
		coordonnees.y2 -=2;
	}
	else if (coordonnees.y2 <= 0 && !coordonnees.sens2)
	{
		coordonnees.sens2 = true;
	}

	/* Troisi�me boule en y */
	if(coordonnees.y3 < 120 && coordonnees.sens3)
	{
		coordonnees.y3 += 4;
	}
	else if (coordonnees.y3 >= 120 && coordonnees.sens3)
	{
		coordonnees.sens3 = false;
	}

	if (coordonnees.y3 > 0 && !coordonnees.sens3)
	{
		coordonnees.y3 -=2;
	}
	else if (coordonnees.y3 <= 0 && !coordonnees.sens3)
	{
		coordonnees.sens3 = true;
	}

	/* Missiles sur les c�t�s en y (x est constant) */
	if (coordonnees.y4 > 0 && !coordonnees.sens4)
	{
		coordonnees.y4 -= 10;
	}
	else if(coordonnees.y4 <= 0 && !coordonnees.sens4)
	{
		coordonnees.sens4 = true;
	}

	if (coordonnees.y4 < 1400 && coordonnees.sens4)
	{
		coordonnees.y4 += 10;
	}
	else if(coordonnees.y4 >= 1400 && coordonnees.sens4)
	{
		coordonnees.sens4 = false;
	}

	return true;	//On retourne 'true' pour que la r�p�tition de la fonction continue
}


void FermerCredit(GtkWidget *pWidget, GdkEvent *event, gpointer pData)
{
	FMOD_CHANNEL *channelEnCours;

	/* On met en pause la musique des cr�dits */
	FMOD_System_GetChannel((FMOD_SYSTEM *)g_slist_nth_data(pData, 3), M_CREDITS, &channelEnCours);
	FMOD_Channel_SetPaused(channelEnCours, true);

	/* On remet la musique du menu en marche */
	FMOD_System_GetChannel((FMOD_SYSTEM *)g_slist_nth_data(pData, 3), M_MENU, &channelEnCours);
	FMOD_Channel_SetPaused(channelEnCours, false);

	/* On d�truit la fen�tre des cr�dits et on affiche la fen�tre du menu */
	gtk_widget_destroy(pWidget);
	gtk_widget_show_all(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));
}


void FermerFenetre(GtkWidget *pWidget, gpointer pData)
{
	/* Cette fonction permet de fermer la fen�tre re�ue en param�tre */

	/* On r�cup�re la fen�tre si jamais 'pWidget' est un bouton */
	GtkWidget *pWindow = gtk_widget_get_toplevel(pWidget);

	/* On ferme la fen�tre si on a bien r�cup�r� une fen�tre (top level) */
	if(gtk_widget_is_toplevel(pWindow))
	{
		gtk_widget_destroy(gtk_widget_get_toplevel(pWidget));
	}
}


void SauverOptions(GtkWidget *pWidget, gpointer pData)
{
	Options *pOptions = pData;	//On prend le pointeur sur la structure Options re�u en param�tre
	char options[10][50] = {{""}};	//Un tableau de 10 cha�nes de 50 caract�res
	char chaine[50] = "";	//Une cha�nes de 50 caract�res

	options[0][0] = (char)pOptions->nbLigne;	//On stocke le nombre de ligne � mettre dans le fichier d'options dans le premier octet du tableau

	/* On �crit dans chaque ligne qui suit une option et sa valeur */
	sprintf(chaine, "musique=%d", pOptions->musique);
	strcat(options[1], chaine);
	sprintf(chaine, "sons=%d", pOptions->sons);
	strcat(options[2], chaine);
	sprintf(chaine, "r�solution=%dx%d", pOptions->largeur, pOptions->hauteur);
	strcat(options[3], chaine);
	sprintf(chaine, "vies=%d", pOptions->vies);
	strcat(options[4], chaine);
	sprintf(chaine, "musique=%lf", pOptions->volume);
	strcat(options[5], chaine);
	sprintf(chaine, "full=%d", pOptions->fullScreen);
	strcat(options[6], chaine);

	ValiderChangement(options);	//On sauvegarde dans le fichier

/* On r�affecte les variables globales avec les nouvelles options que l'on vient d'enregistrer pour qu'elles prennent effet sans avoir besoin de red�marrer */
	Hauteur = pOptions->hauteur;
	Largeur = pOptions->largeur;
	TailleBloc = ceil(Largeur/40.0);
	TailleBoule = Arrondir(Largeur/45.0);
	TailleMissileW = Arrondir(Largeur/30.0);
	TailleMissileH = Arrondir(Hauteur/5.5);
	BSons = pOptions->sons;
	BMusique = pOptions->musique;
	Volume = pOptions->volume;

	FermerFenetre(pWidget, NULL);	//On ferme la fen�tre
}


void AfficherMenu(GtkWidget *pWidget, gpointer pData)
{
	gtk_widget_show_all(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));	//Permet d'afficher le fen�tre du menu
}


void ModifierOptionsToggleButton1(GtkToggleButton *pToggleButton, gpointer pData)
{
	/* On met � jour la structure Options avec l'�tat du bouton */
	int etat = gtk_toggle_button_get_active(pToggleButton);
	Options *pOptions = pData;

	pOptions->musique = etat;
}


void ModifierOptionsToggleButton2(GtkToggleButton *pToggleButton, gpointer pData)
{
	/* On met � jour la structure Options avec l'�tat du bouton */
	int etat = gtk_toggle_button_get_active(pToggleButton);
	Options *pOptions = pData;

	pOptions->sons = etat;
}


void ModifierOptionsToggleButton3(GtkToggleButton *pToggleButton, gpointer pData)
{
	/* On met � jour la structure Options avec l'�tat du bouton */
	int etat = gtk_toggle_button_get_active(pToggleButton);
	Options *pOptions = pData;

	pOptions->fullScreen = etat;
}


void ModifierOptionsListe(GtkComboBox *pComboBox, gpointer pData)
{
	/* On met � jour la structure Options avec le choix s�lectionn� dans la liste */
	int choix = gtk_combo_box_get_active(pComboBox);
	Options *pOptions = pData;

	switch(choix)
	{
	case 0:
		pOptions->largeur = 800;
		pOptions->hauteur = 500;
		break;

	case 1:
		pOptions->largeur = 1280;
		pOptions->hauteur = 800;
		break;

	case 2:
		pOptions->largeur = 1440;
		pOptions->hauteur = 900;
		break;

	case 3:
		pOptions->largeur = 1680;
		pOptions->hauteur = 1050;
		break;

	case 4:
		pOptions->largeur = 1920;
		pOptions->hauteur = 1200;
		break;

	default:	//Par d�faut on met la r�solution la plus basse
		pOptions->largeur = 800;
		pOptions->hauteur = 500;
		break;
	}
}


gboolean ModifierOptionsRange1(GtkRange *range, GtkScrollType scroll, double valeur, gpointer pData)
{
	/* On met � jour la structure Options avec la valeur du curseur */
	Options *pOptions = pData;

	pOptions->vies = (char)Arrondir(valeur);

	return false;	//On renvoie 'false' pour propager le signal "change-value" � gtk
}

gboolean ModifierOptionsRange2(GtkRange *range, GtkScrollType scroll, double valeur, gpointer pData)
{
	/* On met � jour la structure Options avec la valeur du curseur */
	Options *pOptions = pData;

	pOptions->volume = (float)valeur;

	return false;	//On renvoie 'false' pour propager le signal "change-value" � gtk
}

void Connexion(GtkWidget *pWidget, gpointer pData)
{
	GtkWidget *pWindow, *pEntryPseudo, *pEntryMDP, *pBoutonCO, *pBoutonGuest, *pBoutonAnnuler, *pBoutonAnnulerLabel, *pBoutonCOLabel, *pBoutonGuestLabel, *pBoxHEntries, *pBoxHButtons, *pBoxVAll;
	GtkEntryBuffer *pBufferPseudo, *pBufferMDP;	//2 buffers pour les champs de saisie
	GdkRGBA couleurBoutons= {0.650, 0.850, 0.925, 1}, couleurBoutonsEnfonce= {0.550, 0.655, 0.700, 1}, couleurFond= {0.610, 0.805, 0.920, 1};

	GSList *pListeElements = (GSList*)pData;	//On r�cup�re la liste des �l�ments

	/* On cr�e la fen�tre, on la centre, on lui donne sa taille */
	pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_icon_from_file(GTK_WINDOW(pWindow), "ressources/img/z.png", NULL);
	gtk_window_set_title(GTK_WINDOW(pWindow), "Connexion");
	gtk_widget_override_background_color(pWindow, GTK_STATE_FLAG_NORMAL, &couleurFond);
	gtk_widget_set_size_request(pWindow, 600, 200);
	gtk_window_set_resizable(GTK_WINDOW(pWindow), false);
	gtk_window_set_position(GTK_WINDOW(pWindow), GTK_WIN_POS_CENTER_ALWAYS);

	/*							   */
	/* On cr�e le bouton CONNEXION */
	pBoutonCO = gtk_button_new();
	pBoutonCOLabel = gtk_label_new(g_locale_to_utf8("<span size=\"15000\"><b>Connexion !</b></span>", -1, NULL, NULL, NULL));
	gtk_label_set_use_markup(GTK_LABEL(pBoutonCOLabel), true);
	gtk_label_set_justify(GTK_LABEL(pBoutonCOLabel), GTK_JUSTIFY_CENTER);
	gtk_container_add(GTK_CONTAINER(pBoutonCO), pBoutonCOLabel);
	gtk_widget_override_background_color(pBoutonCO, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pBoutonCO, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
	gtk_widget_override_background_color(pBoutonCOLabel, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pBoutonCOLabel, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
	gtk_widget_set_margin_right(pBoutonCO, 50);
	gtk_widget_set_margin_top(pBoutonCO, 80);
	g_signal_connect(G_OBJECT(pBoutonCO), "clicked", G_CALLBACK(ConnexionMySql), pListeElements);	//On appelle 'ConnexionMySql' quand on clique dessus

	/*						   */
	/* On cr�e le bouton GUEST */
	pBoutonGuest = gtk_button_new();
	pBoutonGuestLabel = gtk_label_new("<span size=\"15000\"><b>Mode GUEST</b></span>");
	gtk_label_set_use_markup(GTK_LABEL(pBoutonGuestLabel), true);
	gtk_label_set_justify(GTK_LABEL(pBoutonGuestLabel), GTK_JUSTIFY_CENTER);
	gtk_container_add(GTK_CONTAINER(pBoutonGuest), pBoutonGuestLabel);
	gtk_widget_override_background_color(pBoutonGuest, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pBoutonGuest, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
	gtk_widget_override_background_color(pBoutonGuestLabel, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pBoutonGuestLabel, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
	gtk_widget_set_margin_left(pBoutonGuest, 40);
	gtk_widget_set_margin_top(pBoutonGuest, 80);
	g_signal_connect(G_OBJECT(pBoutonGuest), "clicked", G_CALLBACK(ModeGuest), pListeElements);	//On appelle 'ModeGuest' quand on clique dessus

	/*							 */
	/* On cr�e le bouton ANNULER */
	pBoutonAnnuler = gtk_button_new();
	pBoutonAnnulerLabel = gtk_label_new(g_locale_to_utf8("<span size=\"15000\"><b>Annuler</b></span>", -1, NULL, NULL, NULL));
	gtk_label_set_use_markup(GTK_LABEL(pBoutonAnnulerLabel), true);
	gtk_label_set_justify(GTK_LABEL(pBoutonAnnulerLabel), GTK_JUSTIFY_CENTER);
	gtk_container_add(GTK_CONTAINER(pBoutonAnnuler), pBoutonAnnulerLabel);
	gtk_widget_override_background_color(pBoutonAnnuler, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pBoutonAnnuler, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
	gtk_widget_override_background_color(pBoutonAnnulerLabel, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pBoutonAnnulerLabel, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
	gtk_widget_set_margin_left(pBoutonAnnuler, 50);
	gtk_widget_set_margin_top(pBoutonAnnuler, 80);
	g_signal_connect(G_OBJECT(pBoutonAnnuler), "clicked", G_CALLBACK(FermerFenetre), NULL);	//On ferme quand on clique dessus

	/* On met une cha�ne vide dans le buffer et on le lie au champ de saisie*/
	pBufferPseudo = gtk_entry_buffer_new("", 0);
	pEntryPseudo = gtk_entry_new_with_buffer(GTK_ENTRY_BUFFER(pBufferPseudo));
	gtk_entry_set_placeholder_text(GTK_ENTRY(pEntryPseudo), "Pseudo");	//Remplisseur pour indiquer ce qu'il faut mettre dans le champ
	gtk_widget_set_margin_left(pEntryPseudo, 100);
	gtk_widget_set_margin_top(pEntryPseudo, 40);

	/* On met une cha�ne vide dans le buffer et on le lie au champ de saisie*/
	pBufferMDP = gtk_entry_buffer_new("", 0);
	pEntryMDP = gtk_entry_new_with_buffer(GTK_ENTRY_BUFFER(pBufferMDP));
	gtk_entry_set_placeholder_text(GTK_ENTRY(pEntryMDP), "Mot de passe");//Remplisseur pour indiquer ce qu'il faut mettre dans le champ
	gtk_entry_set_visibility(GTK_ENTRY(pEntryMDP), false);
	gtk_widget_set_margin_right(pEntryMDP, 100);
	gtk_widget_set_margin_top(pEntryMDP, 40);

	/* On cr�e 2 bo�tes horizontales pour stocker les champs de saisie et les boutons*/
	pBoxHButtons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	pBoxHEntries = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	/* Une grande bo�te verticale pour mettre les 2 autres dedans */
	pBoxVAll = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	/* On ajoute les 3 boutons dans la bo�tes H1 */
	gtk_box_pack_start(GTK_BOX(pBoxHButtons), pBoutonAnnuler, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pBoxHButtons), pBoutonGuest, false, false, 0);
	gtk_box_pack_end(GTK_BOX(pBoxHButtons), pBoutonCO, false, false, 0);

	/* On ajoute les 2 champs de saisie dans la bo�te H2 */
	gtk_box_pack_start(GTK_BOX(pBoxHEntries), pEntryPseudo, false, false, 0);
	gtk_box_pack_end(GTK_BOX(pBoxHEntries), pEntryMDP, false, false, 0);

	/* On ajoute les deux bo�tes pr�c�dentes dans V1 */
	gtk_box_pack_start(GTK_BOX(pBoxVAll), pBoxHEntries, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pBoxVAll), pBoxHButtons, false, false, 0);

	/* On ajoute la bo�te V1 dans la fen�tre */
	gtk_container_add(GTK_CONTAINER(pWindow), pBoxVAll);

	/* On ajoute les pointeurs des 2 buffers � la liste cha�n�e */
	g_list_append((GList*)pListeElements, pBufferPseudo);
	g_list_append((GList*)pListeElements, pBufferMDP);

	/* On met le focus sur le bouton CONNEXION par d�faut */
	gtk_widget_grab_focus(pBoutonCO);

	/* On affiche la fen�tre */
	gtk_widget_show_all(pWindow);
}


void ConnexionMySql(GtkWidget *pWidget, gpointer pData)
{
	/* Cette fonction permet de se connecter � la base MySql et de r�cup�rer les scores du joueur si celui-ci c'est correctement identifi� */

	MYSQL *mysql = mysql_init(NULL);	//On initialise une structure MYSQL qui va contenir les informations de connexion
	MYSQL_RES *resultat;	//Pointeur sur un jeu de r�sultat
	MYSQL_ROW row;	//Pointeur sur une structure pour contenir les colonnes d'une requ�te
	GdkRGBA couleurFond= {0.610, 0.805, 0.920, 1};

	/* On cr�e une nouvelle fen�tre de dialogue */
	GtkWidget *pWindowInfo = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(pWidget)), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_OTHER, GTK_BUTTONS_OK, "");

	/* On r�cup�re les pointeurs vers les structures Joueur et les 2 buffers */
	Joueur *pJoueur = (Joueur*)g_list_nth_data((GList*)pData, 6);
	GtkWidget *pEntryPseudo = (GtkWidget *)g_list_nth_data((GList*)pData, 7), *pEntryMDP = (GtkWidget *)g_list_nth_data((GList*)pData, 8);

	/* On d�clare des structures pour utiliser les fonctions de la biblioth�que md5 */
	md5_state_t etat;
	md5_byte_t mdp[100], digest[17];

	int i=0;	//Petit compteur

	InitialiserJoueur(pJoueur);	//On initialise la structure Joueur pour qu'elle soit vierge

	gtk_widget_override_background_color(pWindowInfo, GTK_STATE_FLAG_NORMAL, &couleurFond);

	/* On �crit dans les cha�nes ce qu'il y a dans les buffers */
	sprintf(pJoueur->pseudo, gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(pEntryPseudo)));
	sprintf((char*)mdp, gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(pEntryMDP)));

	md5_init(&etat);	//On initialise l'�tat du crypteur md5
	md5_append(&etat, mdp, strlen((char*)mdp));	//On ajoute le mot de passe � l'empreinte md5
	md5_finish(&etat, digest);	//On r�cup�re l'empreinte finale apr�s avoir ajout� toutes les cha�nes (ici seulement le mot de passe)

	digest[16] = '\0';	//On ajoute le caract�re de fin de cha�ne apr�s les 16 premiers caract�res

	/* On copie un � un, en base 16 avec 2 chiffres, chaque caract�re de l'empreinte */
	while (digest[i] != '\0')
	{
		sprintf((pJoueur->mdp)+2*i, "%02x", digest[i]);
		i++;
	}

	/* On prend les options de connexion par d�faut */
	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "default");

	/* Si la connexion � la base a r�ussie */
	if(mysql_real_connect(mysql, "mysql1.alwaysdata.com", "89504_beaussart", "beaussart62", "ballsandmovement_players", 3306, NULL, 0))
	{
		mysql_query(mysql, "SELECT * FROM projetz");	//On s�lectionne toutes les lignes de la base

		resultat = mysql_use_result(mysql);	//On stocke le r�sulat

		/* Tant qu'il y a encore une ligne, on d�coupe chaque colonne */
		while ((row = mysql_fetch_row(resultat)) != NULL)
		{
			if (strcmp(row[2], pJoueur->mdp) == 0 && strcmp(row[1], pJoueur->pseudo) == 0)	//Si on a trouv� le pseudo et que le mot de passe correspond
			{
				/* On connecte le joueur et on remplit les champs de la struture Joueur avec les infos de la base */
				pJoueur->connexion = 1;
				pJoueur->score_max = strtol(row[3], NULL, 10);
				pJoueur->niveau_max = strtol(row[4], NULL, 10);
				sprintf(pJoueur->autre, row[5]);
			}
		}

		/* Lib�ration du jeu de r�sultat */
		mysql_free_result(resultat);

		/* On ferme la connexion */
		mysql_close(mysql);
	}
	else	//Sinon on met le mode GUEST en marche, score et niveau par d�faut
	{
		pJoueur->connexion = 2;
		pJoueur->score_max = 1000;
		pJoueur->niveau_max = 1;
	}

	/* On regarde si la connexion a r�ussi, si elle est en mode GUEST ou si elle a �chou� et on affiche un message diff�rent */
	switch(pJoueur->connexion)
	{
		/* On dit que les identifiants ne sont pas bons et on grise les boutons JOUER et EDITEUR */
	case 0:
		gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(pWindowInfo), "Impossible de se connecter");
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(pWindowInfo), g_locale_to_utf8("Identifiants invalides.\nPseudo: %s introuvable ou ne correspondant pas au mot de passe entr�.", -1, NULL, NULL, NULL), pJoueur->pseudo);
		gtk_widget_set_sensitive(g_list_nth_data(pData, 1), false);
		gtk_widget_set_sensitive(g_list_nth_data(pData, 2), false);
		break;

		/* Connexion r�ussie: on affiche un message de bienvenue avec le pseudo et on d�grise les boutons */
	case 1:
		gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(pWindowInfo), g_locale_to_utf8("Connexion r�ussie!", -1, NULL, NULL, NULL));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(pWindowInfo), g_locale_to_utf8("Bienvenu %s", -1, NULL, NULL, NULL), pJoueur->pseudo);
		gtk_widget_set_sensitive(g_list_nth_data(pData, 1), true);
		gtk_widget_set_sensitive(g_list_nth_data(pData, 2), true);
		break;

		/* Connexion impossible: on affiche la cause de l'erreur, on active le mode GUEST et on d�grise les boutons */
	case 2:
		gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(pWindowInfo), g_locale_to_utf8("Connexion au serveur impossible. Mode GUEST activ�, vos scores ne seront pas sauvegard�s.", -1, NULL, NULL, NULL));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(pWindowInfo), "Erreur: %s\n", mysql_error(mysql));
		gtk_widget_set_sensitive(g_list_nth_data(pData, 1), true);
		gtk_widget_set_sensitive(g_list_nth_data(pData, 2), true);
		break;
	}

	/* On lance le dialogue d'information */
	gtk_dialog_run(GTK_DIALOG(pWindowInfo));

	/* On ferme la fen�tre de connexion quand le joueur � cliquer sur la fen�tre de dialogue */
	FermerFenetre(pWidget, NULL);

	/* On d�r�f�rence les buffers pour qu'ils soient d�truits */
	g_object_unref(pEntryMDP);
	g_object_unref(pEntryPseudo);

	/* On retire les pointeurs de la liste puisque les buffers n'existent plus et pour pouvoir r�essayer une autre fois de se connecter */
	g_list_remove(pData, pEntryMDP);
	g_list_remove(pData, pEntryPseudo);
}

int QuitterEchape(GtkWidget *pWidget, GdkEventKey *pEvent)
{
	/* Cette fonction est appel�e lors d'un appui sur une touche, elle v�rifie qu'il s'agit de la touche Echap, puis d�truit la fen�tre ce qui entraine l'apparition du dialogue voulez- vous quitter ? */
	if(pEvent->keyval == GDK_KEY_Escape)
	{
		DestructionFenetre(pWidget, NULL);
	}

	return false; //On renvoie 'false' pour propager le signal � gtk
}

void ModeGuest(GtkWidget *pWidget, gpointer pData)
{
	/* Cette fonction est appel�e si on clique sur 'Mode GUEST' dans la fen�tre de connexion */

	Joueur *pJoueur = (Joueur*)g_list_nth_data((GList*)pData, 6);	//On r�cup�re le pointeur vers la structure Joueur

	/* On cr�e un nouveau dialogue pour informer du mode GUEST */
	GtkWidget *pWindowInfo = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(pWidget)), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_OTHER, GTK_BUTTONS_OK, "Vous jouer en mode GUEST");

	/* On r�cup�re les pointeurs des buffers */
	GtkWidget *pEntryPseudo = (GtkWidget *)g_list_nth_data((GList*)pData, 7), *pEntryMDP = (GtkWidget *)g_list_nth_data((GList*)pData, 8);
	GdkRGBA couleurFond= {0.610, 0.805, 0.920, 1};

	gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(pWindowInfo), g_locale_to_utf8("Vos scores ne seront pas sauvegard�s.", -1, NULL, NULL, NULL));

	gtk_widget_override_background_color(pWindowInfo, GTK_STATE_FLAG_NORMAL, &couleurFond);

	/* On met � z�ro les champs de la structure Joueur */
	InitialiserJoueur(pJoueur);

	/* On met le score et le niveau par d�faut, on signal que l'on est en mode GUEST */
	pJoueur->connexion = 2;
	pJoueur->score_max = 1000;
	pJoueur->niveau_max = 1;

	/* On lance le dialogue */
	gtk_dialog_run(GTK_DIALOG(pWindowInfo));

	/* On d�grise les boutons JOUER et EDITEUR */
	gtk_widget_set_sensitive(g_list_nth_data(pData, 1), true);
	gtk_widget_set_sensitive(g_list_nth_data(pData, 2), true);

	FermerFenetre(pWidget, NULL);	//On ferme la fen�tre

	/* On d�r�f�rence les buffers */
	g_object_unref(pEntryMDP);
	g_object_unref(pEntryPseudo);

	/* On supprime les buffers de la liste cha�n�e */
	g_list_remove(pData, pEntryMDP);
	g_list_remove(pData, pEntryPseudo);
}

//Fin du fichier callback.c
