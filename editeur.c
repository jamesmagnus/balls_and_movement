/*
Projet-ISN

Fichier: editeur.c

Contenu: Fonctions relatives à l'éditeur de niveau.

Actions: C'est ici que se trouve les fonctions qui gèrent l'éditeur de niveau.

Bibliothèques utilisées: Bibliothèques standards, SDL, SDL_image, SDL_ttf, FMOD, GTK

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <gtk/gtk.h>
#include <fmod.h>
#include <SDL2_gfxPrimitives.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "jeu.h"
#include "IOediteur.h"
#include "editeur.h"

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;		//Lien vers les variables globales déclarées dans main.c
extern double Volume, Hauteur, Largeur;


int Editeur (SDL_Renderer *pMoteurRendu, Sprite images[], FMOD_SYSTEM *pMoteurSon, Sons *pSons, TTF_Font *polices[], Joueur *pJoueur)
{
	Map *pMap = NULL;	//Pointeur sur une structure Map
	int continuer=true, etat=0, objetPris=AUCUN_BONUS;
	unsigned int temps=0, tempsAncien=0;	//Les temps sont forcément positifs
	ClavierSouris entrees;	//Structure pour gérer les entrées clavier et souris
	FMOD_CHANNEL *pChannelEnCours=NULL;
	SDL_Event evenementPoubelle;	//Une structure pour prendre les évènements dans la file d'attente sans les utiliser (purge de la file des évènements)

	EntreesZero(&entrees);	//On initialise la structure des entrées

	/* On définit les positions et les tailles des images, on charge une map vierge et on crée les boutons pour ajouter des missiles */
	pMap = InitialisationEditeur(pMoteurRendu, images, polices, pJoueur);

	while(SDL_PollEvent(&evenementPoubelle));	//On purge la file d'attente des évènements (si des touches ont été appuyées pendant le chargement)

	while(continuer)	//Tant que continuer ne vaut pas 0
	{
		temps = SDL_GetTicks();	//On prend le temps écoulé depuis le lancement de la SDL

		/* On effectue l'affichage s'il s'est écoulé T_FPS depuis le dernier affichage, pour ne pas saturer le GPU */
		if(temps - tempsAncien > T_FPS)
		{
			AffichageEditeur(pMoteurRendu, images, pMap, entrees, objetPris);
			tempsAncien = temps;	//On garde le temps actuel dans une autre variable pour pouvoir faire la soustraction
		}

		/* On met à jour la map de l'éditeur et on récupère l'éventuel objet qui est sélectionné et qui suit le curseur pour l'affichage */
		objetPris = MiseAJourMap(pMap, images, &entrees, pMoteurSon, pSons);

		/* On met à jour la structure qui donne l'état du clavier et de la souris en lisant le prochain évènement en file d'attente */
		GestionEvenements(&entrees);

		/* Si il y a eu appuis sur la touche Echap ou demande de fermeture (croix ou Alt + F4), on affiche un message de confirmation */
		if (entrees.clavier[ECHAP] || entrees.fermeture)
		{
			entrees.clavier[ECHAP] = false;

			/* Si on appuit sur entrée alors on quitte vraiment l'éditeur */
			if(MessageInformations("Voulez-vous vraiment retourner au menu sans sauvegarder ?", polices, pMoteurRendu, &entrees) == 1)
			{
				continuer = false;
			}
		}

		/* Si on appuit sur S on vérifie la validité de la map et on la sauvegarde */
		if (entrees.clavier[S])
		{
			etat = VerifierEmplacements(images, pMap);	//On vérifie

			/* Si un missile et une boule sont sur la même trajectoire */
			if(etat == -1)
			{
				if(BSons)	//On joue un son d'alarme en boucle
				{
					FMOD_System_PlaySound(pMoteurSon, S_ALARME+10, pSons->bruits[S_ALARME], false, NULL);
					FMOD_System_GetChannel(pMoteurSon, S_ALARME+10, &pChannelEnCours);
					FMOD_Channel_SetLoopCount(pChannelEnCours, -1);
				}

				/* On affiche un message */
				MessageInformations("Le missile et une des boules sont sur la même trajectoire.", polices, pMoteurRendu, &entrees);

				if(BSons)	// On arrête l'alarme
				{
					FMOD_Channel_Stop(pChannelEnCours);
				}
			}
			else if (etat == -2)	//Si une boule est dans le sol dès le départ
			{
				if(BSons)	//Alarme
				{
					FMOD_System_PlaySound(pMoteurSon, S_ALARME+10, pSons->bruits[S_ALARME], false, NULL);
					FMOD_System_GetChannel(pMoteurSon, S_ALARME+10, &pChannelEnCours);
					FMOD_Channel_SetLoopCount(pChannelEnCours, 10);
				}

				/* Message */
				MessageInformations("Une des boules est en collision.", polices, pMoteurRendu, &entrees);

				if(BSons)	//Fin de l'alarme
				{
					FMOD_Channel_Stop(pChannelEnCours);
				}
			}
			else if (etat == -3)	//Si le vortex bleu est dans le sol
			{
				if(BSons)	//Alarme
				{
					FMOD_System_PlaySound(pMoteurSon, S_ALARME+10, pSons->bruits[S_ALARME], false, NULL);
					FMOD_System_GetChannel(pMoteurSon, S_ALARME+10, &pChannelEnCours);
					FMOD_Channel_SetLoopCount(pChannelEnCours, 10);
				}

				/* Message */
				MessageInformations("Le vortex bleu est en collision.", polices, pMoteurRendu, &entrees);

				if(BSons)	//Fin de l'alarme
				{
					FMOD_Channel_Stop(pChannelEnCours);
				}
			}
			else if (etat == -4)	//Si le vortex vert est dans le sol
			{
				if(BSons)	//Alarme
				{
					FMOD_System_PlaySound(pMoteurSon, S_ALARME+10, pSons->bruits[S_ALARME], false, NULL);
					FMOD_System_GetChannel(pMoteurSon, S_ALARME+10, &pChannelEnCours);
					FMOD_Channel_SetLoopCount(pChannelEnCours, 10);
				}

				/* Message */
				MessageInformations("Le vortex vert est en collision.", polices, pMoteurRendu, &entrees);

				if(BSons)	// Fin de l'alarme
				{
					FMOD_Channel_Stop(pChannelEnCours);
				}
			}
			else	//Sinon si la map est correcte
			{
				if(BSons)	//Petit son de sauvegarde
				{
					FMOD_System_PlaySound(pMoteurSon, S_SAVE+10, pSons->bruits[S_SAVE], true, NULL);
					FMOD_System_GetChannel(pMoteurSon, S_SAVE+10, &pChannelEnCours);
					FMOD_Channel_SetVolume(pChannelEnCours, Volume/110.0);
					FMOD_Channel_SetPaused(pChannelEnCours, false);
				}

				/* On enregistre dans le fichier level.lvl */
				if(SauvegardeNiveau(pMap, images) == -1)
				{
					/* Message erreur */
					MessageInformations("Erreur lors de la sauvegarde !", polices, pMoteurRendu, &entrees);
				}
				else
				{
					/* Message */
					MessageInformations("Niveau sauvegardé !", polices, pMoteurRendu, &entrees);
				}

				continuer = false;	// On retourne au menu
			}
		}

		SDL_Delay(1);	//Délais d'au moins 1 ms pour empêcher des bugs et une surcharge du processeur pour les autres tâches que l'affichage
	}

	/* On détruit la map après la boucle de l'éditeur pour éviter les fuites de mémoire */
	DestructionMap(pMap);

	/* On remet la musique du menu */
	if(BMusique)
	{
		FMOD_System_GetChannel(pMoteurSon, M_JEU, &pChannelEnCours);
		FMOD_Channel_SetPaused(pChannelEnCours, true);
		FMOD_System_GetChannel(pMoteurSon, M_MENU, &pChannelEnCours);
		FMOD_Channel_SetPaused(pChannelEnCours, false);
	}

	return 0;	// On retourne au menu
}


Map* InitialisationEditeur (SDL_Renderer *pMoteurRendu, Sprite images[], TTF_Font *polices[], Joueur *pJoueur)
{
	Map* pMap = NULL;	//Pointeur vers une structure Map
	SDL_Surface *pSurfMissileH, *pSurfMissileV;		//Pointeurs vers des surfaces
	SDL_Color blancOpaque = {255, 255, 255, SDL_ALPHA_OPAQUE};

	/* On crée une nouvelle surface avec le texte spécifié, on met à la bonne taille, on la transforme en texture et on libère cette surface */
	pSurfMissileH = TTF_RenderText_Blended(polices[SNICKY], "Ajout missile H", blancOpaque);
	images[AJOUTER_MISSILE_H].pTextures[0] = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfMissileH);
	TTF_SizeText(polices[SNICKY], "Ajout missile H", &images[AJOUTER_MISSILE_H].position[0].w, &images[AJOUTER_MISSILE_H].position[0].h);
	SDL_FreeSurface(pSurfMissileH);

	/* On crée une nouvelle surface avec le texte spécifié, on met à la bonne taille, on la transforme en texture et on libère cette surface */
	pSurfMissileV = TTF_RenderText_Blended(polices[SNICKY], "Ajout missile V", blancOpaque);
	images[AJOUTER_MISSILE_V].pTextures[0] = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfMissileV);
	TTF_SizeText(polices[SNICKY], "Ajout missile V", &images[AJOUTER_MISSILE_V].position[0].w, &images[AJOUTER_MISSILE_V].position[0].h);
	SDL_FreeSurface(pSurfMissileV);

	pMap = ChargementNiveau(pMoteurRendu, pJoueur, -1);	//On charge une map en mode 'éditeur' donc vierge

	InitialisationPositions(images, pJoueur, -1);		//On initialise les positions et les tailles des images en mode 'éditeur'

	return pMap;	//On renvoie l'adresse de la map
}


int AffichageEditeur(SDL_Renderer *pMoteurRendu, Sprite images[], Map* pMap, ClavierSouris entrees, int objetPris)
{
	/* Cette fonction s'occupe de l'affichage */

	int i=0;               //Variables de comptage
	SDL_Point pointOrigine={0, 0};	//Coordonnées d'un point pour faire les rotations

	/* On efface l'écran avec du noir */
	SDL_SetRenderDrawColor(pMoteurRendu, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(pMoteurRendu);

	/* On dessine un fond bleu sur toute la surface dans laquelle on peut travailler (si l'écran n'est pas au format 16/10, il y aura des bandes en haut et en bas ou sur les côtés, elles resteront noires) */
	boxRGBA(pMoteurRendu, 0, 0, Largeur, Hauteur, 85, 120, 180, SDL_ALPHA_OPAQUE);

	/* On affiche les blocs de la map */
	AffichageMap(pMoteurRendu, pMap);

	/* On affiche les bonus */
	AffichageBonus(pMoteurRendu, pMap, images);

	/* Puis les images de boules, de vortex et les boutons "Ajout de missiles" */
	SDL_RenderCopy(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[0]);
	SDL_RenderCopy(pMoteurRendu, images[VORTEX_BLEU].pTextures[0], NULL, &images[VORTEX_BLEU].position[0]);
	SDL_RenderCopy(pMoteurRendu, images[VORTEX_VERT].pTextures[0], NULL, &images[VORTEX_VERT].position[0]);
	SDL_RenderCopy(pMoteurRendu, images[BOULE_BLEUE].pTextures[0], NULL, &images[BOULE_BLEUE].position[0]);
	SDL_RenderCopy(pMoteurRendu, images[BOULE_MAGENTA].pTextures[0], NULL, &images[BOULE_MAGENTA].position[0]);
	SDL_RenderCopy(pMoteurRendu, images[BOULE_VERTE].pTextures[0], NULL, &images[BOULE_VERTE].position[0]);
	SDL_RenderCopy(pMoteurRendu, images[AJOUTER_MISSILE_V].pTextures[0], NULL, &images[AJOUTER_MISSILE_V].position[0]);
	SDL_RenderCopy(pMoteurRendu, images[AJOUTER_MISSILE_H].pTextures[0], NULL, &images[AJOUTER_MISSILE_H].position[0]);

	/* On affiche ensuite les missiles */
	for(i=0; i<5; i++)
	{
		SDL_RenderCopy(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i]);
	}
	/* Les 5 derniers sont horizontales donc rotation de 90° */
	for (i=5; i<10; i++)
	{
		SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i], 90, &pointOrigine, SDL_FLIP_NONE);
	}

	/* On affiche les bonus dans leur cadre de sélection */
	AffichageBonusEditeur(pMoteurRendu, images);

	/* Le cadre en question */
	AffichageBoxEditeur(pMoteurRendu, &entrees);

	/* On affiche le curseur et l'éventuel objet sélectionné qui suit ce dernier */
	AffichageObjetCurseurEditeur(pMoteurRendu, &entrees, images, objetPris);


	SDL_RenderPresent(pMoteurRendu);    //Mise à jour de l'écran

	return 0; //Affichage terminé
}


int MiseAJourMap (Map *pMap, Sprite images[], ClavierSouris *pEntrees, FMOD_SYSTEM *pMoteurSon, Sons *pSons)
{
	/* On retient ce qui est sélectionné même lorsque la fonction est rappelée plusieurs fois, ces variables sont statiques */
	static int diaPris=AUCUN_BONUS, missilePris=0;

	/* On déplace l'objets qui est sélectionné par exemple une boule */
	DeplacementObjetEditeur(pMoteurSon, pSons, images, pEntrees);

	/* On place ou on retire un bloc ou un bonus selon l'état des entrées */
	MiseAjourMapEtBonusEditeur(pEntrees, pMoteurSon, pSons, pMap, &diaPris, &missilePris);

	/* On ajoute un missile si on clique sur un bouton ou si on en a un de sélectionner et que l'on clique sur la map */
	MiseAJourMapMissileEditeur(pMoteurSon, pSons, pEntrees, images, &missilePris);

	/* On change le type de chaque bloc de la map selon ceux qui se trouve au-dessus */
	AmeliorationMap(pMap);

	/* S'il y a un bonus de sélectionné on renvoie lequel */
	if(diaPris != AUCUN_BONUS)
	{
		return diaPris;
	}
	else	// Sinon on renvoie 100 ou 101 pour un éventuel missile H ou V, ou 0 s'il n 'y a aucun objet sélectionné
	{
		return missilePris;
	}
}


int VerifierEmplacements(Sprite images[], Map *pMap)
{
	/* Cette fonction vérifie les collisions et les alignements interdits, comme une boule dans le sol ou sur la trajectoire d'un missile */

	int i=0, j=0;
	Collision collision={COLL_NONE, 0};	//Structure pour retenir les collisions et l'indice du missile incriminé s'il y en a un

	/* On teste chaque boule une par une */
	for(i=BOULE_BLEUE; i<=BOULE_VERTE; i++)
	{
		/* On regarde si chaque missile V va toucher la boule que l'on teste  au cours de son déplacement */
		for(j=0; j<5; j++)
		{
			if(images[i].position[0].x + images[i].position[0].w > images[MISSILE].position[j].x && images[i].position[0].x < images[MISSILE].position[j].x + images[MISSILE].position[j].w)
		{
			return -1;
		}
		}

		/* On fait la même chose avec chaque missile H */
		for(j=5; j<10; j++)
		{
			if(images[i].position[0].y + images[i].position[0].h > images[MISSILE].position[j].y && images[i].position[0].y < images[MISSILE].position[j].y + images[MISSILE].position[j].w)
		{
			return -1;
		}
		}

	/* On vérifie si la boule que l'on teste est en collision avec quoi que ce soit */
	CollisionDetect(images, i, pMap, &collision);

		if(collision.etatColl & ~COLL_NONE)	//Si un seul bit est à 1 c'est qu'il y a une collision
		{
			return -2;
		}
	}

	/* On vérifie ensuite les collisions pour les 2 vortex */
		CollisionDetect(images, VORTEX_BLEU, pMap, &collision);

		if(collision.etatColl & ~COLL_NONE)
		{
			return -3;
		}

		CollisionDetect(images, VORTEX_VERT, pMap, &collision);

		if(collision.etatColl & ~COLL_NONE)
		{
			return -4;
		}

	return 0;	//On renvoie 0 s'il n'y a aucun problème
}


void DeplacementObjetEditeur(FMOD_SYSTEM *pMoteurSon, Sons *pSons, Sprite images[], ClavierSouris *pEntrees)
{
	int i=0;
	static int deplacement = -1;	//On retient dans une variable statique si on a sélectionné un objet et lequel (-1 pas d'objet)

	/* On s'occupe de chaque boule et des 2 vortexs l'un après l'autre, on saute les missiles */
	for (i=BOULE_BLEUE; i<=VORTEX_VERT; i++)
	{
		if(i == MISSILE)
		{
			continue;	//On passe directement au prochain tour de boucle
		}

		/* On regarde si la souris est sur l'image et si on appuie sur la molette OU si un déplacement est en cours */
		if((pEntrees->souris.touches[C_MOLETTE] && pEntrees->souris.position.x > images[i].position[0].x && pEntrees->souris.position.x < (images[i].position[0].x + images[i].position[0].w) && pEntrees->souris.position.y > images[i].position[0].y && pEntrees->souris.position.y < (images[i].position[0].y + images[i].position[0].h)) || deplacement != -1)
		{
			/* Si il n'y avait pas de déplacement en cours c'est une sélection d'objet */
			if (deplacement == -1)
			{
				FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
				deplacement = i;	//On retient l'indice de l'image que l'on est en train de sélectionner
				pEntrees->souris.touches[C_MOLETTE] = false;	//On remet à zéro le clic de molette
			}


			/* La position de l'image suit alors celle de la souris mais elle est centrée par rapport au curseur */
			images[deplacement].position[0].x = pEntrees->souris.position.x - images[deplacement].position[0].w /2;
			images[deplacement].position[0].y = pEntrees->souris.position.y - images[deplacement].position[0].h /2;

	/* Si un déplacement est en cours, un objet est déjà sélectionné, deplacement != -1 ; si on fait un clic de molette on dépose alors l'image là où elle est */
			if (deplacement != -1 && pEntrees->souris.touches[C_MOLETTE])
			{
				FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);
				deplacement = -1;
				pEntrees->souris.touches[C_MOLETTE] = false;
			}
		}
	}
}


void MiseAjourMapEtBonusEditeur(ClavierSouris *pEntrees, FMOD_SYSTEM *pMoteurSon, Sons *pSons, Map *pMap, int *pDiaPris, int *pMissilePris)
{
	SDL_Rect pos;	//Variable de position et de taille (x; y; h; w)
	int i=0, j=0, k=0;	//Compteurs

	/* On prend les coordonnées de la souris */
	pos.x = pEntrees->souris.position.x;
	pos.y = pEntrees->souris.position.y;

	/* On fait bien attention à ce que la souris soit dans la zone et non sur les bandes noires qui pourraient apparaître sur les côtés, sinon plantage assuré */
	if(pos.x > 0 && pos.x < Largeur && pos.y > 0 && pos.y < Hauteur)
	{
		/* Si on fait un clic gauche sans appuyer sur SHIFT et qu'il n'y a aucun objet sélectionné on met un bloc */
		if(pEntrees->souris.touches[C_GAUCHE] && !pEntrees->clavier[SHIFT] && *pDiaPris == AUCUN_BONUS && !(*pMissilePris))
		{
			if(BSons)	//Bruit de clic
			{
				FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);
			}

			pMap->plan[pos.x /TailleBloc][pos.y /TailleBloc] = SOL_NORMAL; //On place un bloc de sol normal dans la case sur laquelle se trouve la souris
		}
		else if(pEntrees->souris.touches[C_DROIT] && !pEntrees->clavier[SHIFT] && *pDiaPris == AUCUN_BONUS && !(*pMissilePris))
		{
			pMap->plan[pos.x /TailleBloc][pos.y /TailleBloc] = VIDE;	//Si c'est un clic droit dans les mêmes conditions on efface la case
		}
		else if (*pDiaPris && pEntrees->souris.touches[C_GAUCHE])	//S'il y a un bonus sélectionné et que l'on fait un clic gauche on dépose le bonus
		{
			if(BSons)	//Bruit de clic
			{
				FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);
			}

			pMap->planObjets[pos.x /TailleBloc][pos.y /TailleBloc] = *pDiaPris;	//On dépose le bonus sélectionné
			*pDiaPris = AUCUN_BONUS;	//Plus rien n'est sélectionné
			pEntrees->souris.touches[C_GAUCHE] = false;	//Clic gauche désactivé pour éviter que des blocs ne soit posés dans la foulé
		}
		else if (pEntrees->souris.touches[C_DROIT] && pEntrees->clavier[SHIFT])	//Si on appuie sur SHIFT et que l'on fait un clic droit alors on efface le bonus
		{
			pMap->planObjets[pos.x /TailleBloc][pos.y /TailleBloc] = AUCUN_BONUS;	//Plus de bonus ici
		}

		/* Maintenant on s'occupe de sélectionner un bonus dans le cadre des bonus */
	if(pEntrees->clavier[SHIFT] && pEntrees->souris.touches[C_GAUCHE])	//Si on appuie sur SHIFT et que l'on fait un clic gauche
	{
		/* On va balayer tous les bonus (3 lignes avec i et 9 bonus par ligne avec j), k donne le rang du bonus (de 1 jusqu'à 18) */
		for (k=1, i=0; i<2; i++)
		{
			for (j=0; j<9; j++, k++)
			{
				/* On regarde si la souris est sur le bonus actuel */
				if(pEntrees->souris.position.x > 0.075*Largeur*(double)j +0.081*Largeur && pEntrees->souris.position.x < 0.075*Largeur*(double)(j+1) +0.081*Largeur && pEntrees->souris.position.y > 0.075*Hauteur*(double)i +0.83*Hauteur && pEntrees->souris.position.y < 0.075*Largeur*(double)(i+1) +0.83*Hauteur)
				{
					FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
					*pDiaPris = k;	//Le bonus sélectionné est retenu
					pEntrees->souris.touches[C_GAUCHE] = false;	//Le clic gauche est désactivé pour éviter que le bonus ne soit déposé instantanément
				}
			}
		}
	}
	}
}


void MiseAJourMapMissileEditeur(FMOD_SYSTEM *pMoteurSon, Sons *pSons, ClavierSouris *pEntrees, Sprite images[], int *pMissilePris)
{
	int i=0, j=0;	//Compteurs

	/* Si on clique sur le bouton Ajouter Missile H */
	if(pEntrees->souris.touches[C_MOLETTE] && pEntrees->souris.position.x > images[AJOUTER_MISSILE_H].position[0].x && pEntrees->souris.position.x < images[AJOUTER_MISSILE_H].position[0].x + images[AJOUTER_MISSILE_H].position[0].w)
	{
		if(pEntrees->souris.position.y > images[AJOUTER_MISSILE_H].position[0].y && pEntrees->souris.position.y < images[AJOUTER_MISSILE_H].position[0].y + images[AJOUTER_MISSILE_H].position[0].h)
		{
			FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
			*pMissilePris=101;	//On sélectionne un missile H
			pEntrees->souris.touches[C_MOLETTE] = false;	//On désactive le clic de molette pour éviter que le missile ne soit déposé instantanément
		}
	}

	/* On fait la même chose avec le bouton pour les missiles V */
	if(pEntrees->souris.touches[C_MOLETTE] && pEntrees->souris.position.x > images[AJOUTER_MISSILE_V].position[0].x && pEntrees->souris.position.x < images[AJOUTER_MISSILE_V].position[0].x + images[AJOUTER_MISSILE_V].position[0].w)
	{
		if(pEntrees->souris.position.y > images[AJOUTER_MISSILE_V].position[0].y && pEntrees->souris.position.y < images[AJOUTER_MISSILE_V].position[0].y + images[AJOUTER_MISSILE_V].position[0].h)
		{
			FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
			*pMissilePris=100;	//On sélectionne un missile V
			pEntrees->souris.touches[C_MOLETTE] = false;	//On désactive le clic de molette
		}
	}

	/* Si un missile est sélectionné et que l'on fait un clic de molette on va le déposer*/
	if (*pMissilePris && pEntrees->souris.touches[C_MOLETTE])
	{
		/* On regarde si c'est un missile H ou V */
		switch(*pMissilePris)
		{
		case 100:	//Missile V
			for(i=0; i<5; i++)	//On regarde si parmis les missiles V il en reste qui ne sont pas placés
			{
				if(images[MISSILE].position[i].x < 0)
				{
					j=1;	//Si oui on met j à 1 et on quitte la boucle tout de suite 'break;'
					break;
				}
				else
				{
					j=-1;	//Sinon on met -1 dans j pour dire que ce missile V est déjà placé
				}
			}

			if (j != -1)	//Si on a trouvé un missile V pas encore placé on le place sinon il ne se passe rien
			{
				images[MISSILE].position[i].x = pEntrees->souris.position.x;
				images[MISSILE].position[i].y = pEntrees->souris.position.y;
			}

			pEntrees->souris.touches[C_MOLETTE] = false;	//On désactive le clic de molette
			break;

		case 101:	//Missile H
			for(i=5; i<10; i++)	//On vérifie s'ils ne sont pas tous placés
			{
				if(images[MISSILE].position[i].x < 0)
				{
					j=1;	//On en a trouvé un pas encore placé
					break;
				}
				else
				{
					j=-1;
				}
			}

			if (j != -1)	//On place le missile H s'ils ne sont pas tous déjà placés
			{
				images[MISSILE].position[i].x = pEntrees->souris.position.x;
				images[MISSILE].position[i].y = pEntrees->souris.position.y;
			}

			pEntrees->souris.touches[C_MOLETTE] = false;	//On désactive le clic de molette
			break;
		}

		FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);	//Bruit de clic
		*pMissilePris = 0;	//Plus de missile sélectionné
	}
}


void AmeliorationMap(Map *pMap)
{
	/* Cette fonction change automatiquement le type de bloc de la map en fonction de ceux au-dessus, elle évite ainsi que plusieurs couches d'herbe ne soient superposées */

	int i=0, j=0;	//Compteurs

	/* On parcourt la map colonne par colonne */
	for (i=0; i< pMap->nbtiles_largeur_monde; i++)
	{
		/* Dans chaque colonne on descent bloc par bloc */
		for (j=0; j< pMap->nbtiles_hauteur_monde; j++)
		{
			/* On vérifie si il y a du sol avec de l'herbe et en dessous autre chose que du vide alors on met le sol de transition qui doit être juste en dessous de l'herbe, on vérifie également que le bloc avec de l'herbe n'est pas tout en bas de la colonne sinon plantage */
			if(pMap->plan[i][j] == SOL_NORMAL && pMap->plan[i][j+1] != VIDE && j < (pMap->nbtiles_hauteur_monde-1))
			{
				pMap->plan[i][j+1] = SOL_PLEIN_3;

				/* On vérifie ensuite si en dessous de ce second bloc il y autre chose que du vide et on place le bloc de sol uniforme, on vérifie également que le bloc avec de l'herbe n'est pas l'avant dernier de la colonne */
				if (pMap->plan[i][j+2] != VIDE && j < (pMap->nbtiles_hauteur_monde-2))
				{
					pMap->plan[i][j+2] = SOL_PLEIN_UNI;
				}
			}

			/* Si on a un bloc non vide et qu'au dessus c'est un sol uniforme alors ce bloc est aussi un sol uniforme */
			if (pMap->plan[i][j] != VIDE && pMap->plan[i][j-1] == SOL_PLEIN_UNI)
			{
				pMap->plan[i][j] = SOL_PLEIN_UNI;
			}

			/* Si on a un bloc non vide et qu'au dessus le bloc est vide alors ce bloc est un bloc d'herbe */
			if (pMap->plan[i][j] != VIDE && pMap->plan[i][j-1] == VIDE)
			{
				pMap->plan[i][j] = SOL_NORMAL;
			}
		}
	}
}


void AffichageBonusEditeur(SDL_Renderer *pMoteurRendu, Sprite images[])
{
	/* Cette fonction affiche les bonus dans leur cadre de sélection */

	int i=0, j=0, k=0;	//Compteurs

	/* On parcourt les 18 bonus grâce à une double boucle, k compte les bonus de 1 à 18 */
	for (k=1, i=0; i<2; i++)
	{
		for (j=0; j<9; j++, k++)
		{
			/* On colle les bonus à une position qui dépend de leur ordre sur l'image d'origine */
			images[GEMMES].position[0].x = Arrondir(0.075*Largeur*j + 0.081*Largeur);
			images[GEMMES].position[0].y = Arrondir(0.075*Hauteur*i + 0.83*Hauteur);
			SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[k], &images[GEMMES].position[0]);
		}
	}
}


void AffichageBoxEditeur(SDL_Renderer *pMoteurRendu, ClavierSouris *pEntrees)
{
	/* Cette fonction affiche les cadres où sont placés les objets au départ, on en dessine 4 décalés d'un pixel pour plus d'épaisseur */

	int i=0;	//Compteur

	if(pEntrees->clavier[SHIFT])	//Si on appuie sur SHIFT on dessine des cadres plus clairs pour les bonus et des normaux pour les images
	{
		for (i=0; i<=4; i++)
		{
			roundedRectangleRGBA(pMoteurRendu, (0.89*Largeur +i), i, (Largeur-10-i), (Hauteur-10-i), 10, 255, 255, 255, 128);
			roundedRectangleRGBA(pMoteurRendu, (10+i), (0.8*Hauteur +i), (0.8*Largeur -i), (Hauteur-10-i), 10, 255, 255, 255, 200);
		}
	}
	else	//Sinon des cadres normaux pour tous
	{
		for (i=0; i<=4; i++)
		{
			roundedRectangleRGBA(pMoteurRendu, (0.89*Largeur +i), i, (Largeur-10-i), (Hauteur-10-i), 10, 255, 255, 255, 128);
			roundedRectangleRGBA(pMoteurRendu, (10+i), (0.8*Hauteur +i), (0.8*Largeur -i), (Hauteur-10-i), 10, 255, 255, 255, 100);
		}
	}
}


void AffichageObjetCurseurEditeur(SDL_Renderer *pMoteurRendu, ClavierSouris *pEntrees, Sprite images[], int objetPris)
{
	SDL_Point pointOrigine={0, 0};	//Coordonnées du point d'origine de l'image pour faire la rotation
	int angleCurseur=335;

	/* On définit les coordonnées des différentes images que l'on pourrait avoir à coller sur celle de la souris */
	images[MISSILE].position[10].x = images[GEMMES].position[0].x = images[CURSEUR].position[0].x = pEntrees->souris.position.x;
	images[MISSILE].position[10].y = images[GEMMES].position[0].y = images[CURSEUR].position[0].y = pEntrees->souris.position.y;

	/* Si il y a un bonus mais pas de missile on colle le bonus près de la souris */
	if(objetPris != AUCUN_BONUS && objetPris != 100 && objetPris != 101)
	{
		SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[objetPris], &images[GEMMES].position[0]);
	}
	else if (objetPris == 100)	//Sinon s'il y a un missile V on colle un missile V près de la souris
	{
		SDL_RenderCopy(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[10]);
	}
	else if (objetPris == 101)	//Sinon s'il y a un missile H on colle un missile H près de la souris
	{
		SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[10], 90, &pointOrigine, SDL_FLIP_NONE);
	}

	/* Enfin on colle le curseur */
	SDL_RenderCopyEx(pMoteurRendu, images[CURSEUR].pTextures[0], NULL, &images[CURSEUR].position[0], angleCurseur, NULL, SDL_FLIP_NONE);
}

//Fin du fichier editeur.c
