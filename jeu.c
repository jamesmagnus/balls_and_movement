/*
Projet-ISN

Fichier: jeu.c

Contenu: Fonctions principales du jeu: la boucle, la gestion des collisions, l'affichage ...

Actions: C'est ici que sont effectuées les tâches principales du jeu, la boucle principale, le rendu à l'écran, les collisions, ...

Bibliothèques utilisées: Bibliothèques standards, SDL, SDL_image, SDL_ttf, FMOD, GTK

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <gtk/gtk.h>
#include <SDL2_gfxPrimitives.h>
#include <fmod.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "jeu.h"

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;		//Lien vers les variables globales déclarées dans main.c
extern double Volume, Largeur, Hauteur;
extern InfoDeJeu infos;

int Boucle_principale(Joueur *pJoueur, Sprite images[], Animation anim[], SDL_Renderer *pMoteurRendu, FMOD_SYSTEM *pMoteurSon, Sons *pSons, TTF_Font *polices[])
{
	ClavierSouris entrees;     //Structure pour connaître l'état du clavier et de la souris
	unsigned char descente[10]={true};	//Tableau pour savoir quand les missiles montent ou descendent
	unsigned char ajoutAnim=false;
	unsigned int tempsFPS=0, tempsAncienFPS=0;	//Différents temps pour les calculs
	int differenceFPS=0, etat=0, control=JEU_EN_COURS, etatNiveau;
	FMOD_CHANNEL *pChannelEnCours=NULL;	//Contrôle des canaux audio
	Map *pMap=NULL;	//Pointeurs sur des structures Map
	SDL_Event evenementPoubelle;	//Structure event pour purger la file des évènements

	EntreesZero(&entrees);	//On initialise la structure

	InitialisationPositions(images, pJoueur, infos.niveau);	//On charge les positions des images pour le niveau
	pMap = ChargementNiveau(pMoteurRendu, pJoueur, infos.niveau, &etatNiveau);	//On charge la map du niveau

	if(pMap == NULL)	//On vérifie que la map a bien été chargée
	{
		if(etatNiveau == CHARGEMENT_ERREUR || etatNiveau == CHARGEMENT_FICHIER_CORROMPU)	//Erreur de chargement
		{
			control = JEU_FIN_ERREUR_CHARGEMENT;
		}
		else if(etatNiveau == CHARGEMENT_GAGNE)
		{
			control = JEU_FIN_GAGNE;	//Fin normale, on a tout fait, on a déjà gagné
		}
	}

	while (SDL_PollEvent(&evenementPoubelle));	//On purge la file des évènements


	/* Boucle principale du jeu */
	while (control == JEU_EN_COURS)
	{
		if (ajoutAnim)	//Tant qu'une animation est en cours on bloque les commandes en purgeant la file et en remettant à zéro la structure
		{
			SDL_PollEvent(&evenementPoubelle);
			EntreesZero(&entrees);
		}
		else
		{
			GestionEvenements(&entrees);     //Traitement des événements du clavier et de la souris normalement
		}

		if(entrees.clavier[ECHAP] || entrees.fermeture)	//Appuis sur échap ou sur Entrée, on demande à quitter
		{
			entrees.clavier[ECHAP] = false;

			if(MessageInformations("Voulez-vous vraiment retourner au menu principal?", polices, pMoteurRendu, &entrees) == 1)
			{
				control = JEU_FIN;      //En cas d'appuie sur Entrée on coupe la boucle.
			}
		}

		if(entrees.clavier[F5])	//Appuis sur F5 on demande à recommencer le niveau
		{
			if(MessageInformations("Voulez-vous vraiment recommencer ce niveau ?", polices, pMoteurRendu, &entrees) == 1)
			{
				/* En cas d'appuis sur Entrée on recharge le niveau */
				pMap = ChargementNiveau(pMoteurRendu, pJoueur, infos.niveau, &etatNiveau);
				InitialisationPositions(images, pJoueur, infos.niveau);
				infos.compteurTemps = 0;
				infos.score = 1000;
				infos.bonus &= AUCUN_BONUS;
			}
		}

		/* Maintenant on va mettre à jour les coordonnées et faire l'affichage */

		tempsFPS = SDL_GetTicks();	//On prend le temps écoulé depuis l'initialisation de la SDL

		differenceFPS = tempsFPS - tempsAncienFPS;	//On soustrait

		if(differenceFPS > T_FPS)	//On regarde s'il s'est écoulé plus de T_FPS ms depuis le dernier affichage
		{
			MiseAJourCoordonnees(entrees, images, descente, pMap, pMoteurSon, pSons);

			/* Ensuite on effectue l'affichage */
			Affichage(pMoteurRendu, images, polices, descente, pMap, anim, &ajoutAnim);

			tempsAncienFPS = tempsFPS;
		}

		/* On regarde si le joueur a gagné le niveau ou s'il s'est fait tuer */
		etat = VerifierMortOUGagne(images, pMap, pMoteurSon, pSons);
		TraitementEtatDuNiveau(pMoteurRendu, pMoteurSon, pSons, &pMap, pJoueur, images, polices, anim, &entrees, &etat, &ajoutAnim, &control);

		if(infos.vies == 0)	//S'il ne lui reste plus de vie
		{
			if(BMusique)
			{
				FMOD_System_GetChannel(pMoteurSon, M_JEU, &pChannelEnCours);	//On arrête la musique du jeu
				FMOD_Channel_SetPaused(pChannelEnCours, true);

				FMOD_System_PlaySound(pMoteurSon, M_PERDU, pSons->music[M_PERDU], true, NULL);		//On joue la musique quand on a perdu
				FMOD_System_GetChannel(pMoteurSon, M_PERDU, &pChannelEnCours);
				FMOD_Channel_SetVolume(pChannelEnCours, Volume/100.0);
				FMOD_Channel_SetPaused(pChannelEnCours, false);
			}

			Perdu(pMoteurRendu, images, anim, pMap, polices, &ajoutAnim);	//On affiche l'écran de fin quand on a perdu

			control = JEU_FIN;
		}

		DetectionBonus(images, BOULE_MAGENTA, pMap);	//Si le joueur prend un bonus

		/* On s'occupe d'ajouter de la vie ou du score selon les bonus pris, on désactive les bonus ensuite et on joue un son */
		TraitementBonus(pMoteurSon, pSons);

		Chrono();	//Mise à jour du compteur de temps du niveau

		SDL_Delay(1);	//Petit délais d'au moins 1 ms pour éviter les bugs d'affichage et une surcharge du processeur
	}

	/* Si on a fini le jeu normalement (pas à cause d'une erreur) on libère la mémoire de la map */
	if (control != JEU_FIN_ERREUR_CHARGEMENT)
	{
		DestructionMap(pMap);
	}

	if (BMusique)
	{
		FMOD_System_GetChannel(pMoteurSon, M_PERDU, &pChannelEnCours);	//On arrête la musique quand on a perdu
		FMOD_Channel_SetPaused(pChannelEnCours, true);

		FMOD_System_GetChannel(pMoteurSon, M_JEU, &pChannelEnCours);	//On arrête la musique du jeu
		FMOD_Channel_SetPaused(pChannelEnCours, true);

		FMOD_System_GetChannel(pMoteurSon, M_GAGNE, &pChannelEnCours);	//On arrête la musique quand on a gagné
		FMOD_Channel_SetPaused(pChannelEnCours, true);

		FMOD_System_GetChannel(pMoteurSon, M_MENU, &pChannelEnCours);	//Et on remet celle du menu
		FMOD_Channel_SetPaused(pChannelEnCours, false);
	}

	pJoueur->niveau_max = infos.niveau;	// On stocke le niveau maximum que le joueur a pu atteindre

	return control;	//On renvoie le code d'erreur pour savoir pourquoi la boucle s'est terminée
}

int MiseAJourCoordonnees(ClavierSouris entrees, Sprite images[], unsigned char descente[], Map *pMap, FMOD_SYSTEM *pMoteurSon, Sons *pSons)
{
	static unsigned char sautEnCoursBleue=false, sautEnCoursVerte=false;
	static int v_gx=0; 	// Variables de vitesse

	/* Maintenant on va changer les positions en fonction des valeurs du tableau 'clavier' */
	DeplacementBoules(images, pMap, &entrees, pMoteurSon, pSons);

	Sauts(images, pMap, &entrees, &sautEnCoursBleue, &sautEnCoursVerte);

	DeplacementMissiles(images, descente);

	Gravite(images, pMap, sautEnCoursBleue, sautEnCoursVerte);

	return 0;
}

int Sauts(Sprite images[], Map *pMap, ClavierSouris *pEntrees, unsigned char *pSautEnCoursBleue, unsigned char *pSautEnCoursVerte)
{
	static unsigned char timerBleue=false, timerVerte=false;
	static unsigned int temps=0, tempsAncien=0;
	static int savePosBleue=0, savePosVerte=0, difference=0;
	Collision collision={COLL_NONE, 0};

	temps = SDL_GetTicks();

	difference = temps - tempsAncien;

	if (difference >= 250)
	{
		tempsAncien = temps;
		timerBleue = timerVerte = false;
	}
	else if (difference < 0)
	{
		tempsAncien = 0;
	}

	if((pEntrees->clavier[ESPACE] || *pSautEnCoursBleue) && !timerBleue)
	{
		if(*pSautEnCoursBleue)
			{
				savePosBleue = SautBleue(&images[BOULE_BLEUE].position[0], pSautEnCoursBleue);
				images[BOULE_BLEUE].position[0].y += savePosBleue;
			}
		else if (pMap->plan[Arrondir(images[BOULE_BLEUE].position[0].x + images[BOULE_BLEUE].position[0].w/2.0 - TailleBoule*0.3) /TailleBloc][Arrondir(images[BOULE_BLEUE].position[0].y + images[BOULE_BLEUE].position[0].h +1) /TailleBloc] != VIDE || pMap->plan[Arrondir(images[BOULE_BLEUE].position[0].x + images[BOULE_BLEUE].position[0].w/2.0 + TailleBoule*0.3) /TailleBloc][Arrondir(images[BOULE_BLEUE].position[0].y + images[BOULE_BLEUE].position[0].h +1) /TailleBloc] != VIDE)
			{
				savePosBleue = SautBleue(&images[BOULE_BLEUE].position[0], pSautEnCoursBleue);
				images[BOULE_BLEUE].position[0].y += savePosBleue;
			}

		CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

		if((collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
		{
			images[BOULE_BLEUE].position[0].y -= savePosBleue;
			savePosBleue=0;
			*pSautEnCoursBleue=false;
			timerBleue=true;
		}
	}

	if((pEntrees->clavier[ESPACE] || *pSautEnCoursVerte) && !timerVerte)
	{
		if(*pSautEnCoursVerte)
			{
				savePosVerte = SautVerte(&images[BOULE_VERTE].position[0], pSautEnCoursVerte);
				images[BOULE_VERTE].position[0].y -= savePosVerte;
			}
		else if (pMap->plan[Arrondir(images[BOULE_VERTE].position[0].x + images[BOULE_VERTE].position[0].w/2.0 - TailleBoule*0.3) /TailleBloc][Arrondir(images[BOULE_VERTE].position[0].y -1)/ TailleBloc] != VIDE || pMap->plan[Arrondir(images[BOULE_VERTE].position[0].x + images[BOULE_VERTE].position[0].w/2.0 + TailleBoule*0.3) /TailleBloc][Arrondir(images[BOULE_VERTE].position[0].y -1)/ TailleBloc] != VIDE)
			{
				savePosVerte = SautVerte(&images[BOULE_VERTE].position[0], pSautEnCoursVerte);
				images[BOULE_VERTE].position[0].y -= savePosVerte;
			}

		CollisionDetect(images, BOULE_VERTE, pMap, &collision);

		if((collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_DECOR))
		{
			images[BOULE_VERTE].position[0].y += savePosVerte;
			savePosVerte=0;
			*pSautEnCoursVerte=false;
			timerVerte=true;
		}
	}

	return 0;
}

int Gravite(Sprite images[], Map *pMap, unsigned char sautEnCoursBleue, unsigned char sautEnCoursVerte)
{
	int i=0, v_gx = Arrondir(0.0085*Hauteur);
	Collision collision={COLL_NONE, 0};

	if(sautEnCoursBleue == false)
	{
		images[BOULE_BLEUE].position[0].y += v_gx;

		CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

		if((collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE))
		{
			images[BOULE_BLEUE].position[0].y -= v_gx;

			for (i=0; i<v_gx; i++)
			{
				images[BOULE_BLEUE].position[0].y += 1;

				CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

				if((collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE))
				{
					images[BOULE_BLEUE].position[0].y -= 1;
				}
			}
		}
	}

	if(sautEnCoursVerte == false)
	{
		images[BOULE_VERTE].position[0].y -= v_gx;

		CollisionDetect(images, BOULE_VERTE, pMap, &collision);

		if((collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE))
		{
			images[BOULE_VERTE].position[0].y += v_gx;

			for (i=0; i<v_gx; i++)
			{
				images[BOULE_VERTE].position[0].y -= 1;

				CollisionDetect(images, BOULE_VERTE, pMap, &collision);

				if((collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE))
				{
					images[BOULE_VERTE].position[0].y += 1;
				}
			}
		}
	}

	return 0;
}

int DeplacementMissiles(Sprite images[], unsigned char descente[])
{
	int i=0;

	for(i=0; i<5; i++)
{
	if(images[MISSILE].position[i].y < (Hauteur-images[MISSILE].position[i].h) && descente[i] == 1)
	{
		images[MISSILE].position[i].y +=  Arrondir(0.003*Largeur);

		if (images[MISSILE].position[i].y >= (Hauteur-images[MISSILE].position[i].h))
		{
			descente[i] = 0;
		}
	}

	if((images[MISSILE].position[i].y > 0) && descente[i] == 0)
	{
		images[MISSILE].position[i].y -= Arrondir(0.003*Largeur);

		if (images[MISSILE].position[i].y <= 0)
		{
			descente[i] = 1;
		}
	}
}

for(i=5; i<10; i++)
{
	if(images[MISSILE].position[i].x < Largeur && descente[i] == 1)
	{
		images[MISSILE].position[i].x +=  Arrondir(0.003*Largeur);

		if (images[MISSILE].position[i].x >= Largeur)
		{
			descente[i] = 0;
		}
	}

	if((images[MISSILE].position[i].x > 0) && descente[i] == 0)
	{
		images[MISSILE].position[i].x -= Arrondir(0.003*Largeur);

		if (images[MISSILE].position[i].x - images[MISSILE].position[i].h <= 0)
		{
			descente[i] = 1;
		}
	}
}

return 0;
}

int DeplacementBoules(Sprite images[], Map *pMap, ClavierSouris *pEntrees, FMOD_SYSTEM *pMoteurSon, Sons *pSons)
{
	int i=0, enLecture;
	FMOD_CHANNEL *pChannelEnCours=NULL;
	Collision collision={COLL_NONE, 0};
	static int v_x1=0, v_y1=0, v_x2=0, v_x3=0;	// Variables de vitesse

	if(pEntrees->clavier[HAUT] && v_y1 > Arrondir(-0.002*Largeur))
	{
		v_y1--;
	}
	else if(pEntrees->clavier[BAS] && v_y1 < Arrondir(0.002*Largeur))
	{
		v_y1++;
	}
	else if(!(pEntrees->clavier[HAUT] || pEntrees->clavier[BAS]))
	{
		v_y1 = 0;
	}

	images[BOULE_MAGENTA].position[0].y += v_y1;

	CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);

	if ((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BORD_HAUT) || (collision.etatColl & COLL_BORD_BAS))
	{
		if((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE))
		{
			FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOULE+10, &pChannelEnCours);
			FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);
			if (!enLecture)
			{
				FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOULE+10, pSons->bruits[S_BOULE_BOULE], false, NULL);
			}
		}

		images[BOULE_MAGENTA].position[0].y -= v_y1;

		if (v_y1 > 0)
		{
			for (i=0; i<v_y1; i++)
			{
				images[BOULE_MAGENTA].position[0].y += 1;

				CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);

				if ((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BORD_HAUT) || (collision.etatColl & COLL_BORD_BAS))
				{
					images[BOULE_MAGENTA].position[0].y -= 1;
				}
			}
		}
		else
		{
			for (i=0; i>v_y1; i--)
			{
				images[BOULE_MAGENTA].position[0].y -= 1;

				CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);

				if ((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BORD_HAUT) || (collision.etatColl & COLL_BORD_BAS))
				{
					images[BOULE_MAGENTA].position[0].y += 1;
				}
			}
		}
	}

	if(pEntrees->clavier[GAUCHE] && v_x1 > Arrondir(-0.002*Largeur))
	{
		v_x1--;
	}
	else if(pEntrees->clavier[DROITE] && v_x1 < Arrondir(0.002*Largeur))
	{
		v_x1++;
	}
	else if(!(pEntrees->clavier[GAUCHE] || pEntrees->clavier[DROITE]))
	{
		v_x1 = 0;
	}

	images[BOULE_MAGENTA].position[0].x += v_x1;

	CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);

	if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
	{
		if((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE))
		{
			FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOULE+10, &pChannelEnCours);
			FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);
			if (!enLecture)
			{
				FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOULE+10, pSons->bruits[S_BOULE_BOULE], false, NULL);
			}
		}

		images[BOULE_MAGENTA].position[0].x -= v_x1;

		if (v_x1 > 0)
		{
			for (i=0; i<v_x1; i++)
			{
				images[BOULE_MAGENTA].position[0].x += 1;

				CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);

				if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
				{
					images[BOULE_MAGENTA].position[0].x -= 1;
				}
			}
		}
		else
		{
			for (i=0; i>v_x1; i--)
			{
				images[BOULE_MAGENTA].position[0].x -= 1;

				CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);

				if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
				{
					images[BOULE_MAGENTA].position[0].x += 1;
				}
			}
		}
	}

	if (infos.bonus & BONUS_VITESSE_BLEUE_FORT)
	{
		if(pEntrees->clavier[Q] && v_x2 > Arrondir(-0.002*Largeur*1.8))
		{
			v_x2--;
		}
		else if(pEntrees->clavier[D] && v_x2 < Arrondir(0.002*Largeur*1.8))
		{
			v_x2++;
		}
		else if(!(pEntrees->clavier[Q] || pEntrees->clavier[D]))
		{
			v_x2 = 0;
		}
	}
	else if (infos.bonus & BONUS_VITESSE_BLEUE_FAIBLE)
	{
		if(pEntrees->clavier[Q] && v_x2 > Arrondir(-0.002*Largeur*1.5))
		{
			v_x2--;
		}
		else if(pEntrees->clavier[D] && v_x2 < Arrondir(0.002*Largeur*1.5))
		{
			v_x2++;
		}
		else if(!(pEntrees->clavier[Q] || pEntrees->clavier[D]))
		{
			v_x2 = 0;
		}
	}
	else
	{
		if(pEntrees->clavier[Q] && v_x2 > Arrondir(-0.002*Largeur))
		{
			v_x2--;
		}
		else if(pEntrees->clavier[D] && v_x2 < Arrondir(0.002*Largeur))
		{
			v_x2++;
		}
		else if(!(pEntrees->clavier[Q] || pEntrees->clavier[D]))
		{
			v_x2 = 0;
		}
	}

	images[BOULE_BLEUE].position[0].x += v_x2;

	CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

	if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
	{
		if((collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE))
		{
			FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOULE+10, &pChannelEnCours);
			FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);
			if (!enLecture)
			{
				FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOULE+10, pSons->bruits[S_BOULE_BOULE], false, NULL);
			}
		}

		images[BOULE_BLEUE].position[0].x -= v_x2;

		if (v_x2 > 0)
		{
			for (i=0; i<v_x2; i++)
			{
				images[BOULE_BLEUE].position[0].x += 1;

				CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

				if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
				{
					images[BOULE_BLEUE].position[0].x -= 1;
				}
			}
		}
		else
		{
			for (i=0; i>v_x2; i--)
			{
				images[BOULE_BLEUE].position[0].x -= 1;

				CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

				if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
				{
					images[BOULE_BLEUE].position[0].x += 1;
				}
			}
		}
	}

	if (infos.bonus & BONUS_VITESSE_VERTE_FORT)
	{
		if(pEntrees->clavier[J] && v_x3 > Arrondir(-0.002*Largeur*1.8))
		{
			v_x3--;
		}
		else if(pEntrees->clavier[L] && v_x3 < Arrondir(0.002*Largeur*1.8))
		{
			v_x3++;
		}
		else if(!(pEntrees->clavier[J] || pEntrees->clavier[L]))
		{
			v_x3 = 0;
		}
	}
	else if (infos.bonus & BONUS_VITESSE_VERTE_FAIBLE)
	{
		if(pEntrees->clavier[J] && v_x3 > Arrondir(-0.002*Largeur*1.5))
		{
			v_x3--;
		}
		else if(pEntrees->clavier[L] && v_x3 < Arrondir(0.002*Largeur*1.5))
		{
			v_x3++;
		}
		else if(!(pEntrees->clavier[J] || pEntrees->clavier[L]))
		{
			v_x3 = 0;
		}
	}
	else
	{
		if(pEntrees->clavier[J] && v_x3 > Arrondir(-0.002*Largeur))
		{
			v_x3--;
		}
		else if(pEntrees->clavier[L] && v_x3 < Arrondir(0.002*Largeur))
		{
			v_x3++;
		}
		else if(!(pEntrees->clavier[J] || pEntrees->clavier[L]))
		{
			v_x3 = 0;
		}
	}

	images[BOULE_VERTE].position[0].x += v_x3;

	CollisionDetect(images, BOULE_VERTE, pMap, &collision);

	if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_DECOR))
	{
		if((collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE))
		{
			FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOULE+10, &pChannelEnCours);
			FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);
			if (!enLecture)
			{
				FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOULE+10, pSons->bruits[S_BOULE_BOULE], false, NULL);
			}
		}

		images[BOULE_VERTE].position[0].x -= v_x3;

		if (v_x3 > 0)
		{
			for (i=0; i<v_x3; i++)
			{
				images[BOULE_VERTE].position[0].x += 1;

				CollisionDetect(images, BOULE_VERTE, pMap, &collision);

				if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_DECOR))
				{
					images[BOULE_VERTE].position[0].x -= 1;
				}
			}
		}
		else
		{
			for (i=0; i>v_x3; i--)
			{
				images[BOULE_VERTE].position[0].x -= 1;

				CollisionDetect(images, BOULE_VERTE, pMap, &collision);

				if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_DECOR))
				{
					images[BOULE_VERTE].position[0].x += 1;
				}
			}
		}
	}

	return 0;
}

char VerifierMortOUGagne(Sprite images[], Map *pMap, FMOD_SYSTEM *pMoteurSon, Sons *pSons)
{
	FMOD_CHANNEL *musicEnCours = NULL;
	Collision collision={0, 0};
	int i=0;

	for(i=BOULE_BLEUE; i<=BOULE_VERTE; i++)
	{
		CollisionDetect(images, i, pMap, &collision);
		if(collision.etatColl & COLL_MISSILE)
		{
			FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOUM+10, pSons->bruits[S_BOULE_BOUM], true, NULL);
			FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOUM+10, &musicEnCours);
			FMOD_Channel_SetVolume(musicEnCours, Volume/100.0);
			FMOD_Channel_SetPaused(musicEnCours,  false);
			return collision.numMissile;
		}
	}

	CollisionDetect(images, BOULE_BLEUE, pMap, &collision);
	if (collision.etatColl & COLL_BORD_BAS)
	{
		return MORT_BORDURE;
	}

	CollisionDetect(images, BOULE_VERTE, pMap, &collision);
	if (collision.etatColl & COLL_BORD_HAUT)
	{
		return MORT_BORDURE;
	}

	CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

	if(collision.etatColl & COLL_VORTEX_BLEU)
	{
		CollisionDetect(images, BOULE_VERTE, pMap, &collision);

		if (collision.etatColl & COLL_VORTEX_VERT)
		{
			FMOD_System_PlaySound(pMoteurSon, S_SORTIE+10, pSons->bruits[S_SORTIE], true, NULL);
			FMOD_System_GetChannel(pMoteurSon, S_SORTIE+10, &musicEnCours);
			FMOD_Channel_SetVolume(musicEnCours, Volume/130.0);
			FMOD_Channel_SetPaused(musicEnCours,  false);
			return GAGNE;
		}
	}

	return RIEN;
}

int TraitementEtatDuNiveau(SDL_Renderer *pMoteurRendu, FMOD_SYSTEM *pMoteurSon, Sons *pSons, Map **ppMap, Joueur *pJoueur, Sprite images[], TTF_Font *polices[], Animation anim[], ClavierSouris *pEntrees, int *pEtat, unsigned char *pAjoutAnim, int *pControl)
{
	char missileTouche =-1;
	int etatNiveau;
	Map *pMapNew=NULL;
	FMOD_CHANNEL *pChannelEnCours=NULL;
	char chaine[100];	//Chaîne pour travailler


	if(*pEtat == MORT_BORDURE)
	{
		FMOD_System_PlaySound(pMoteurSon, S_TOMBE+10, pSons->bruits[S_TOMBE], false, NULL);

		infos.vies--;	//On enlève une vie
		infos.score -= 150;	//On enlève du score
		InitialisationPositions(images, pJoueur, infos.niveau);	//On recommence
		*ppMap = ChargementNiveau(pMoteurRendu, pJoueur, infos.niveau, &etatNiveau);
		infos.bonus &= AUCUN_BONUS;
	}
	else if (*pEtat >= 0)	//Mort par un missile
	{
		/* On enlève de la vie, du score et on lance l'animation */
		infos.vies--;
		infos.score -= 150;
		*pAjoutAnim = true;
		missileTouche = *pEtat;	//On récupère le numéro du missile qui a explosé

		/* On définit la taille et l'emplacement de l'animation */
		if (missileTouche >= 5)	//Missiles H
		{
			anim[ANIM_0].pos.x = Arrondir(images[MISSILE].position[missileTouche].x - (0.03 * Largeur) - images[MISSILE].position[missileTouche].h);
			anim[ANIM_0].pos.y = Arrondir(images[MISSILE].position[missileTouche].y - (0.03 * Largeur));
			anim[ANIM_0].pos.h = Arrondir(images[MISSILE].position[missileTouche].w + (0.06 * Largeur));
			anim[ANIM_0].pos.w = Arrondir(images[MISSILE].position[missileTouche].h + (0.03 * Largeur));
		}
		else	//Missile V
		{
			anim[ANIM_0].pos.x = Arrondir(images[MISSILE].position[missileTouche].x - (0.03 * Largeur));
			anim[ANIM_0].pos.y = Arrondir(images[MISSILE].position[missileTouche].y - (0.03 * Largeur));
			anim[ANIM_0].pos.h = Arrondir(images[MISSILE].position[missileTouche].h + (0.03 * Largeur));
			anim[ANIM_0].pos.w = Arrondir(images[MISSILE].position[missileTouche].w + (0.06 * Largeur));
		}

		InitialisationPositions(images, pJoueur, infos.niveau);	//On recommence
		*ppMap = ChargementNiveau(pMoteurRendu, pJoueur, infos.niveau, &etatNiveau);
		infos.bonus &= AUCUN_BONUS;
	}
	else if (*pEtat == GAGNE)	//Si on a gagné le niveau en cours
	{
		FMOD_System_PlaySound(pMoteurSon, S_SORTIE+10, pSons->bruits[S_SORTIE], false, NULL);

		infos.score += 350; //On gagne 350 en score
		infos.score -= Arrondir(0.75*infos.compteurTemps);	//On perd 75% du temps mis pour effectuer le niveau

/* On gagne 120 en score si on a toutes ses vies de départ, sinon ça dépend de combien il en reste par rapport à combien il y en avait au départ */
		infos.score += Arrondir(infos.vies*(120.0/(double)infos.viesInitiales));	
		infos.niveau++;	//Niveau suivant

		/* On charge le niveau suivant dans un autre pointeur */
		pMapNew = ChargementNiveau(pMoteurRendu, pJoueur, infos.niveau, &etatNiveau);

		sprintf(chaine, "Votre score est de: %ld, passage au niveau %d", infos.score, infos.niveau);
		MessageInformations(chaine, polices, pMoteurRendu, pEntrees);

		if (pMapNew == NULL)	//Si on a pas réussi (c'était le dernier niveau ou il y a eu un problème)
		{
			if(etatNiveau == CHARGEMENT_GAGNE)	//Si c'est parce que c'était le dernier niveau, on a gagné
			{
				if(BMusique)
				{
					FMOD_System_GetChannel(pMoteurSon, M_JEU, &pChannelEnCours);	//On arrête la musique de jeu
					FMOD_Channel_SetPaused(pChannelEnCours, true);

					FMOD_System_PlaySound(pMoteurSon, M_GAGNE, pSons->music[M_GAGNE], true, NULL);	//On lance celle de fin
					FMOD_System_GetChannel(pMoteurSon, M_GAGNE, &pChannelEnCours);
					FMOD_Channel_SetVolume(pChannelEnCours, Volume/100.0);
					FMOD_Channel_SetPaused(pChannelEnCours, false);
				}

				Gagne(pMoteurRendu, images, *ppMap, polices);	//On affiche l'écran de fin quand on a gagné

				*pControl = JEU_FIN_GAGNE;		//On sort de la boucle principale
			}
			else if(etatNiveau == CHARGEMENT_FICHIER_CORROMPU || etatNiveau == CHARGEMENT_ERREUR) //Si c'est une erreur de chargement
			{
				*pControl = JEU_FIN_ERREUR_CHARGEMENT;
			}
		}
		else	//On a chargé le niveau suivant
		{
			/* On ajoute le niveau précédent et son score à la liste */
			sprintf(chaine, "%d:%ld;", infos.niveau-1, infos.score);
			strcat(pJoueur->autre, chaine);

			DestructionMap(*ppMap);	//On libère la mémoire prise par l'ancienne structure Map
			*ppMap = pMapNew;	//On copie l'adresse de la nouvelle Map dans l'autre pointeur

			infos.compteurTemps = 0;	//On remet le compteur de temps à 0
			infos.score = 1000;	//On remet le score à 1000
			infos.vies = infos.viesInitiales;	//On remet la vie au maximum pour le prochain niveau

			InitialisationPositions(images, pJoueur, infos.niveau);	//On charge les positions

			*pEtat = RIEN;	//On réinitialise la valeur 'etat' pour que le nouveau niveau ne se termine pas instantanément

/* On désactive tous les bonus, équivalent à "infos.bonus = infos.bonus & AUCUN_BONUS". On fait un ET bit à bit avec AUCUN_BONUS = 00000000 */
			infos.bonus &= AUCUN_BONUS;
		}
	}

	return 0;
}

void TraitementBonus(FMOD_SYSTEM *pMoteurSon, Sons *pSons)
{
	FMOD_CHANNEL *pChannelEnCours=NULL;
	int enLecture=false;

	/* On vérifie si le son des bonus n'est pas déjà en lecture */
	FMOD_System_GetChannel(pMoteurSon, S_BONUS+10, &pChannelEnCours);
	FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);

	/* S'il ne l'est pas et que c'est un bonus instantané (pas de vitesse) alors on le joue */
	if((infos.bonus & (BONUS_VIE|BONUS_SCORE_FORT|BONUS_SCORE_FAIBLE)) && !enLecture)
	{
		FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], false, NULL);
	}

	/* Ensuite on s'occupe des bonus instantanés */
	if((infos.bonus & BONUS_VIE) && infos.vies < infos.viesInitiales)
	{
		infos.vies++;	//On ajoute de la vie si on est pas déjà au maximum
		infos.bonus &= ~BONUS_VIE;
	}
	else if (infos.bonus & BONUS_VIE)
	{
		infos.score += 75;	//Sinon on gagne un peu de score
		infos.bonus &= ~BONUS_VIE;
	}

	/* Bonus de score */
	if (infos.bonus & BONUS_SCORE_FAIBLE)
	{
		infos.score += 100;
		infos.bonus &= ~BONUS_SCORE_FAIBLE;
	}

	if (infos.bonus & BONUS_SCORE_FORT)
	{
		infos.score += 220;
		infos.bonus &= ~BONUS_SCORE_FORT;
	}
}

void Chrono()
{
	static unsigned int tempsChrono=0, tempsAncienChrono=0;
	static int difference=0;

	tempsChrono = SDL_GetTicks();	//On prend le temps

	difference = tempsChrono - tempsAncienChrono;	//On calcul la différence

	if(difference >= 1000)
	{
		infos.compteurTemps++;	// S'il s'est écoulé plus d'une seconde on augmente le compteur de 1
		tempsAncienChrono = tempsChrono;
	}
	else if(difference < 0)
	{
		tempsAncienChrono = 0;
	}
}

void CollisionDetect(Sprite images[], int indiceImage, Map *pMap, Collision *pCollision)
{
	pCollision->etatColl &= COLL_NONE;

	/* S'il y a une collision avec une autre image */
	pCollision->etatColl |= CollisionImage(images, indiceImage, pCollision);

	/* S'il y a une collision avec un des bords de la fenêtre */
	pCollision->etatColl |= CollisionBordure(images, indiceImage);

	/*S'il y a une collision avec le décor*/
	pCollision->etatColl |= CollisionDecor(images, indiceImage, pMap);
}

int Affichage(SDL_Renderer *pMoteurRendu, Sprite images[], TTF_Font *polices[], unsigned char descente[], Map* pMap, Animation anim[], unsigned char *pAjoutAnim)
{
	SDL_Rect posFond;

	posFond.h = (int)Hauteur;
	posFond.w = (int)Largeur;
	posFond.x = 0;
	posFond.y = 0;

	SDL_SetRenderDrawColor(pMoteurRendu, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(pMoteurRendu);      //On efface la fenêtre

	SDL_RenderCopy(pMoteurRendu, pMap->fond, NULL, &posFond);

	AffichageMap(pMoteurRendu, pMap);

	AffichageBonus(pMoteurRendu, pMap, images);

	AffichageImages(pMoteurRendu, images, anim, descente, pAjoutAnim);

	AffichageVies(pMoteurRendu, images);

	AffichageTextes(pMoteurRendu, polices, images[FOND_TEXTES].pTextures[0]);

	SDL_RenderPresent(pMoteurRendu);         //Mise à jour de l'écran

	return 0;
}

int AffichageMap(SDL_Renderer *pMoteurRendu, Map *pMap)
{
	int i, j;
	SDL_Rect pos;

	pos.h = TailleBloc;
	pos.w = TailleBloc;

	for(i=0; i< pMap->nbtiles_largeur_monde; i++)
		for(j=0; j< pMap->nbtiles_hauteur_monde; j++)
		{
			pos.x = i*TailleBloc;
			pos.y = j*TailleBloc;

			switch(pMap->plan[i][j])
			{
			case SOL_PETITE_PENTE_G:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PETITE_PENTE_G].src, &pos);
				break;

			case SOL_PETITE_PENTE_D:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PETITE_PENTE_D].src, &pos);
				break;

			case SOL_COIN_D_1:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_COIN_D_1].src, &pos);
				break;

			case SOL_COIN_D_2:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_COIN_D_2].src, &pos);
				break;

			case SOL_COIN_G_1:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_COIN_G_1].src, &pos);
				break;

			case SOL_COIN_G_2:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_COIN_G_2].src, &pos);
				break;

			case SOL_GRANDE_PENTE_D:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_GRANDE_PENTE_D].src, &pos);
				break;

			case SOL_GRANDE_PENTE_G:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_GRANDE_PENTE_G].src, &pos);
				break;

			case SOL_FIN_D:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_FIN_D].src, &pos);
				break;

			case SOL_FIN_G:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_FIN_G].src, &pos);
				break;

			case SOL_NORMAL:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_NORMAL].src, &pos);
				break;

			case SOL_PLEIN_1:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PLEIN_1].src, &pos);
				break;

			case SOL_PLEIN_2:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PLEIN_2].src, &pos);
				break;

			case SOL_PLEIN_3:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PLEIN_3].src, &pos);
				break;

			case SOL_PLEIN_4:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PLEIN_4].src, &pos);
				break;

			case SOL_PLEIN_5:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PLEIN_5].src, &pos);
				break;

			case SOL_PLEIN_UNI:
				SDL_RenderCopy(pMoteurRendu, pMap->tileset, &pMap->props[SOL_PLEIN_UNI].src, &pos);
				break;

			case VIDE:
				break;

			default:
				break;
			}
		}

	return 0;
}

int AffichageVies(SDL_Renderer *pMoteurRendu, Sprite images[])
{
	int i;

	switch(infos.viesInitiales)
	{
	case 1:
		switch(infos.vies)
		{
		case 1:
			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);
			break;

		case 0:
			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[0]);
			break;
		}

		break;

	case 2:
		switch(infos.vies)
		{
		case 2:
			for (i=0; i<2; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);
			}

			break;

		case 1:
			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[1]);
			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);
			break;

		case 0:
			for (i=0; i<2; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);
			}

			break;
		}

		break;

	case 3:
		switch(infos.vies)
		{
		case 3:
			for (i=0; i<3; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);
			}

			break;

		case 2:
			for (i=0; i<2; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);
			}

			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[2]);
			break;

		case 1:
			for (i=1; i<3; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);
			}

			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);
			break;

		case 0:
			for (i=0; i<3; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);
			}

			break;
		}

		break;

	case 4:
		switch(infos.vies)
		{
		case 4:
			for (i=0; i<4; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);
			}

			break;

		case 3:
			for (i=0; i<3; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);
			}

			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[3]);
			break;

		case 2:
			for (i=0; i<2; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);
			}

			for (i=2; i<4; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);
			}

			break;

		case 1:
			for (i=1; i<4; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);
			}

			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);
			break;

		case 0:
			for (i=0; i<4; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);
			}

			break;
		}

		break;

	case 5:
		switch(infos.vies)
		{
		case 5:
			for (i=0; i<5; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);
			}

			break;

		case 4:
			for (i=0; i<4; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);
			}

			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[4]);
			break;

		case 3:
			for (i=0; i<3; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);
			}

			for (i=3; i<5; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);
			}

			break;

		case 2:
			for (i=0; i<2; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);
			}

			for (i=2; i<5; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);
			}

			break;

		case 1:
			for (i=1; i<5; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);
			}

			SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);
			break;

		case 0:
			for (i=0; i<5; i++)
			{
				SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);
			}

			break;
		}

		break;
	}

	return 0;
}

int AffichageBonus(SDL_Renderer *pMoteurRendu, Map *pMap, Sprite images[])
{
	int i, j;

	for (i=0; i<pMap->nbtiles_largeur_monde; i++)
	{
		for (j=0; j<pMap->nbtiles_hauteur_monde; j++)
		{
			images[GEMMES].position[0].x = Arrondir((double)i* (double)TailleBloc + ((double)TailleBloc * 0.3)/2.0);
			images[GEMMES].position[0].y = Arrondir((double)j* (double)TailleBloc + ((double)TailleBloc * 0.3)/2.0);

			switch(pMap->planObjets[i][j])
			{
			case DIA_VERTCLAIR:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_VERTCLAIR], &images[GEMMES].position[0]);
				break;

			case DIA_NOIR:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_NOIR], &images[GEMMES].position[0]);
				break;

			case DIA_TURQUOISE:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_TURQUOISE], &images[GEMMES].position[0]);
				break;

			case DIA_JAUNE:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_JAUNE], &images[GEMMES].position[0]);
				break;

			case DIA_ROSE:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_ROSE], &images[GEMMES].position[0]);
				break;

			case DIA_ROUGE:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_ROUGE], &images[GEMMES].position[0]);
				break;

			case DIA_ORANGE:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_ORANGE], &images[GEMMES].position[0]);
				break;

			case DIA_BLEUCLAIR:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_BLEUCLAIR], &images[GEMMES].position[0]);
				break;

			case DIA_BLEUMARINE:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_BLEUMARINE], &images[GEMMES].position[0]);
				break;

			case DIA_VERT:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_VERT], &images[GEMMES].position[0]);
				break;

			case DIA_MARRON:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_MARRON], &images[GEMMES].position[0]);
				break;

			case DIA_BLEU:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_BLEU], &images[GEMMES].position[0]);
				break;

			case DIA_ROSECLAIR:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_ROSECLAIR], &images[GEMMES].position[0]);
				break;

			case DIA_VIOLET:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_VIOLET], &images[GEMMES].position[0]);
				break;

			case DIA_FUSHIA:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_FUSHIA], &images[GEMMES].position[0]);
				break;

			case DIA_MARRONCLAIR:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_MARRONCLAIR], &images[GEMMES].position[0]);
				break;

			case DIA_GRIS:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_GRIS], &images[GEMMES].position[0]);
				break;

			case DIA_BLEUFONCE:
				SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[DIA_BLEUFONCE], &images[GEMMES].position[0]);
				break;

			default:
				break;
			}
		}
	}

	return 0;
}

int AffichageImages(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], unsigned char descente[], unsigned char *pAjoutAnim)
{
	static unsigned int angleVortex=360;
	int i=0;
	static const SDL_Point pointOrigine={0, 0};

	angleVortex -= 10;

	if (angleVortex==0 || angleVortex>360)
	{
		angleVortex=360;
	}

	SDL_RenderCopyEx(pMoteurRendu, images[VORTEX_BLEU].pTextures[0], NULL, &images[VORTEX_BLEU].position[0], angleVortex, NULL, SDL_FLIP_NONE);
	SDL_RenderCopyEx(pMoteurRendu, images[VORTEX_VERT].pTextures[0], NULL, &images[VORTEX_VERT].position[0], angleVortex, NULL, SDL_FLIP_NONE);

	for(i=0; i < MISSILE; i++)
	{
		SDL_RenderCopy(pMoteurRendu, images[i].pTextures[0], NULL, &images[i].position[0]);        //Collage des surfaces
	}

	if(*pAjoutAnim)
	{
		if (LectureAnimation(pMoteurRendu, anim, ANIM_0) == -1)
		{
			*pAjoutAnim = false;
		}
	}
	else
	{
		for(i=0; i<5; i++)
		{
		if(descente[i])
		{
			SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i], 180, NULL, SDL_FLIP_NONE);	// On retourne le missile quand il descend
		}
		else
		{
			SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i], 0, NULL, SDL_FLIP_NONE);
		}
		}

		for(i=5; i<10; i++)
		{
		if(descente[i])
		{
			SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i], 90, &pointOrigine, SDL_FLIP_NONE);	// On retourne le missile quand il descend
		}
		else
		{
			SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i], 90, &pointOrigine, SDL_FLIP_VERTICAL);
		}
		}
	}

	return 0;
}

int AffichageTextes(SDL_Renderer *pMoteurRendu, TTF_Font *polices[], SDL_Texture *pTextureFond)
{
	char chaineTemps[50]="", chaineScore[50]="";
	SDL_Color blancOpaque= {255, 255, 255, 255};
	static SDL_Rect posTemps, posScore, posFond;
	SDL_Surface *pSurfTemps=NULL, *pSurfScore=NULL;
	SDL_Texture *pTextureTemps=NULL, *pTextureScore=NULL;

	posFond.x = posFond.y=0;
	posTemps.x = posScore.x = Arrondir(0.008*Largeur);
	posTemps.y=0;
	posScore.y = Arrondir(0.023*Largeur);

	sprintf(chaineTemps, "%d", infos.compteurTemps);
	sprintf(chaineScore, "%ld", infos.score);

	pSurfTemps = TTF_RenderText_Solid(polices[POLICE_ARIAL], chaineTemps, blancOpaque);
	TTF_SizeText(polices[POLICE_ARIAL], chaineTemps, &posTemps.w, &posTemps.h);
	posTemps.w = Arrondir(((double)posTemps.w/1280.0) * Largeur);
	posTemps.h = Arrondir(((double)posTemps.h/1280.0) * Largeur);

	pSurfScore = TTF_RenderText_Solid(polices[POLICE_ARIAL], chaineScore, blancOpaque);
	TTF_SizeText(polices[POLICE_ARIAL], chaineScore, &posScore.w, &posScore.h);
	posScore.w = Arrondir(((double)posScore.w/1280.0) * Largeur);
	posScore.h = Arrondir(((double)posScore.h/1280.0) * Largeur);

	posFond.h = posScore.y + posScore.h + Arrondir(0.008*Largeur);
	posFond.w = posScore.x + posScore.w + Arrondir(0.008*Largeur);

	pTextureTemps = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfTemps);
	pTextureScore = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfScore);

	SDL_RenderCopy(pMoteurRendu, pTextureFond, NULL, &posFond);
	SDL_RenderCopy(pMoteurRendu, pTextureTemps, NULL, &posTemps);
	SDL_RenderCopy(pMoteurRendu, pTextureScore, NULL, &posScore);

	SDL_FreeSurface(pSurfScore);
	SDL_FreeSurface(pSurfTemps);

	SDL_DestroyTexture(pTextureScore);
	SDL_DestroyTexture(pTextureTemps);

	return 0;
}

int InitialisationPositions(Sprite images[], Joueur *pJoueur, int level)
{
	int i, j, k;
	double nb;
	char *c = NULL;

	if (pJoueur->mode == MODE_CAMPAGNE || pJoueur->mode == MODE_PERSO)
	{
		FILE *pFichierNiveau = NULL;
		char ligne[20] = "";

		if(pJoueur->mode == MODE_CAMPAGNE)
		{
			pFichierNiveau = fopen("ressources/level.lvl", "r");
		}
		else if(pJoueur->mode == MODE_PERSO)
		{
			pFichierNiveau = fopen("ressources/levelUser.lvl", "r");
		}

		if (pFichierNiveau == NULL)
		{
			return -1;
		}

		for (i=0; i<level-1; i++)
		{
			fgets(ligne, 20, pFichierNiveau);

			do
			{
				fgets(ligne, 20, pFichierNiveau);
			}
			while (strcmp(ligne, "##--##\n") != 0);
		}

		k=0;

		while(k++, strcmp(ligne, "#missileV\n") != 0)
		{
			fgets(ligne, 20, pFichierNiveau);

			if(k>10)
			{
				return -1;
			}
		}


		for(i=0; i<5; i++)
		{
		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[MISSILE].position[i].x = Arrondir(nb*Largeur);

		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[MISSILE].position[i].y = Arrondir(nb*Hauteur);
		}

		fgets(ligne, 20, pFichierNiveau);

		for(i=5; i<10; i++)
		{
		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[MISSILE].position[i].x = Arrondir(nb*Largeur);

		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[MISSILE].position[i].y = Arrondir(nb*Hauteur);
		}

			if(fgets(ligne, 20, pFichierNiveau), strcmp(ligne, "#vortex\n") != 0)
			{
				return -1;
			}

			fgets(ligne, 20, pFichierNiveau);

			nb = strtod(ligne, NULL);

			if(nb == 0)
			{
				c = strstr(ligne, ".");
				*c = ',';
				nb = strtod(ligne, NULL);
			}

			images[VORTEX_BLEU].position[0].x = Arrondir(nb*Largeur);

			fgets(ligne, 20, pFichierNiveau);

			nb = strtod(ligne, NULL);

			if(nb == 0)
			{
				c = strstr(ligne, ".");
				*c = ',';
				nb = strtod(ligne, NULL);
			}

			images[VORTEX_BLEU].position[0].y = Arrondir(nb*Hauteur);

			fgets(ligne, 20, pFichierNiveau);

			nb = strtod(ligne, NULL);

			if(nb == 0)
			{
				c = strstr(ligne, ".");
				*c = ',';
				nb = strtod(ligne, NULL);
			}

			images[VORTEX_VERT].position[0].x = Arrondir(nb*Largeur);

			fgets(ligne, 20, pFichierNiveau);

			nb = strtod(ligne, NULL);

			if(nb == 0)
			{
				c = strstr(ligne, ".");
				*c = ',';
				nb = strtod(ligne, NULL);
			}

			images[VORTEX_VERT].position[0].y = Arrondir(nb*Hauteur);

		if (fgets(ligne, 20, pFichierNiveau), strcmp(ligne, "#boules\n") != 0)
		{
			return -1;
		}
			fgets(ligne, 20, pFichierNiveau);

			nb = strtod(ligne, NULL);

			if(nb == 0)
			{
				c = strstr(ligne, ".");
				*c = ',';
				nb = strtod(ligne, NULL);
			}

			images[BOULE_BLEUE].position[0].x = Arrondir(nb*Largeur);

			fgets(ligne, 20, pFichierNiveau);

			nb = strtod(ligne, NULL);

			if(nb == 0)
			{
				c = strstr(ligne, ".");
				*c = ',';
				nb = strtod(ligne, NULL);
			}

			images[BOULE_BLEUE].position[0].y = Arrondir(nb*Hauteur);

			fgets(ligne, 20, pFichierNiveau);

			nb = strtod(ligne, NULL);

			if(nb == 0)
			{
				c = strstr(ligne, ".");
				*c = ',';
				nb = strtod(ligne, NULL);
			}

			images[BOULE_MAGENTA].position[0].x = Arrondir(nb*Largeur);

			fgets(ligne, 20, pFichierNiveau);

			nb = strtod(ligne, NULL);

			if(nb == 0)
			{
				c = strstr(ligne, ".");
				*c = ',';
				nb = strtod(ligne, NULL);
			}

			images[BOULE_MAGENTA].position[0].y = Arrondir(nb*Hauteur);

		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[BOULE_VERTE].position[0].x = Arrondir(nb*Largeur);

		fgets(ligne, 20, pFichierNiveau);

		nb = strtod(ligne, NULL);

		if(nb == 0)
		{
			c = strstr(ligne, ".");
			*c = ',';
			nb = strtod(ligne, NULL);
		}

		images[BOULE_VERTE].position[0].y = Arrondir(nb*Hauteur);

		fclose(pFichierNiveau);
	}
	else if(pJoueur->mode == MODE_EDITEUR)
	{
		for(i=0; i<10; i++)
		{
			images[MISSILE].position[i].x =	images[MISSILE].position[i].y = -2000;
		}
		images[AJOUTER_MISSILE_H].position[0].x = Arrondir(0.900*Largeur);
		images[AJOUTER_MISSILE_H].position[0].y = Arrondir(0.055*Hauteur);
		images[AJOUTER_MISSILE_V].position[0].x = Arrondir(0.900*Largeur);
		images[AJOUTER_MISSILE_V].position[0].y = Arrondir(0.120*Hauteur);
		images[VORTEX_BLEU].position[0].x = Arrondir(0.925*Largeur);
		images[VORTEX_BLEU].position[0].y = Arrondir(images[AJOUTER_MISSILE_V].position[0].y + 0.08*Hauteur);
		images[VORTEX_VERT].position[0].x = Arrondir(0.925*Largeur);
		images[VORTEX_VERT].position[0].y = Arrondir(images[VORTEX_BLEU].position[0].y+ (double)TailleBoule*2.0 + 0.08*Hauteur);
		images[BOULE_BLEUE].position[0].x = Arrondir(0.925*Largeur);
		images[BOULE_BLEUE].position[0].y = Arrondir(images[VORTEX_VERT].position[0].y + (double)TailleBoule*2.0 + 0.08*Hauteur);
		images[BOULE_MAGENTA].position[0].x = Arrondir(0.925*Largeur);
		images[BOULE_MAGENTA].position[0].y = Arrondir(images[BOULE_BLEUE].position[0].y + (double)TailleBoule + 0.08*Hauteur);
		images[BOULE_VERTE].position[0].x = Arrondir(0.925*Largeur);
		images[BOULE_VERTE].position[0].y = Arrondir(images[BOULE_MAGENTA].position[0].y + (double)TailleBoule + 0.08*Hauteur);
	}

	for(i=0; i<infos.viesInitiales; i++)
	{
		images[VIE].position[i].x = Arrondir(Largeur - (double)(i+1)*0.05*Largeur);
		images[VIE].position[i].y = 0;
	}

	for (k=1, i=0; i<3; i++)
	{
		for (j=0; j<6; j++, k++)
		{
			images[GEMMES].position[k].x = 91*j;
			images[GEMMES].position[k].y = 78*i;
		}
	}

	//Initialisation des tailles
	images[BOULE_BLEUE].position[0].h = TailleBoule;
	images[BOULE_BLEUE].position[0].w = TailleBoule;
	images[BOULE_MAGENTA].position[0].h = TailleBoule;
	images[BOULE_MAGENTA].position[0].w = TailleBoule;
	images[BOULE_VERTE].position[0].h = TailleBoule;
	images[BOULE_VERTE].position[0].w = TailleBoule;

	for(i=0; i<11; i++)
	{
		images[MISSILE].position[i].h = TailleMissileH;
		images[MISSILE].position[i].w = TailleMissileW;
	}

	images[VORTEX_BLEU].position[0].w = images[VORTEX_BLEU].position[0].h = images[VORTEX_VERT].position[0].w = images[VORTEX_VERT].position[0].h = Arrondir(TailleBloc*1.7);
	images[GEMMES].position[0].w = images[GEMMES].position[0].h = Arrondir((double)TailleBloc - ((double)TailleBloc * 0.3));
	images[CURSEUR].position[0].w = Arrondir(0.02*Largeur);
	images[CURSEUR].position[0].h = Arrondir(0.027*Largeur);

	for (i=0; i<infos.viesInitiales; i++)
	{
		images[VIE].position[i].h = images[VIE].position[i].w = Arrondir(0.04*Largeur);
	}

	for (i=0; i<18; i++)
	{
		images[GEMMES].position[i+1].h = 78;
		images[GEMMES].position[i+1].w = 91;
	}

	return 0;
}

int SautBleue(SDL_Rect *pPosition, unsigned char *pSautEnCours)
{
	static int positionRelative=0, positionRelativeAncienne=0, positionFinale=0;
	static int positionInitiale=0, temps=0;
	double vitesse_initiale=0.0;

	if(infos.bonus & BONUS_SAUT_BLEUE_FORT)
	{
		vitesse_initiale=1.4;
	}
	else if(infos.bonus & BONUS_SAUT_BLEUE_FAIBLE)
	{
		vitesse_initiale=1.25;
	}
	else
	{
		vitesse_initiale=1.0;
	}

	if(*pSautEnCours == false)
	{
		positionInitiale = pPosition->y;
		temps = positionRelativeAncienne = 0;
		*pSautEnCours = true;
	}

	positionRelative = Arrondir(((double)((vitesse_initiale * temps)-((_G_ * temps * temps)/2000)) /600.0)*Hauteur);

	positionFinale = positionRelative - positionRelativeAncienne;
	positionRelativeAncienne = positionRelative;

	if(pPosition->y - positionRelative > positionInitiale)
	{
		temps = 0;
		*pSautEnCours = false;
		positionInitiale = 0;
		return 0;
	}

	temps += 5;

	return (-positionFinale);
}

int SautVerte(SDL_Rect *pPosition, unsigned char *pSautEnCours)
{
	static int positionRelative, positionRelativeAncienne=0, positionFinale;
	static int positionInitiale=0, temps=0;
	double vitesse_initiale;

	if(infos.bonus & BONUS_SAUT_VERTE_FORT)
	{
		vitesse_initiale=1.4;
	}
	else if(infos.bonus & BONUS_SAUT_VERTE_FAIBLE)
	{
		vitesse_initiale=1.25;
	}
	else
	{
		vitesse_initiale=1.0;
	}

	if(*pSautEnCours == false)
	{
		positionInitiale = pPosition->y;
		temps = positionRelativeAncienne =0;
		*pSautEnCours = true;
	}

	positionRelative = Arrondir(((double)((vitesse_initiale * temps)-((_G_ * temps * temps)/2000)) /600.0)*Hauteur);

	positionFinale = positionRelative - positionRelativeAncienne;
	positionRelativeAncienne = positionRelative;

	if(pPosition->y + positionRelative < positionInitiale)
	{
		temps = 0;
		*pSautEnCours = false;
		positionInitiale = 0;
		return 0;
	}

	temps += 5;

	return (-positionFinale);
}

int Arrondir(double nombre)
{
	if (nombre >= 0)
	{
		return (int)(nombre+0.5);
	}
	else
	{
		return (int)(nombre-0.5);
	}
}

unsigned int CollisionBordure (Sprite images[], int indiceImage)
{
	/* S'il y a une collision avec un des bords de la fenêtre */

	if(images[indiceImage].position[0].y <= 0)
	{
		return  COLL_BORD_HAUT;
	}

	if(images[indiceImage].position[0].y + images[indiceImage].position[0].h >= Hauteur)
	{
		return COLL_BORD_BAS;
	}

	if(images[indiceImage].position[0].x <= 0)
	{
		return COLL_BORD_GAUCHE;
	}

	if(images[indiceImage].position[0].x + images[indiceImage].position[0].w >= Largeur)
	{
		return COLL_BORD_DROIT;
	}

	return COLL_NONE;
}

unsigned int CollisionImage (Sprite images[], int indiceImage, Collision *pCollision)
{
	int i=0;

	if (indiceImage < VORTEX_BLEU)
	{
	for(i=0; i < VORTEX_BLEU; i++)
	{
		if (i == indiceImage)
		{
			continue;
		}

		if((images[indiceImage].position[0].y + images[indiceImage].position[0].h -1> images[i].position[0].y) && (images[indiceImage].position[0].y < images[i].position[0].y + images[i].position[0].h -1))
		{
			if(((images[indiceImage].position[0].x + images[indiceImage].position[0].w -1) > images[i].position[0].x) && (images[indiceImage].position[0].x < images[i].position[0].x + images[i].position[0].w -1))
			{
				switch(i)		//S'il y a une collision avec une image se trouvant avant dans le tableau
				{
				case BOULE_BLEUE:
					return COLL_BOULE_BLEUE;

				case BOULE_MAGENTA:
					return COLL_BOULE_MAGENTA;

				case BOULE_VERTE:
					return COLL_BOULE_VERTE;
				}
			}
		}
	}
	}

	for(i=0; i < 5; i++)
	{
		if((images[indiceImage].position[0].y + images[indiceImage].position[0].h -1> images[MISSILE].position[i].y) && (images[indiceImage].position[0].y < images[MISSILE].position[i].y + images[MISSILE].position[i].h -1))
		{
			if(((images[indiceImage].position[0].x + images[indiceImage].position[0].w -1) > images[MISSILE].position[i].x) && (images[indiceImage].position[0].x < images[MISSILE].position[i].x + images[MISSILE].position[i].w -1))
			{
				pCollision->numMissile = i;
				return COLL_MISSILE;
			}
		}
	}

	for(i=5; i < 10; i++)
	{
		if((images[indiceImage].position[0].y + images[indiceImage].position[0].h -1> images[MISSILE].position[i].y) && (images[indiceImage].position[0].y < images[MISSILE].position[i].y -1 + images[MISSILE].position[i].w))
		{
			if(((images[indiceImage].position[0].x + images[indiceImage].position[0].w -1) > images[MISSILE].position[i].x - images[MISSILE].position[i].h) && (images[indiceImage].position[0].x < images[MISSILE].position[i].x -1))
			{
				pCollision->numMissile = i;
				return COLL_MISSILE;
			}
		}
	}

	if (indiceImage != VORTEX_BLEU)
	{
	if((images[indiceImage].position[0].y + images[indiceImage].position[0].h > images[VORTEX_BLEU].position[0].y) && (images[indiceImage].position[0].y < images[VORTEX_BLEU].position[0].y + images[VORTEX_BLEU].position[0].h))
	{
		if(((images[indiceImage].position[0].x + images[indiceImage].position[0].w) > images[VORTEX_BLEU].position[0].x) && (images[indiceImage].position[0].x < images[VORTEX_BLEU].position[0].x + images[VORTEX_BLEU].position[0].w))
		{
			return COLL_VORTEX_BLEU;	//S'il y a une collision avec le centre du vortex
		}
	}
	}

	if (indiceImage != VORTEX_VERT)
	{
	if((images[indiceImage].position[0].y + images[indiceImage].position[0].h > images[VORTEX_VERT].position[0].y) && (images[indiceImage].position[0].y < images[VORTEX_VERT].position[0].y + images[VORTEX_VERT].position[0].h))
	{
		if(((images[indiceImage].position[0].x + images[indiceImage].position[0].w) > images[VORTEX_VERT].position[0].x) && (images[indiceImage].position[0].x < images[VORTEX_VERT].position[0].x + images[VORTEX_VERT].position[0].w))
		{
			return COLL_VORTEX_VERT;		//S'il y a une collision avec le centre du vortex
		}
	}
	}

	return COLL_NONE;
}

unsigned int CollisionDecor (Sprite images[], int indiceImage, Map* pMap)
{
	if((pMap->plan[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)]) != VIDE)
	{
		return COLL_DECOR;
	}

	if((pMap->plan[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)]) != VIDE)
	{
		return COLL_DECOR;
	}

	if((pMap->plan[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)]) != VIDE)
	{
		return COLL_DECOR;
	}

	if((pMap->plan[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)]) != VIDE)
	{
		return COLL_DECOR;
	}

	return COLL_NONE;
}

void DetectionBonus (Sprite images[], int indiceImage, Map* pMap)
{
	int i;

	for (i=DIA_VERTCLAIR; i<=DIA_BLEUFONCE; i++)
	{
		if((pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)]) == i)
		{
			switch(i)
			{
			case DIA_VERTCLAIR:
				infos.bonus |= BONUS_VITESSE_VERTE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_VERT:
				infos.bonus |= BONUS_VITESSE_VERTE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRONCLAIR:
				infos.bonus |= BONUS_SAUT_VERTE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRON:
				infos.bonus |= BONUS_SAUT_VERTE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_JAUNE:
				infos.bonus |= BONUS_SCORE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ORANGE:
				infos.bonus |= BONUS_SCORE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ROUGE:
				infos.bonus |= BONUS_VIE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUCLAIR:
				infos.bonus |= BONUS_SAUT_BLEUE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUFONCE:
				infos.bonus |= BONUS_SAUT_BLEUE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEU:
				infos.bonus |= BONUS_VITESSE_BLEUE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUMARINE:
				infos.bonus |= BONUS_VITESSE_BLEUE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;
			}
		}

		if((pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)]) == i)
		{
			switch(i)
			{
			case DIA_VERTCLAIR:
				infos.bonus |= BONUS_VITESSE_VERTE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_VERT:
				infos.bonus |= BONUS_VITESSE_VERTE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRONCLAIR:
				infos.bonus |= BONUS_SAUT_VERTE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRON:
				infos.bonus |= BONUS_SAUT_VERTE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_JAUNE:
				infos.bonus |= BONUS_SCORE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ORANGE:
				infos.bonus |= BONUS_SCORE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ROUGE:
				infos.bonus |= BONUS_VIE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUCLAIR:
				infos.bonus |= BONUS_SAUT_BLEUE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUFONCE:
				infos.bonus |= BONUS_SAUT_BLEUE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEU:
				infos.bonus |= BONUS_VITESSE_BLEUE_FAIBLE;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUMARINE:
				infos.bonus |= BONUS_VITESSE_BLEUE_FORT;
				pMap->planObjets[((images[indiceImage].position[0].x + images[indiceImage].position[0].w) / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;
			}
		}

		if((pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)]) == i)
		{
			switch(i)
			{
			case DIA_VERTCLAIR:
				infos.bonus |= BONUS_VITESSE_VERTE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = false;
				break;

			case DIA_VERT:
				infos.bonus |= BONUS_VITESSE_VERTE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRONCLAIR:
				infos.bonus |= BONUS_SAUT_VERTE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRON:
				infos.bonus |= BONUS_SAUT_VERTE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_JAUNE:
				infos.bonus |= BONUS_SCORE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ORANGE:
				infos.bonus |= BONUS_SCORE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ROUGE:
				infos.bonus |= BONUS_VIE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUCLAIR:
				infos.bonus |= BONUS_SAUT_BLEUE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUFONCE:
				infos.bonus |= BONUS_SAUT_BLEUE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEU:
				infos.bonus |= BONUS_VITESSE_BLEUE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUMARINE:
				infos.bonus |= BONUS_VITESSE_BLEUE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][((images[indiceImage].position[0].y + images[indiceImage].position[0].h) / TailleBloc)] = AUCUN_BONUS;
				break;
			}
		}

		if((pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)]) == i)
		{
			switch(i)
			{
			case DIA_VERTCLAIR:
				infos.bonus |= BONUS_VITESSE_VERTE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_VERT:
				infos.bonus |= BONUS_VITESSE_VERTE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRONCLAIR:
				infos.bonus |= BONUS_SAUT_VERTE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_MARRON:
				infos.bonus |= BONUS_SAUT_VERTE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_JAUNE:
				infos.bonus |= BONUS_SCORE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ORANGE:
				infos.bonus |= BONUS_SCORE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_ROUGE:
				infos.bonus |= BONUS_VIE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUCLAIR:
				infos.bonus |= BONUS_SAUT_BLEUE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUFONCE:
				infos.bonus |= BONUS_SAUT_BLEUE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEU:
				infos.bonus |= BONUS_VITESSE_BLEUE_FAIBLE;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;

			case DIA_BLEUMARINE:
				infos.bonus |= BONUS_VITESSE_BLEUE_FORT;
				pMap->planObjets[(images[indiceImage].position[0].x / TailleBloc)][(images[indiceImage].position[0].y / TailleBloc)] = AUCUN_BONUS;
				break;
			}
		}
	}
}

int Perdu(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], Map* pMap, TTF_Font *polices[], unsigned char *pAjoutAnim)
{
	SDL_Surface *psFondPerdu = NULL;
	SDL_Texture *pFondPerdu = NULL;
	Texte information;

	ClavierSouris entrees;
	SDL_Color color = {255, 255, 255, 255};
	int rmask, gmask, bmask, amask, differenceFPS=0, i=0;
	unsigned int tempsFPS=0, tempsAncienFPS=0;

	sprintf(information.chaines[0], "PERDU");
	sprintf(information.chaines[1], "Niveau: %d", infos.niveau);
	sprintf(information.chaines[2], "Temps mis pour ce niveau: %d", infos.compteurTemps);
	sprintf(information.chaines[3], "Score: %ld", infos.score);

	EntreesZero(&entrees);

	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;

	psFondPerdu = SDL_CreateRGBSurface(0 , (int)Largeur, (int)Hauteur, 32, rmask, gmask, bmask, amask);
	SDL_FillRect(psFondPerdu, NULL, SDL_MapRGBA(psFondPerdu->format, 200, 125, 75, 128));
	pFondPerdu = SDL_CreateTextureFromSurface(pMoteurRendu, psFondPerdu);

	SDL_FreeSurface(psFondPerdu);

	for (i=0; i<4; i++)
	{
		information.surface = TTF_RenderText_Blended(polices[POLICE_SNICKY_GRAND], information.chaines[i], color);
		TTF_SizeText(polices[POLICE_SNICKY_GRAND], information.chaines[i], &information.positions[i].w, &information.positions[i].h);
		information.positions[i].w = Arrondir(((double)information.positions[i].w / 1280.0) * Largeur);
		information.positions[i].h = Arrondir(((double)information.positions[i].h / 1280.0) * Largeur);

		information.positions[i].x = Arrondir((Largeur/2.0) - ((double)information.positions[i].w/2.0));
		information.positions[i].y = Arrondir((0.063*Largeur + 0.141*Largeur*(double)i));

		information.pTextures[i] = SDL_CreateTextureFromSurface(pMoteurRendu, information.surface);

		SDL_FreeSurface(information.surface);
	}

	while(!entrees.clavier[ECHAP] && !entrees.clavier[ESPACE] && !entrees.clavier[ENTREE] && !entrees.fermeture)
	{
		GestionEvenements(&entrees);

		tempsFPS = SDL_GetTicks();

		differenceFPS = tempsFPS - tempsAncienFPS;

		if (differenceFPS > T_FPS)
		{
			PerduAffichage(pMoteurRendu, images, anim, pFondPerdu, pMap, &information, pAjoutAnim);
			tempsAncienFPS = tempsFPS;
		}
	}

	entrees.clavier[ECHAP] = entrees.clavier[ESPACE] = entrees.clavier[ENTREE] = false;

	MessageInformations("Oh non, les boules ont péri, brisées par les missiles !", polices, pMoteurRendu, &entrees);

	entrees.clavier[ECHAP] = entrees.clavier[ENTREE] = false;

	return 0;
}

int PerduAffichage(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], SDL_Texture *pFondPerdu, Map *pMap, Texte *pInformation, unsigned char *pAjoutAnim)
{
	static SDL_Rect pos, posFond;
	int i=0;

	pos.h = pos.w = TailleBloc;

	posFond.h = (int)Hauteur;
	posFond.w = (int)Largeur;
	posFond.x = posFond.y = 0;

	SDL_SetRenderDrawColor(pMoteurRendu, 65, 118, 150, 255); //Remplissage en bleu pour le fond
	SDL_RenderClear(pMoteurRendu);

	SDL_RenderCopy(pMoteurRendu, pMap->fond, NULL, &posFond);

	SDL_RenderCopy(pMoteurRendu, images[VORTEX_BLEU].pTextures[0], NULL, &images[VORTEX_BLEU].position[0]);        //Collage des surfaces
	SDL_RenderCopy(pMoteurRendu, images[VORTEX_VERT].pTextures[0], NULL, &images[VORTEX_VERT].position[0]);

	AffichageMap(pMoteurRendu, pMap);

	AffichageBonus(pMoteurRendu, pMap, images);

	AffichageVies(pMoteurRendu, images);

	if(*pAjoutAnim)
	{
		if (LectureAnimation(pMoteurRendu, anim, ANIM_0) == -1)
		{
			*pAjoutAnim = false;
		}
	}
	else
	{
		SDL_RenderCopy(pMoteurRendu, pFondPerdu, NULL, &posFond);

		for(i=0; i<4; i++)
		{
			SDL_RenderCopy(pMoteurRendu, pInformation->pTextures[i], NULL, &pInformation->positions[i]);
		}
	}

	SDL_RenderPresent(pMoteurRendu);         //Mise à jour de l'écran

	return 0;
}

int LectureAnimation(SDL_Renderer *pMoteurRendu, Animation anim[], int animNB)
{
	static int temps=0, tempsAncien=0, k=0;

	temps = SDL_GetTicks();

	if (anim[animNB].img[k] != NULL && temps - tempsAncien > T_ANIM)
	{
		SDL_RenderCopy(pMoteurRendu, anim[animNB].img[k], NULL, &anim[animNB].pos);
		k++;
		tempsAncien = temps;
	}
	else if (anim[animNB].img[k] != NULL)
	{
		SDL_RenderCopy(pMoteurRendu, anim[animNB].img[k], NULL, &anim[animNB].pos);
	}
	else
	{
		k=0;
		temps = tempsAncien = 0;
		return -1;
	}

	return 0;
}

int Gagne(SDL_Renderer *pMoteurRendu, Sprite images[], Map *pMap, TTF_Font *polices[])
{
	SDL_Surface *pSurfFondGagne = NULL;
	SDL_Texture *pTextureFondGagne = NULL;

	Texte information;

	ClavierSouris entrees;
	SDL_Color color = {255, 255, 255, 255};
	int rmask, gmask, bmask, amask, i=0, differenceFPS=0;
	unsigned int tempsFPS=0, tempsAncienFPS=0;

	sprintf(information.chaines[0], "GAGNÉ");
	sprintf(information.chaines[1], "Niveau: %d", infos.niveau -1);
	sprintf(information.chaines[2], "Temps mis pour ce niveau: %d", infos.compteurTemps);
	sprintf(information.chaines[3], "Score: %ld", infos.score);
	sprintf(information.chaines[4], "Vies: %d sur %d", infos.vies, infos.viesInitiales);

	EntreesZero(&entrees);

	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;

	pSurfFondGagne = SDL_CreateRGBSurface(0 , (int)Largeur, (int)Hauteur, 32, rmask, gmask, bmask, amask);
	SDL_FillRect(pSurfFondGagne, NULL, SDL_MapRGBA(pSurfFondGagne->format, 200, 125, 75, 128));
	pTextureFondGagne = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfFondGagne);

	SDL_FreeSurface(pSurfFondGagne);

	for (i=0; i<5; i++)
	{
		information.surface = TTF_RenderText_Blended(polices[POLICE_SNICKY_GRAND], information.chaines[i], color);
		TTF_SizeText(polices[POLICE_SNICKY_GRAND], information.chaines[i], &information.positions[i].w, &information.positions[i].h);
		information.positions[i].w = Arrondir(((double)information.positions[i].w / 1280.0) * Largeur);
		information.positions[i].h = Arrondir(((double)information.positions[i].h / 1280.0) * Largeur);

		information.positions[i].x = ((Largeur/2.0) - (information.positions[i].w/2));
		information.positions[i].y = Arrondir(0.05*Largeur + 0.125*Largeur*(double)i);

		information.pTextures[i] = SDL_CreateTextureFromSurface(pMoteurRendu, information.surface);

		SDL_FreeSurface(information.surface);
	}

	while(!entrees.clavier[ECHAP] && !entrees.clavier[ESPACE] && !entrees.clavier[ENTREE] && !entrees.fermeture)
	{
		GestionEvenements(&entrees);

		tempsFPS = SDL_GetTicks();

		differenceFPS = tempsFPS - tempsAncienFPS;

		if (differenceFPS > T_FPS)
		{
			GagneAffichage(pMoteurRendu, images, pTextureFondGagne, pMap, &information);
			tempsAncienFPS = tempsFPS;
		}
	}

	entrees.clavier[ECHAP] = entrees.clavier[ENTREE] = entrees.clavier[ESPACE] = false;

	MessageInformations("Bravo! Vous avez sauvé les boules.", polices, pMoteurRendu, &entrees);

	entrees.clavier[ECHAP] = entrees.clavier[ENTREE] = entrees.clavier[ESPACE] = false;

	return 0;
}

int GagneAffichage(SDL_Renderer *pMoteurRendu, Sprite images[], SDL_Texture *pTextureFondGagne, Map *pMap, Texte *pInformation)
{
	static SDL_Rect pos, posFond;
	int i=0;

	pos.h = pos.w = TailleBloc;

	posFond.h = Hauteur;
	posFond.w = Largeur;
	posFond.x = posFond.y = 0;

	SDL_SetRenderDrawColor(pMoteurRendu, 0, 0, 0, SDL_ALPHA_OPAQUE); //Remplissage en bleu pour le fond
	SDL_RenderClear(pMoteurRendu);

	SDL_RenderCopy(pMoteurRendu, pMap->fond, NULL, &posFond);

	SDL_RenderCopy(pMoteurRendu, images[VORTEX_BLEU].pTextures[0], NULL, &images[VORTEX_BLEU].position[0]);        //Collage des surfaces
	SDL_RenderCopy(pMoteurRendu, images[VORTEX_VERT].pTextures[0], NULL, &images[VORTEX_VERT].position[0]);

	AffichageMap(pMoteurRendu, pMap);

	AffichageBonus(pMoteurRendu, pMap, images);

	AffichageVies(pMoteurRendu, images);

	SDL_RenderCopy(pMoteurRendu, pTextureFondGagne, NULL, &posFond);

	for(i=0; i<5; i++)
	{
		SDL_RenderCopy(pMoteurRendu, pInformation->pTextures[i], NULL, &pInformation->positions[i]);
	}

	SDL_RenderPresent(pMoteurRendu);         //Mise à jour de l'écran

	return 0;
}
//Fin du fichier jeu.c
