/*
Projet-ISN

Fichier: callback.c

Contenu: Fonctions de callback pour l'interface du menu

Actions: C'est ici que se trouve les fonctions pour l'interface du menu.

Bibliothèques utilisées: Bibliothèques standards, SDL, SDL_image, SDL_ttf, FMOD, GTK

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

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;
extern double Volume, Largeur, Hauteur;

Coordo coordonnees =
{
	100, 400, 800, 750,
	0, 50, 120, 2000,
	false, true, false, false
};

int Redessiner(gpointer pData)
{
	gtk_widget_queue_draw(pData);

	return true;
}

void DestructionFenetre(GtkWidget *pWidget, gpointer pData)
{
	GtkWidget *pFenetreDemande = NULL, *pBoutonOUI, *pBoutonNON;
	GdkRGBA couleurFond= {0.610, 0.805, 0.920, 1}, couleurBoutons= {0.650, 0.850, 0.925, 1}, couleurBoutonsEnfonce= {0.550, 0.655, 7.000, 1};

	pFenetreDemande = gtk_dialog_new();

	gtk_window_set_position(GTK_WINDOW(pFenetreDemande), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(pFenetreDemande), "Vraiment ?");
	gtk_window_set_icon_from_file(GTK_WINDOW(pFenetreDemande), "ressources/img/z.png", NULL);
	gtk_window_set_resizable(GTK_WINDOW(pFenetreDemande), false);
	gtk_widget_override_background_color(pFenetreDemande, GTK_STATE_FLAG_NORMAL, &couleurFond);

	pBoutonNON = gtk_button_new_with_label("Non");
	gtk_widget_override_background_color(pBoutonNON, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pBoutonNON, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);

	pBoutonOUI = gtk_button_new_with_label("Oui");
	gtk_widget_override_background_color(pBoutonOUI, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pBoutonOUI, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);

	gtk_dialog_add_action_widget(GTK_DIALOG(pFenetreDemande), pBoutonNON, GTK_RESPONSE_NO);
	gtk_dialog_add_action_widget(GTK_DIALOG(pFenetreDemande), pBoutonOUI, GTK_RESPONSE_YES);

	gtk_widget_show_all(pFenetreDemande);

	switch(gtk_dialog_run(GTK_DIALOG(pFenetreDemande)))
	{
	case GTK_RESPONSE_YES:
		{
			/* Arrêt de la boucle évènementielle */
			gtk_main_quit();
			break;
		}

	case GTK_RESPONSE_NO:
		{
			break;
		}

	default:
		{
			break;
		}
	}

	gtk_widget_destroy(pFenetreDemande);
}

void OuvrirSDL(GtkWidget *pWidget, gpointer pData)
{
	Joueur *pJoueur = (Joueur *)g_slist_nth_data((GSList*)pData, 6);

	if (pJoueur->connexion)
	{
		gtk_widget_hide(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));

		if(pWidget == GTK_WIDGET(g_slist_nth_data((GSList*)pData, 1)))
		{
			LancerJeu((FMOD_SYSTEM *)g_slist_nth_data((GSList*)pData, 3), (Sons *)g_slist_nth_data((GSList*)pData, 4), "jouer", pJoueur);
		}

		else if(pWidget == GTK_WIDGET(g_slist_nth_data((GSList*)pData, 2)))
		{
			LancerJeu((FMOD_SYSTEM *)g_slist_nth_data((GSList*)pData, 3), (Sons *)g_slist_nth_data((GSList*)pData, 4), "editeur", pJoueur);
		}

		gtk_widget_show_all(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));
	}
}

void LancementCredits(GtkWidget *pWidget, gpointer pData)
{
	GtkWidget *lignesCredits[100] = {NULL};
	GtkWidget *pGrille=NULL, *pWindow=NULL;
	FMOD_CHANNEL *channelEnCours;
	GdkRGBA couleurFond= {0.610, 0.805, 0.920, 1};
	texte credits;
	int nbLignes = ChargementCredits(&credits);
	int i=0;

	FMOD_System_GetChannel((FMOD_SYSTEM *)g_slist_nth_data(pData, 3), M_MENU, &channelEnCours);
	FMOD_Channel_SetPaused(channelEnCours, true);

	FMOD_Sound_SetLoopCount(((Sons *)g_slist_nth_data(pData, 4))->music[M_CREDITS], -1);      // On active la lecture en boucle

	FMOD_System_PlaySound((FMOD_SYSTEM *)g_slist_nth_data(pData, 3), M_CREDITS, ((Sons *)g_slist_nth_data(pData, 4))->music[M_CREDITS], true, NULL);        // On lit la musique
	FMOD_System_GetChannel((FMOD_SYSTEM *)g_slist_nth_data(pData, 3), M_CREDITS, &channelEnCours);
	FMOD_Channel_SetVolume(channelEnCours, (float)(((Options*)g_slist_nth_data(pData, 5))->volume/100.0));
	FMOD_Channel_SetPaused(channelEnCours, false);

	lignesCredits[0] = gtk_label_new(g_locale_to_utf8("<span underline=\"single\" font-family=\"Snickles\" size=\"55000\">Crédits</span>", -1, NULL, NULL, NULL));
	gtk_label_set_use_markup(GTK_LABEL(lignesCredits[0]), true);

	gtk_widget_set_size_request(lignesCredits[0], 800, 50);

	for (i=1; i<nbLignes; i++)
	{
		lignesCredits[i] = gtk_label_new(g_markup_printf_escaped("<span font-family=\"Snickles\" size=\"15000\">%s</span>", g_locale_to_utf8(credits.chaines[i], -1, NULL, NULL, NULL)));
		gtk_label_set_use_markup(GTK_LABEL(lignesCredits[i]), true);

		gtk_widget_set_size_request(lignesCredits[i], 800, 600/nbLignes);
	}

	pGrille = gtk_grid_new();

	pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(pWindow), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_widget_set_size_request(pWindow, 800, 600);
	gtk_window_set_icon_from_file(GTK_WINDOW(pWindow), "ressources/img/z.png", NULL);
	gtk_widget_override_background_color(pWindow, GTK_STATE_FLAG_NORMAL, &couleurFond);
	g_signal_connect(pWindow, "delete-event", G_CALLBACK(FermerCredit), pData);

	gtk_widget_set_hexpand(lignesCredits[0], true);
	gtk_widget_set_halign (lignesCredits[0], GTK_ALIGN_CENTER);
	g_object_set(lignesCredits[0], "margin-top", 10, NULL);
	gtk_grid_attach(GTK_GRID(pGrille), lignesCredits[0], 0, 0, 10, 1);

	for (i=1; i<nbLignes; i++)
	{
		gtk_widget_set_hexpand(lignesCredits[i], true);
		gtk_widget_set_halign (lignesCredits[i], GTK_ALIGN_CENTER);
		gtk_grid_attach_next_to(GTK_GRID(pGrille), lignesCredits[i], NULL, GTK_POS_BOTTOM, 10, 1);
	}

	gtk_widget_hide(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));

	gtk_container_add(GTK_CONTAINER(pWindow), pGrille);

	gtk_widget_show_all(pWindow);
}

void LancementOptions(GtkWidget *pWidget, gpointer pData)
{
	GtkWidget *pWindow, *pHbox1, *pHbox2, *pVbox1, *pVbox2, *pVbox3, *pTitre, *pCase1, *pCase2, *pCase3, *pBoutonValider, *pBoutonAnnuler, *pListeResolution, *pCurseur1, *pCurseur2, *pResolution, *pNBVies, *pVolume, *pSeparateur;
	char options[50][50] = {{""}};
	Options *pOptions = NULL;
	GdkRGBA couleurFond= {0.610, 0.805, 0.920, 1};

	LectureOptions(options);
	pOptions = DecouperOptions(options);

	pWindow = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_title(GTK_WINDOW(pWindow), "Options");
	gtk_window_set_position(GTK_WINDOW(pWindow), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_widget_set_size_request(pWindow, 800, 600);
	gtk_widget_override_background_color(pWindow, GTK_STATE_FLAG_NORMAL, &couleurFond);
	g_signal_connect(pWindow, "destroy", G_CALLBACK(AfficherMenu), pData);

	pTitre = gtk_label_new("<span underline=\"single\" font-family=\"Snickles\" size=\"46080\">Options</span>");
	gtk_label_set_use_markup(GTK_LABEL(pTitre), true);
	gtk_widget_set_margin_bottom(pTitre, 40);
	gtk_widget_set_margin_top(pTitre, 20);

	pResolution = gtk_label_new(g_locale_to_utf8("<span underline=\"single\">Résolution:</span>", -1, NULL, NULL, NULL));
	gtk_label_set_use_markup(GTK_LABEL(pResolution), true);
	gtk_widget_set_margin_bottom(pResolution, 10);
	pNBVies = gtk_label_new("<span underline=\"single\">Vies:</span>");
	gtk_label_set_use_markup(GTK_LABEL(pNBVies), true);
	pVolume = gtk_label_new("<span underline=\"single\">Volume:</span>");
	gtk_label_set_use_markup(GTK_LABEL(pVolume), true);

	pSeparateur = gtk_frame_new(NULL);
	gtk_widget_set_margin_left(pSeparateur, 70);
	gtk_widget_set_margin_right(pSeparateur, 70);

	pHbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_margin_top(pHbox1, 20);

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

	gtk_box_pack_start(GTK_BOX(pHbox1), pCase1, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pHbox1), pCase3, false, false, 0);
	gtk_box_pack_end(GTK_BOX(pHbox1), pCase2, false, false, 0);

	pVbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_margin_top(pVbox1, 20);

	pListeResolution = gtk_combo_box_text_new();
	gtk_widget_set_margin_left(pListeResolution, 250);
	gtk_widget_set_margin_right(pListeResolution, 250);
	gtk_widget_set_margin_bottom(pListeResolution, 40);

	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pListeResolution), "800 x 500");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pListeResolution), "1280 x 800");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pListeResolution), "1440 x 900");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pListeResolution), "1680 x 1050");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(pListeResolution), "1920 x 1200");
	g_signal_connect(pListeResolution, "changed", G_CALLBACK(ModifierOptionsListe), pOptions);

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

	pCurseur1 = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 5, 1);
	gtk_range_set_value(GTK_RANGE(pCurseur1), pOptions->vies);
	gtk_widget_set_margin_left(pCurseur1, 200);
	gtk_widget_set_margin_right(pCurseur1, 200);
	gtk_widget_set_margin_bottom(pCurseur1, 50);
	g_signal_connect(pCurseur1, "change-value", G_CALLBACK(ModifierOptionsRange1), pOptions);

	pCurseur2 = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
	gtk_range_set_value(GTK_RANGE(pCurseur2), pOptions->volume);
	gtk_widget_set_margin_left(pCurseur2, 80);
	gtk_widget_set_margin_right(pCurseur2, 80);
	gtk_widget_set_margin_bottom(pCurseur2, 30);
	g_signal_connect(pCurseur2, "change-value", G_CALLBACK(ModifierOptionsRange2), pOptions);

	gtk_box_pack_start(GTK_BOX(pVbox1), pResolution, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox1), pListeResolution, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox1), pNBVies, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox1), pCurseur1, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox1), pVolume, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox1), pCurseur2, false, false, 0);

	pVbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	gtk_box_pack_start(GTK_BOX(pVbox2), pHbox1, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox2), pVbox1, false, false, 0);

	gtk_container_add(GTK_CONTAINER(pSeparateur), pVbox2);

	pHbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_margin_top(pHbox2, 40);

	pBoutonAnnuler = gtk_button_new_with_label("ANNULER");
	gtk_widget_set_margin_left(pBoutonAnnuler, 120);
	g_signal_connect(pBoutonAnnuler, "clicked", G_CALLBACK(FermerFenetre), pData);

	pBoutonValider = gtk_button_new_with_label("VALIDER");
	gtk_widget_set_margin_right(pBoutonValider, 120);
	g_signal_connect(pBoutonValider, "clicked", G_CALLBACK(SauverOptions), pOptions);

	gtk_box_pack_start(GTK_BOX(pHbox2), pBoutonAnnuler, false, false, 0);
	gtk_box_pack_end(GTK_BOX(pHbox2), pBoutonValider, false, false, 0);

	pVbox3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	gtk_box_pack_start(GTK_BOX(pVbox3), pTitre, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox3), pSeparateur, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pVbox3), pHbox2, false, false, 0);

	gtk_container_add(GTK_CONTAINER(pWindow), pVbox3);
	gtk_widget_hide(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));
	gtk_widget_show_all(pWindow);
}

void Peindre(GtkWidget *pWidget, cairo_t *cr, gpointer pData)
{
	gdk_cairo_set_source_pixbuf(cr, ((GdkImages*)pData)->img[0], coordonnees.x1, coordonnees.y1);
	cairo_paint(cr);

	gdk_cairo_set_source_pixbuf(cr, ((GdkImages*)pData)->img[1], coordonnees.x2, coordonnees.y2);
	cairo_paint(cr);

	gdk_cairo_set_source_pixbuf(cr, ((GdkImages*)pData)->img[2], coordonnees.x3, coordonnees.y3);
	cairo_paint(cr);
}

void PeindreV1(GtkWidget *pWidget, cairo_t *cr, gpointer pData)
{
	cairo_matrix_t mat;

	cairo_matrix_init_identity (&mat);
	cairo_matrix_scale (&mat, 0.18, 0.18);
	cairo_set_matrix (cr, &mat);

	gdk_cairo_set_source_pixbuf(cr, ((GdkImages*)pData)->img[3], coordonnees.x4, coordonnees.y4);
	cairo_paint(cr);
}

void PeindreV2(GtkWidget *pWidget, cairo_t *cr, gpointer pData)
{
	cairo_matrix_t mat;

	cairo_matrix_init_identity (&mat);
	cairo_matrix_scale (&mat, 0.18, 0.18);
	cairo_set_matrix (cr, &mat);

	gdk_cairo_set_source_pixbuf(cr, ((GdkImages*)pData)->img[3], coordonnees.x4, coordonnees.y4);
	cairo_paint(cr);
}

int Avancer()
{
	if (coordonnees.x1 <= 800)
	{
		coordonnees.x1 += 3;
	}
	else
	{
		coordonnees.x1 = 0;
	}

	if (coordonnees.x2 <= 800)
	{
		coordonnees.x2 += 4;
	}
	else
	{
		coordonnees.x2 = 0;
	}

	if (coordonnees.x3 <= 800)
	{
		coordonnees.x3 += 5;
	}
	else
	{
		coordonnees.x3 = 0;
	}

	if(coordonnees.y1 < 120 && coordonnees.sens1)
	{
		coordonnees.y1 += 4;
	}
	else if (coordonnees.y1 >= 120)
	{
		coordonnees.sens1 = false;
	}

	if (coordonnees.y1 > 0 && coordonnees.sens1 == false)
	{
		coordonnees.y1 -=2;
	}
	else if (coordonnees.sens1 == false)
	{
		coordonnees.sens1 = true;
	}

	if(coordonnees.y2 < 120 && coordonnees.sens2)
	{
		coordonnees.y2 += 4;
	}
	else if (coordonnees.y2 >= 120)
	{
		coordonnees.sens2 = false;
	}

	if (coordonnees.y2 > 0 && coordonnees.sens2 == false)
	{
		coordonnees.y2 -=2;
	}
	else if (coordonnees.sens2 == false)
	{
		coordonnees.sens2 = true;
	}

	if(coordonnees.y3 < 120 && coordonnees.sens3)
	{
		coordonnees.y3 += 4;
	}
	else if (coordonnees.y3 >= 120)
	{
		coordonnees.sens3 = false;
	}

	if (coordonnees.y3 > 0 && coordonnees.sens3 == false)
	{
		coordonnees.y3 -=2;
	}
	else if (coordonnees.sens3 == false)
	{
		coordonnees.sens3 = true;
	}

	if (coordonnees.y4 > 0 && !coordonnees.sens4)
	{
		coordonnees.y4 -= 10;
	}
	else if(!coordonnees.sens4)
	{
		coordonnees.sens4 = true;
	}

	if (coordonnees.y4 < 1400 && coordonnees.sens4)
	{
		coordonnees.y4 += 10;
	}
	else if(coordonnees.sens4)
	{
		coordonnees.sens4 = false;
	}

	return 1;
}

void FermerCredit(GtkWidget *pWidget, GdkEvent *event, gpointer pData)
{
	FMOD_CHANNEL *channelEnCours;

	FMOD_System_GetChannel((FMOD_SYSTEM *)g_slist_nth_data(pData, 3), M_CREDITS, &channelEnCours);
	FMOD_Channel_SetPaused(channelEnCours, true);

	FMOD_Sound_SetLoopCount(((Sons *)g_slist_nth_data(pData, 4))->music[M_MENU], -1);      // On active la lecture en boucle

	FMOD_System_GetChannel((FMOD_SYSTEM *)g_slist_nth_data(pData, 3), M_MENU, &channelEnCours);
	FMOD_Channel_SetPaused(channelEnCours, false);

	gtk_widget_destroy(pWidget);
	gtk_widget_show_all(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));
}

void FermerFenetre(GtkWidget *pWidget, gpointer pData)
{
	GtkWidget *pWindow = gtk_widget_get_toplevel(pWidget);

	if(gtk_widget_is_toplevel(pWindow))
	{
		gtk_widget_destroy(gtk_widget_get_toplevel(pWidget));
	}
}

void SauverOptions(GtkWidget *pWidget, gpointer pData)
{
	Options *pOptions = pData;
	char options[50][50] = {{""}};
	char chaine[50] = "";

	options[0][0] = (char)pOptions->nbLigne;
	sprintf(chaine, "musique=%d", pOptions->musique);
	strcat(options[1], chaine);
	sprintf(chaine, "sons=%d", pOptions->sons);
	strcat(options[2], chaine);
	sprintf(chaine, "résolution=%dx%d", pOptions->largeur, pOptions->hauteur);
	strcat(options[3], chaine);
	sprintf(chaine, "vies=%d", pOptions->vies);
	strcat(options[4], chaine);
	sprintf(chaine, "musique=%lf", pOptions->volume);
	strcat(options[5], chaine);
	sprintf(chaine, "full=%d", pOptions->fullScreen);
	strcat(options[6], chaine);

	ValiderChangement(options);

	FermerFenetre(pWidget, NULL);

	Hauteur = pOptions->hauteur;
	Largeur = pOptions->largeur;
	TailleBloc = ceil(Largeur/40.0);
	TailleBoule = Arrondir(Largeur/45.0);
	TailleMissileW = Arrondir(Largeur/30.0);
	TailleMissileH = Arrondir(Hauteur/5.5);
	BSons = pOptions->sons;
	BMusique = pOptions->musique;
	Volume = pOptions->volume;
}

void AfficherMenu(GtkWidget *pWidget, gpointer pData)
{
	gtk_widget_show_all(GTK_WIDGET(g_slist_nth_data((GSList*)pData, 0)));
}

void ModifierOptionsToggleButton1(GtkToggleButton *pToggleButton, gpointer pData)
{
	int etat = gtk_toggle_button_get_active(pToggleButton);
	Options *pOptions = pData;

	pOptions->musique = etat;
}

void ModifierOptionsToggleButton2(GtkToggleButton *pToggleButton, gpointer pData)
{
	int etat = gtk_toggle_button_get_active(pToggleButton);
	Options *pOptions = pData;

	pOptions->sons = etat;
}

void ModifierOptionsToggleButton3(GtkToggleButton *pToggleButton, gpointer pData)
{
	int etat = gtk_toggle_button_get_active(pToggleButton);
	Options *pOptions = pData;

	pOptions->fullScreen = etat;
}

void ModifierOptionsListe(GtkComboBox *pComboBox, gpointer pData)
{
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

	default:
		pOptions->largeur = 800;
		pOptions->hauteur = 500;
		break;
	}
}

gboolean ModifierOptionsRange1(GtkRange *range, GtkScrollType scroll, double valeur, gpointer pData)
{
	Options *pOptions = pData;

	pOptions->vies = (char)Arrondir(valeur);

	return false;
}

gboolean ModifierOptionsRange2(GtkRange *range, GtkScrollType scroll, double valeur, gpointer pData)
{
	Options *pOptions = pData;

	pOptions->volume = (float)valeur;

	return false;
}

void Connexion(GtkWidget *pWidget, gpointer pData)
{
	GtkWidget *pWindow, *pEntryPseudo, *pEntryMDP, *pBoutonCO, *pBoutonGuest, *pBoutonAnnuler, *pBoutonAnnulerLabel, *pBoutonCOLabel, *pBoutonGuestLabel, *pBoxHEntries, *pBoxHButtons, *pBoxVAll;
	GtkEntryBuffer *pBufferPseudo, *pBufferMDP;
	GdkRGBA couleurBoutons= {0.650, 0.850, 0.925, 1}, couleurBoutonsEnfonce= {0.550, 0.655, 0.700, 1}, couleurFond= {0.610, 0.805, 0.920, 1};

	GSList *pListeElements = (GSList*)pData;

	pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_icon_from_file(GTK_WINDOW(pWindow), "ressources/img/z.png", NULL);
	gtk_window_set_title(GTK_WINDOW(pWindow), "Connexion");
	gtk_widget_override_background_color(pWindow, GTK_STATE_FLAG_NORMAL, &couleurFond);
	gtk_widget_set_size_request(pWindow, 600, 200);
	gtk_window_set_resizable(GTK_WINDOW(pWindow), false);
	gtk_window_set_position(GTK_WINDOW(pWindow), GTK_WIN_POS_CENTER_ALWAYS);


	/* On crée le bouton CONNEXION */
	pBoutonCO = gtk_button_new();
	pBoutonCOLabel = gtk_label_new(g_locale_to_utf8("<span size=\"15000\"><b>Connexion !</b></span>", -1, NULL, NULL, NULL));
	gtk_label_set_use_markup(GTK_LABEL(pBoutonCOLabel), true);
	gtk_label_set_justify(GTK_LABEL(pBoutonCOLabel), GTK_JUSTIFY_CENTER);
	gtk_container_add(GTK_CONTAINER(pBoutonCO), pBoutonCOLabel);
	gtk_widget_override_background_color(pBoutonCO, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pBoutonCO, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
	gtk_widget_set_margin_right(pBoutonCO, 50);
	gtk_widget_set_margin_top(pBoutonCO, 80);
	g_signal_connect(G_OBJECT(pBoutonCO), "clicked", G_CALLBACK(ConnexionMySql), pListeElements);

	/* On crée le bouton GUEST */
	pBoutonGuest = gtk_button_new();
	pBoutonGuestLabel = gtk_label_new("<span size=\"15000\"><b>Mode GUEST</b></span>");
	gtk_label_set_use_markup(GTK_LABEL(pBoutonGuestLabel), true);
	gtk_label_set_justify(GTK_LABEL(pBoutonGuestLabel), GTK_JUSTIFY_CENTER);
	gtk_container_add(GTK_CONTAINER(pBoutonGuest), pBoutonGuestLabel);
	gtk_widget_override_background_color(pBoutonGuest, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pBoutonGuest, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
	gtk_widget_set_margin_left(pBoutonGuest, 40);
	gtk_widget_set_margin_top(pBoutonGuest, 80);
	g_signal_connect(G_OBJECT(pBoutonGuest), "clicked", G_CALLBACK(ModeGuest), pListeElements);

	/* On crée le bouton ANNULER */
	pBoutonAnnuler = gtk_button_new();
	pBoutonAnnulerLabel = gtk_label_new(g_locale_to_utf8("<span size=\"15000\"><b>Annuler</b></span>", -1, NULL, NULL, NULL));
	gtk_label_set_use_markup(GTK_LABEL(pBoutonAnnulerLabel), true);
	gtk_label_set_justify(GTK_LABEL(pBoutonAnnulerLabel), GTK_JUSTIFY_CENTER);
	gtk_container_add(GTK_CONTAINER(pBoutonAnnuler), pBoutonAnnulerLabel);
	gtk_widget_override_background_color(pBoutonAnnuler, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
	gtk_widget_override_background_color(pBoutonAnnuler, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
	gtk_widget_set_margin_left(pBoutonAnnuler, 50);
	gtk_widget_set_margin_top(pBoutonAnnuler, 80);
	g_signal_connect(G_OBJECT(pBoutonAnnuler), "clicked", G_CALLBACK(FermerFenetre), NULL);	//On ferme quand on clique dessus

	pBufferPseudo = gtk_entry_buffer_new("", 0);
	pEntryPseudo = gtk_entry_new_with_buffer(GTK_ENTRY_BUFFER(pBufferPseudo));
	gtk_entry_set_placeholder_text(GTK_ENTRY(pEntryPseudo), "Pseudo");
	gtk_widget_set_margin_left(pEntryPseudo, 100);
	gtk_widget_set_margin_top(pEntryPseudo, 40);


	pBufferMDP = gtk_entry_buffer_new("", 0);
	pEntryMDP = gtk_entry_new_with_buffer(GTK_ENTRY_BUFFER(pBufferMDP));
	gtk_entry_set_placeholder_text(GTK_ENTRY(pEntryMDP), "Mot de passe");
	gtk_entry_set_visibility(GTK_ENTRY(pEntryMDP), false);
	gtk_widget_set_margin_right(pEntryMDP, 100);
	gtk_widget_set_margin_top(pEntryMDP, 40);


	pBoxHButtons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	pBoxHEntries = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	pBoxVAll = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	gtk_box_pack_start(GTK_BOX(pBoxHButtons), pBoutonAnnuler, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pBoxHButtons), pBoutonGuest, false, false, 0);
	gtk_box_pack_end(GTK_BOX(pBoxHButtons), pBoutonCO, false, false, 0);

	gtk_box_pack_start(GTK_BOX(pBoxHEntries), pEntryPseudo, false, false, 0);
	gtk_box_pack_end(GTK_BOX(pBoxHEntries), pEntryMDP, false, false, 0);

	gtk_box_pack_start(GTK_BOX(pBoxVAll), pBoxHEntries, false, false, 0);
	gtk_box_pack_start(GTK_BOX(pBoxVAll), pBoxHButtons, false, false, 0);

	gtk_container_add(GTK_CONTAINER(pWindow), pBoxVAll);

	g_list_append((GList*)pListeElements, pBufferPseudo);
	g_list_append((GList*)pListeElements, pBufferMDP);

	gtk_widget_grab_focus(pBoutonAnnuler);

	gtk_widget_show_all(pWindow);
}

void ConnexionMySql(GtkWidget *pWidget, gpointer pData)
{
	MYSQL *mysql = mysql_init(NULL);
	MYSQL_RES *resultat;
	MYSQL_ROW row;
	GdkRGBA couleurFond= {0.610, 0.805, 0.920, 1};

	GtkWidget *pWindowInfo = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(pWidget)), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_OTHER, GTK_BUTTONS_OK, "");

	Joueur *pJoueur = (Joueur*)g_list_nth_data((GList*)pData, 6);
	GtkWidget *pEntryPseudo = (GtkWidget *)g_list_nth_data((GList*)pData, 7), *pEntryMDP = (GtkWidget *)g_list_nth_data((GList*)pData, 8);

	md5_state_t etat;
	md5_byte_t mdp[100], digest[17];

	int i=0;

	InitialiserJoueur(pJoueur);

	gtk_widget_override_background_color(pWindowInfo, GTK_STATE_FLAG_NORMAL, &couleurFond);

	sprintf(pJoueur->pseudo, gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(pEntryPseudo)));
	sprintf((char*)mdp, gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(pEntryMDP)));

	md5_init(&etat);
	md5_append(&etat, mdp, strlen((char*)mdp));
	md5_finish(&etat, digest);

	digest[16] = '\0';

	while (digest[i] != '\0')
	{
		sprintf((pJoueur->mdp)+2*i, "%02x", digest[i]);
		i++;
	}

	mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "default");

	if(mysql_real_connect(mysql, "mysql1.alwaysdata.com", "89504_beaussart", "beaussart62", "ballsandmovement_players", 3306, NULL, 0))
	{
		mysql_query(mysql, "SELECT * FROM projetz");

		resultat = mysql_use_result(mysql);

		/* Tant qu'il y a encore un résultat */
		while ((row = mysql_fetch_row(resultat)) != NULL)
		{
			if (strcmp(row[2], pJoueur->mdp) == 0 && strcmp(row[1], pJoueur->pseudo) == 0)
			{
				pJoueur->connexion = 1;
				pJoueur->score_max = Arrondir(strtol(row[3], NULL, 10));
				pJoueur->niveau_max = Arrondir(strtol(row[4], NULL, 10));
				sprintf(pJoueur->autre, row[5]);
			}
		}

		/*Libération du jeu de résultat*/
		mysql_free_result(resultat);

		mysql_close(mysql);
	}
	else
	{
		pJoueur->connexion = 2;
		pJoueur->score_max = 1000;
		pJoueur->niveau_max = 1;
	}

	switch(pJoueur->connexion)
	{
	case 0:
		gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(pWindowInfo), "Impossible de se connecter");
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(pWindowInfo), g_locale_to_utf8("Identifiants invalides.\nPseudo: %s introuvable ou ne correspondant pas au mot de passe entré.", -1, NULL, NULL, NULL), pJoueur->pseudo);
		gtk_widget_set_sensitive(g_list_nth_data(pData, 1), false);
		gtk_widget_set_sensitive(g_list_nth_data(pData, 2), false);
		break;

	case 1:
		gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(pWindowInfo), g_locale_to_utf8("Connexion réussie!", -1, NULL, NULL, NULL));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(pWindowInfo), g_locale_to_utf8("Bienvenu %s", -1, NULL, NULL, NULL), pJoueur->pseudo);
		gtk_widget_set_sensitive(g_list_nth_data(pData, 1), true);
		gtk_widget_set_sensitive(g_list_nth_data(pData, 2), true);
		break;

	case 2:
		gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(pWindowInfo), g_locale_to_utf8("Connexion au serveur impossible. Mode GUEST activé, vos scores ne seront pas sauvegardés.", -1, NULL, NULL, NULL));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(pWindowInfo), "Erreur: %s\n", mysql_error(mysql));
		gtk_widget_set_sensitive(g_list_nth_data(pData, 1), true);
		gtk_widget_set_sensitive(g_list_nth_data(pData, 2), true);
		break;

	default:
		break;
	}

	gtk_dialog_run(GTK_DIALOG(pWindowInfo));

	FermerFenetre(pWidget, NULL);

	g_object_unref(pEntryMDP);
	g_object_unref(pEntryPseudo);

	g_list_remove(pData, pEntryMDP);
	g_list_remove(pData, pEntryPseudo);
}

int QuitterEchape(GtkWidget *pWidget, GdkEventKey *pEvent)
{
	if(pEvent->keyval == GDK_KEY_Escape)
	{
		DestructionFenetre(pWidget, NULL);
	}

	return false;
}

void ModeGuest(GtkWidget *pWidget, gpointer pData)
{
	Joueur *pJoueur = (Joueur*)g_list_nth_data((GList*)pData, 6);
	GtkWidget *pWindowInfo = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(pWidget)), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_OTHER, GTK_BUTTONS_OK, "Vous jouer en mode GUEST");
	GtkWidget *pEntryPseudo = (GtkWidget *)g_list_nth_data((GList*)pData, 7), *pEntryMDP = (GtkWidget *)g_list_nth_data((GList*)pData, 8);
	GdkRGBA couleurFond= {0.610, 0.805, 0.920, 1};

	gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(pWindowInfo), g_locale_to_utf8("Vos scores ne seront pas sauvegardés.", -1, NULL, NULL, NULL));

	gtk_widget_override_background_color(pWindowInfo, GTK_STATE_FLAG_NORMAL, &couleurFond);

	InitialiserJoueur(pJoueur);

	pJoueur->connexion = 2;
	pJoueur->score_max = 1000;
	pJoueur->niveau_max = 1;

	gtk_dialog_run(GTK_DIALOG(pWindowInfo));

	gtk_widget_set_sensitive(g_list_nth_data(pData, 1), true);
	gtk_widget_set_sensitive(g_list_nth_data(pData, 2), true);

	FermerFenetre(pWidget, NULL);

	g_object_unref(pEntryMDP);
	g_object_unref(pEntryPseudo);

	g_list_remove(pData, pEntryMDP);
	g_list_remove(pData, pEntryPseudo);
}

//Fin du fichier callback.c
