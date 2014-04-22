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

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;
extern double Volume, Hauteur, Largeur;


int Editeur (SDL_Renderer *pMoteurRendu, sprite images[], FMOD_SYSTEM *pMoteurSon, Sons *pSons, TTF_Font *polices[])
{
	Map *pMap = NULL;
	int continuer=true, temps=0, tempsAncien=0, objetPris=AUCUN_BONUS;
	ClavierSouris entrees;
	FMOD_CHANNEL *pChannelEnCours=NULL;
	SDL_Event evenementPoubelle;

	EntreesZero(&entrees);

	pMap = InitialisationEditeur(pMoteurRendu, images, polices);

	while(SDL_PollEvent(&evenementPoubelle));

	while(continuer)
	{
		temps = SDL_GetTicks();

		if(temps - tempsAncien > T_FPS)
		{
			AffichageEditeur(pMoteurRendu, images, pMap, entrees, objetPris);
			objetPris = MiseAJourMap(pMap, images, &entrees, pMoteurSon, pSons);
			tempsAncien = temps;
		}

		GestionEvenements(&entrees);

		if (entrees.clavier[ECHAP] || entrees.fermeture)
		{
			entrees.clavier[ECHAP] = false;

			if(MessageInformations("Voulez-vous vraiment retourner au menu sans sauvegarder ?", polices, pMoteurRendu, &entrees) == 1)
			{
				continuer = false;
			}
		}

		if (entrees.clavier[S])
		{
			if(VerifierEmplacements(images, pMap) == -1)
			{
				if(BSons)
				{
					FMOD_System_PlaySound(pMoteurSon, S_ALARME+10, pSons->bruits[S_ALARME], false, NULL);
					FMOD_System_GetChannel(pMoteurSon, S_ALARME+10, &pChannelEnCours);
					FMOD_Channel_SetLoopCount(pChannelEnCours, 10);
				}

				MessageInformations("Le missile et une des boules sont sur la même trajectoire.", polices, pMoteurRendu, &entrees);

				if(BSons)
				{
					FMOD_Channel_Stop(pChannelEnCours);
				}
			}
			else if (VerifierEmplacements(images, pMap) == -2)
			{
				if(BSons)
				{
					FMOD_System_PlaySound(pMoteurSon, S_ALARME+10, pSons->bruits[S_ALARME], false, NULL);
					FMOD_System_GetChannel(pMoteurSon, S_ALARME+10, &pChannelEnCours);
					FMOD_Channel_SetLoopCount(pChannelEnCours, 10);
				}

				MessageInformations("Une des boules est en collision.", polices, pMoteurRendu, &entrees);

				if(BSons)
				{
					FMOD_Channel_Stop(pChannelEnCours);
				}
			}
			else if (VerifierEmplacements(images, pMap) == -3)
			{
				if(BSons)
				{
					FMOD_System_PlaySound(pMoteurSon, S_ALARME+10, pSons->bruits[S_ALARME], false, NULL);
					FMOD_System_GetChannel(pMoteurSon, S_ALARME+10, &pChannelEnCours);
					FMOD_Channel_SetLoopCount(pChannelEnCours, 10);
				}

				MessageInformations("Le vortex bleu est en collision.", polices, pMoteurRendu, &entrees);

				if(BSons)
				{
					FMOD_Channel_Stop(pChannelEnCours);
				}
			}
			else if (VerifierEmplacements(images, pMap) == -4)
			{
				if(BSons)
				{
					FMOD_System_PlaySound(pMoteurSon, S_ALARME+10, pSons->bruits[S_ALARME], false, NULL);
					FMOD_System_GetChannel(pMoteurSon, S_ALARME+10, &pChannelEnCours);
					FMOD_Channel_SetLoopCount(pChannelEnCours, 10);
				}

				MessageInformations("Le vortex vert est en collision.", polices, pMoteurRendu, &entrees);

				if(BSons)
				{
					FMOD_Channel_Stop(pChannelEnCours);
				}
			}
			else
			{
				if(BSons)
				{
					FMOD_System_PlaySound(pMoteurSon, S_SAVE+10, pSons->bruits[S_SAVE], true, NULL);
					FMOD_System_GetChannel(pMoteurSon, S_SAVE+10, &pChannelEnCours);
					FMOD_Channel_SetVolume(pChannelEnCours, Volume/110.0);
					FMOD_Channel_SetPaused(pChannelEnCours, false);
				}

				SauvegardeNiveau(pMap, images);

				MessageInformations("Niveau sauvegardé !", polices, pMoteurRendu, &entrees);
				continuer = false;
			}
		}

		SDL_Delay(1);
	}

	DestructionMap(pMap);

	if(BMusique)
	{
		FMOD_System_GetChannel(pMoteurSon, M_JEU, &pChannelEnCours);
		FMOD_Channel_SetPaused(pChannelEnCours, true);
		FMOD_System_GetChannel(pMoteurSon, M_MENU, &pChannelEnCours);
		FMOD_Channel_SetPaused(pChannelEnCours, false);
	}

	return 0;
}

Map* InitialisationEditeur (SDL_Renderer *pMoteurRendu, sprite images[], TTF_Font *polices[])
{
	Map* pMap = NULL;
	SDL_Surface *pSurfMissileH, *pSurfMissileV;
	SDL_Color blancOpaque = {255, 255, 255, SDL_ALPHA_OPAQUE};

	pSurfMissileH = TTF_RenderText_Blended(polices[SNICKY], "Ajout missile H", blancOpaque);
	images[AJOUTER_MISSILE_H].pTextures[0] = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfMissileH);
	TTF_SizeText(polices[SNICKY], "Ajout missile H", &images[AJOUTER_MISSILE_H].position[0].w, &images[AJOUTER_MISSILE_H].position[0].h);
	SDL_FreeSurface(pSurfMissileH);

	pSurfMissileV = TTF_RenderText_Blended(polices[SNICKY], "Ajout missile V", blancOpaque);
	images[AJOUTER_MISSILE_V].pTextures[0] = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfMissileV);
	TTF_SizeText(polices[SNICKY], "Ajout missile V", &images[AJOUTER_MISSILE_V].position[0].w, &images[AJOUTER_MISSILE_V].position[0].h);
	SDL_FreeSurface(pSurfMissileV);

	pMap = ChargementNiveau(pMoteurRendu, "editeur", -1);

	InitialisationPositions(images, "editeur", -1);

	return pMap;
}

int AffichageEditeur(SDL_Renderer *pMoteurRendu, sprite images[], Map* pMap, ClavierSouris entrees, int objetPris)
{
	int i=0, j=0, k=0;               //Variable de comptage
	SDL_Point pointOrigine={0, 0};

	SDL_SetRenderDrawColor(pMoteurRendu, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(pMoteurRendu);

	boxRGBA(pMoteurRendu, 0, 0, Largeur, Hauteur, 85, 120, 180, SDL_ALPHA_OPAQUE);


	AffichageMap(pMoteurRendu, pMap);

	AffichageBonus(pMoteurRendu, pMap, images);


	SDL_RenderCopy(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[0]);
	SDL_RenderCopy(pMoteurRendu, images[VORTEX_BLEU].pTextures[0], NULL, &images[VORTEX_BLEU].position[0]);
	SDL_RenderCopy(pMoteurRendu, images[VORTEX_VERT].pTextures[0], NULL, &images[VORTEX_VERT].position[0]);
	SDL_RenderCopy(pMoteurRendu, images[BOULE_BLEUE].pTextures[0], NULL, &images[BOULE_BLEUE].position[0]);
	SDL_RenderCopy(pMoteurRendu, images[BOULE_MAGENTA].pTextures[0], NULL, &images[BOULE_MAGENTA].position[0]);
	SDL_RenderCopy(pMoteurRendu, images[BOULE_VERTE].pTextures[0], NULL, &images[BOULE_VERTE].position[0]);
	SDL_RenderCopy(pMoteurRendu, images[AJOUTER_MISSILE_V].pTextures[0], NULL, &images[AJOUTER_MISSILE_V].position[0]);
	SDL_RenderCopy(pMoteurRendu, images[AJOUTER_MISSILE_H].pTextures[0], NULL, &images[AJOUTER_MISSILE_H].position[0]);

	for(i=0; i<5; i++)
	{
		SDL_RenderCopy(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i]);
	}

	for (i=5; i<10; i++)
	{
		SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i], 90, &pointOrigine, SDL_FLIP_NONE);
	}

	AffichageBonusEditeur(pMoteurRendu, images);

	AffichageBoxEditeur(pMoteurRendu, &entrees);

	AffichageObjetCurseurEditeur(pMoteurRendu, &entrees, images, objetPris);


	SDL_RenderPresent(pMoteurRendu);         //Mise à jour de l'écran

	return 0;
}

int MiseAJourMap (Map *pMap, sprite images[], ClavierSouris *pEntrees, FMOD_SYSTEM *pMoteurSon, Sons *pSons)
{
	static int diaPris=AUCUN_BONUS, missilePris=0;

	DeplacementObjetEditeur(pMoteurSon, pSons, images, pEntrees);

	MiseAjourMapEtBonusEditeur(pEntrees, pMoteurSon, pSons, pMap, &diaPris, &missilePris);

	MiseAJourMapMissileEditeur(pMoteurSon, pSons, pEntrees, images, &missilePris);

	AmeliorationMap(pMap);

	if(diaPris != AUCUN_BONUS)
	{
		return diaPris;
	}
	else
	{
		return missilePris;
	}
}

int VerifierEmplacements(sprite images[], Map *pMap)
{
	int i=0, j=0;
	Collision collision={COLL_NONE, 0};

	for(i=BOULE_BLEUE; i<=BOULE_VERTE; i++)
	{
		for(j=0; j<5; j++)
		{
			if(images[i].position[0].x + images[i].position[0].w > images[MISSILE].position[j].x && images[i].position[0].x < images[MISSILE].position[j].x + images[MISSILE].position[j].w)
		{
			return -1;
		}
		}

		for(j=5; j<10; j++)
		{
			if(images[i].position[0].y + images[i].position[0].h > images[MISSILE].position[j].y && images[i].position[0].y < images[MISSILE].position[j].y + images[MISSILE].position[j].w)
		{
			return -1;
		}
		}


	CollisionDetect(images, i, pMap, &collision);

		if(collision.etatColl & ~COLL_NONE)
		{
			return -2;
		}
	}

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

	return 0;
}

void DeplacementObjetEditeur(FMOD_SYSTEM *pMoteurSon, Sons *pSons, sprite images[], ClavierSouris *pEntrees)
{
	int i=0;
	static int deplacement = -1;

	for (i=BOULE_BLEUE; i<=VORTEX_VERT; i++)
	{
		if(i == MISSILE)
		{
			continue;
		}

		if((pEntrees->souris.touches[C_MOLETTE] && pEntrees->souris.position.x > images[i].position[0].x && pEntrees->souris.position.x < (images[i].position[0].x + images[i].position[0].w) && pEntrees->souris.position.y > images[i].position[0].y && pEntrees->souris.position.y < (images[i].position[0].y + images[i].position[0].h)) || deplacement != -1)
		{
			if (deplacement == -1)
			{
				FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);
				deplacement = i;
				pEntrees->souris.touches[C_MOLETTE] = false;
			}

			images[deplacement].position[0].x = pEntrees->souris.position.x - images[deplacement].position[0].w /2;
			images[deplacement].position[0].y = pEntrees->souris.position.y - images[deplacement].position[0].h /2;

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
	SDL_Rect pos;
	int i=0, j=0, k=0;

	pos.x = pEntrees->souris.position.x;
	pos.y = pEntrees->souris.position.y;

	if(pos.x > 0 && pos.x < Largeur && pos.y > 0 && pos.y < Hauteur)
	{
		if(pEntrees->souris.touches[C_GAUCHE] && !pEntrees->clavier[SHIFT] && !(*pDiaPris) && !(*pMissilePris))
		{
			if(BSons)
			{
				FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);
			}

			pMap->plan[pos.x /TailleBloc][pos.y /TailleBloc] = SOL_NORMAL;
		}
		else if(pEntrees->souris.touches[C_DROIT] && !pEntrees->clavier[SHIFT])
		{
			pMap->plan[pos.x /TailleBloc][pos.y /TailleBloc] = VIDE;
		}
		else if (*pDiaPris && pEntrees->souris.touches[C_GAUCHE])
		{
			if(BSons)
			{
				FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);
			}

			pMap->planObjets[pos.x /TailleBloc][pos.y /TailleBloc] = *pDiaPris;
			*pDiaPris = AUCUN_BONUS;
			pEntrees->souris.touches[C_GAUCHE] = false;
		}
		else if (pEntrees->souris.touches[C_DROIT] && pEntrees->clavier[SHIFT])
		{
			pMap->planObjets[pos.x /TailleBloc][pos.y /TailleBloc] = AUCUN_BONUS;
		}
	}

	if(pEntrees->clavier[SHIFT] && pEntrees->souris.touches[C_GAUCHE])
	{
		for (k=1, i=0; i<2; i++)
		{
			for (j=0; j<9; j++, k++)
			{
				if(pEntrees->souris.position.x > 0.075*Largeur*(double)j +0.081*Largeur && pEntrees->souris.position.x < 0.075*Largeur*(double)(j+1) +0.081*Largeur && pEntrees->souris.position.y > 0.075*Hauteur*(double)i +0.83*Hauteur && pEntrees->souris.position.y < 0.075*Largeur*(double)(i+1) +0.83*Hauteur)
				{
					FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);
					*pDiaPris = k;
					pEntrees->souris.touches[C_GAUCHE] = false;
				}
			}
		}
	}
}

void MiseAJourMapMissileEditeur(FMOD_SYSTEM *pMoteurSon, Sons *pSons, ClavierSouris *pEntrees, sprite images[], int *pMissilePris)
{
	int i=0, j=0;

	if(pEntrees->souris.touches[C_MOLETTE] && pEntrees->souris.position.x > images[AJOUTER_MISSILE_H].position[0].x && pEntrees->souris.position.x < images[AJOUTER_MISSILE_H].position[0].x + images[AJOUTER_MISSILE_H].position[0].w)
	{
		if(pEntrees->souris.position.y > images[AJOUTER_MISSILE_H].position[0].y && pEntrees->souris.position.y < images[AJOUTER_MISSILE_H].position[0].y + images[AJOUTER_MISSILE_H].position[0].h)
		{
			FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);
			*pMissilePris=101;
			pEntrees->souris.touches[C_MOLETTE] = false;
		}
	}

	if(pEntrees->souris.touches[C_MOLETTE] && pEntrees->souris.position.x > images[AJOUTER_MISSILE_V].position[0].x && pEntrees->souris.position.x < images[AJOUTER_MISSILE_V].position[0].x + images[AJOUTER_MISSILE_V].position[0].w)
	{
		if(pEntrees->souris.position.y > images[AJOUTER_MISSILE_V].position[0].y && pEntrees->souris.position.y < images[AJOUTER_MISSILE_V].position[0].y + images[AJOUTER_MISSILE_V].position[0].h)
		{
			FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);
			*pMissilePris=100;
			pEntrees->souris.touches[C_MOLETTE] = false;
		}
	}

	if (*pMissilePris && pEntrees->souris.touches[C_MOLETTE])
	{
		switch(*pMissilePris)
		{
		case 100:
			for(i=0; i<5; i++)
			{
				if(images[MISSILE].position[i].x < 0)
				{
					j=1;
					break;
				}
				else
				{
					j=-1;
				}
			}

			if (j != -1)
			{
				images[MISSILE].position[i].x = pEntrees->souris.position.x;
				images[MISSILE].position[i].y = pEntrees->souris.position.y;
			}

			pEntrees->souris.touches[C_MOLETTE] = false;
			break;

		case 101:
			for(i=5; i<10; i++)
			{
				if(images[MISSILE].position[i].x < 0)
				{
					j=1;
					break;
				}
				else
				{
					j=-1;
				}
			}

			if (j != -1)
			{
				images[MISSILE].position[i].x = pEntrees->souris.position.x;
				images[MISSILE].position[i].y = pEntrees->souris.position.y;
			}

			pEntrees->souris.touches[C_MOLETTE] = false;
			break;

		default:
			break;
		}

		FMOD_System_PlaySound(pMoteurSon, S_CLICK+10, pSons->bruits[S_CLICK], false, NULL);
		*pMissilePris = 0;
	}
}

void AmeliorationMap(Map *pMap)
{
	int i=0, j=0;

	for (i=0; i< pMap->nbtiles_largeur_monde; i++)
	{
		for (j=0; j< pMap->nbtiles_hauteur_monde; j++)
		{
			if(pMap->plan[i][j] == SOL_NORMAL && pMap->plan[i][j+1] != VIDE && j < pMap->nbtiles_hauteur_monde-1)
			{
				pMap->plan[i][j+1] = SOL_PLEIN_3;

				if (pMap->plan[i][j+2] != VIDE && j < (pMap->nbtiles_hauteur_monde-2))
				{
					pMap->plan[i][j+2] = SOL_PLEIN_UNI;
				}
			}

			if(pMap->plan[i][j] == SOL_NORMAL && pMap->plan[i][j-1] == SOL_NORMAL)
			{
				pMap->plan[i][j] = SOL_PLEIN_3;
			}
			else if (pMap->plan[i][j] == SOL_NORMAL && pMap->plan[i][j-1] == SOL_PLEIN_3)
			{
				pMap->plan[i][j] = SOL_PLEIN_UNI;
			}
			else if (pMap->plan[i][j] == SOL_NORMAL && pMap->plan[i][j-1] == SOL_PLEIN_UNI)
			{
				pMap->plan[i][j] = SOL_PLEIN_UNI;
			}

			if (pMap->plan[i][j] == SOL_PLEIN_3 && pMap->plan[i][j-1] == SOL_PLEIN_UNI)
			{
				pMap->plan[i][j] = SOL_PLEIN_UNI;
			}
			else if (pMap->plan[i][j] == SOL_PLEIN_3 && pMap->plan[i][j-1] == VIDE)
			{
				pMap->plan[i][j] = SOL_NORMAL;
			}

			if(pMap->plan[i][j] == SOL_PLEIN_UNI && pMap->plan[i][j-1] == SOL_NORMAL)
			{
				pMap->plan[i][j] = SOL_PLEIN_3;
			}
			else if (pMap->plan[i][j] == SOL_PLEIN_UNI && pMap->plan[i][j-1] == VIDE)
			{
				pMap->plan[i][j] = SOL_NORMAL;
			}
		}
	}
}

void AffichageBonusEditeur(SDL_Renderer *pMoteurRendu, sprite images[])
{
	int i=0, j=0, k=0;

	for (k=1, i=0; i<2; i++)
	{
		for (j=0; j<9; j++, k++)
		{
			images[GEMMES].position[0].x = Arrondir(0.075*Largeur*(double)j + 0.081*Largeur);
			images[GEMMES].position[0].y = Arrondir(0.075*Hauteur*(double)i + 0.83*Hauteur);
			SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[k], &images[GEMMES].position[0]);
		}
	}
}

void AffichageBoxEditeur(SDL_Renderer *pMoteurRendu, ClavierSouris *pEntrees)
{
	int i=0;

	if(pEntrees->clavier[SHIFT])
	{
		for (i=0; i<=4; i++)
		{
			roundedRectangleRGBA(pMoteurRendu, (short)(0.89*Largeur +i), (short)i, (short)(Largeur-10-i), (short)(Hauteur-10-i), (short)10, (unsigned char)255, (unsigned char)255, (unsigned char)255, (unsigned char)128);
			roundedRectangleRGBA(pMoteurRendu, (short)(10+i), (short)(0.8*Hauteur +i), (short)(0.8*Largeur -i), (short)(Hauteur-10-i), (short)10, (unsigned char)255, (unsigned char)255, (unsigned char)255, (unsigned char)200);
		}
	}
	else
	{
		for (i=0; i<=4; i++)
		{
			roundedRectangleRGBA(pMoteurRendu, (short)(0.89*Largeur +i), (short)i, (short)(Largeur-10-i), (short)(Hauteur-10-i), (short)10, (unsigned char)255, (unsigned char)255, (unsigned char)255, (unsigned char)128);
			roundedRectangleRGBA(pMoteurRendu, (short)(10+i), (short)(0.8*Hauteur +i), (short)(0.8*Largeur -i), (short)(Hauteur-10-i), (short)10, (unsigned char)255, (unsigned char)255, (unsigned char)255, (unsigned char)100);
		}
	}
}

void AffichageObjetCurseurEditeur(SDL_Renderer *pMoteurRendu, ClavierSouris *pEntrees, sprite images[], int objetPris)
{
	SDL_Point pointOrigine={0, 0};
	int angleCurseur=335;

	images[MISSILE].position[10].x = images[GEMMES].position[0].x = images[CURSEUR].position[0].x = pEntrees->souris.position.x;
	images[MISSILE].position[10].y = images[GEMMES].position[0].y = images[CURSEUR].position[0].y = pEntrees->souris.position.y;

	if(objetPris != AUCUN_BONUS && objetPris != 100 && objetPris != 101)
	{
		SDL_RenderCopy(pMoteurRendu, images[GEMMES].pTextures[0], &images[GEMMES].position[objetPris], &images[GEMMES].position[0]);
	}
	else if (objetPris == 100)
	{
		SDL_RenderCopy(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[10]);
	}
	else if (objetPris == 101)
	{
		SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[10], 90, &pointOrigine, SDL_FLIP_NONE);
	}

	SDL_RenderCopyEx(pMoteurRendu, images[CURSEUR].pTextures[0], NULL, &images[CURSEUR].position[0], angleCurseur, NULL, SDL_FLIP_NONE);
}

//Fin du fichier editeur.c
