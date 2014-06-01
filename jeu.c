/*
Projet-ISN

Fichier: jeu.c

Contenu: Fonctions principales du jeu: la boucle, l'affichage ...

Actions: C'est ici que sont effectuées les tâches principales du jeu, la boucle principale, le rendu à l'écran ...

Bibliothèques utilisées: Bibliothèques standards, SDL, SDL_image, SDL_ttf, FMOD, GTK

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <gtk/gtk.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <fmod.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "jeu.h"
#include "collision.h"

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;		//Lien vers les variables globales déclarées dans main.c
extern double Volume, Largeur, Hauteur;
extern InfoDeJeu infos;

int BouclePrincipale(Joueur *pJoueur, Sprite images[], Animation anim[], SDL_Renderer *pMoteurRendu, FMOD_SYSTEM *pMoteurSon, Sons *pSons, TTF_Font *polices[])
{
    ClavierSouris entrees;     //Structure pour connaître l'état du clavier et de la souris
    unsigned char descente[10]= {false};	//Tableau pour savoir quand les missiles montent ou descendent
    unsigned char ajoutAnim=false;	//Contrôle de l'exécution d'une animation
    unsigned int tempsFPS=0, tempsAncienFPS=0;	//Différents temps pour les calculs
    int differenceFPS=0, etat=0, control=JEU_EN_COURS, etatNiveau, i=0;
    FMOD_CHANNEL *pChannelEnCours=NULL;	//Contrôle des canaux audio
    Map *pMap=NULL;	//Pointeurs sur une structure Map
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
                infos.bonus &= AUCUN_BONUS;

                for(i=0; i<10; i++)
                {
                    descente[i] = false;
                }
            }
        }

        /* Maintenant on va mettre à jour les coordonnées et faire l'affichage */

        tempsFPS = SDL_GetTicks();	//On prend le temps écoulé depuis l'initialisation de la SDL

        differenceFPS = tempsFPS - tempsAncienFPS;	//On soustrait

        if(differenceFPS > T_FPS)	//On regarde s'il s'est écoulé plus de T_FPS ms depuis le dernier affichage et la mise à jour des coordonnées
        {
            if(!ajoutAnim)	//On ne met pas à jour les coordonnées pendant l'animation (gravité, ect)
            {
                MiseAJourCoordonnees(entrees, images, descente, pMap, pMoteurSon, pSons);
            }

            /* Ensuite on effectue l'affichage */
            Affichage(pMoteurRendu, images, polices, descente, pMap, anim, &ajoutAnim);

            tempsAncienFPS = tempsFPS;
        }

        /* On regarde si le joueur a gagné le niveau ou s'il s'est fait tuer */
        etat = VerifierMortOUGagne(images, pMap, pMoteurSon, pSons);
        /* On réagit en conséquence */
        TraitementEtatDuNiveau(pMoteurRendu, pMoteurSon, pSons, &pMap, pJoueur, images, polices, anim, &entrees, descente, &etat, &ajoutAnim, &control);

        if(infos.vies == 0)	//S'il ne reste plus de vie, on a perdu
        {
            if(BMusique)
            {
                FMOD_System_GetChannel(pMoteurSon, M_JEU, &pChannelEnCours);	//On arrête la musique du jeu
                FMOD_Channel_SetPaused(pChannelEnCours, true);

                FMOD_System_PlaySound(pMoteurSon, M_PERDU, pSons->music[M_PERDU], true, NULL);		//On joue la musique quand on a perdu
                FMOD_System_GetChannel(pMoteurSon, M_PERDU, &pChannelEnCours);
                FMOD_Channel_SetVolume(pChannelEnCours, (float)(Volume/100.0));
                FMOD_Channel_SetPaused(pChannelEnCours, false);
            }

            Perdu(pMoteurRendu, images, anim, pMap, polices, &ajoutAnim);	//On affiche l'écran de fin quand on a perdu

            control = JEU_FIN;	//On coupe la boucle
        }

        DetectionBonus(images, BOULE_MAGENTA, pMap);	//Si le joueur prend un bonus

        /* On s'occupe d'ajouter de la vie ou du score selon les bonus pris, on désactive les bonus ensuite et on joue un son */
        TraitementBonus(pMoteurSon, pSons);

        if(!ajoutAnim)
        {
            Chrono();	//Mise à jour du compteur de temps du niveau (sauf pendant l'animation)
        }

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
    static unsigned char sautEnCoursBleue=false, sautEnCoursVerte=false;	//Variables de contrôle des sauts

    /* Maintenant on va changer les positions des boules */
    DeplacementBoules(images, pMap, &entrees, pMoteurSon, pSons);

    /* On les fait sauter */
    Sauts(images, pMap, &entrees, &sautEnCoursBleue, &sautEnCoursVerte);

    /* On déplace les missiles */
    DeplacementMissiles(images, descente);

    /* On applique la gravité */
    Gravite(images, pMap, sautEnCoursBleue, sautEnCoursVerte);

    return 0;
}

int Sauts(Sprite images[], Map *pMap, ClavierSouris *pEntrees, unsigned char *pSautEnCoursBleue, unsigned char *pSautEnCoursVerte)
{
    static unsigned char timerBleue=false, timerVerte=false;	//Variables de contrôle des timers des sauts
    static unsigned int temps=0, tempsAncien=0;	//Variables de temps
    static int savePosBleue=0, savePosVerte=0, difference=0;	//Variables pour sauvegarder des positions et une différence de temps
    Collision collision= {COLL_NONE, 0};	//Structure pour gérer les collisions

    temps = SDL_GetTicks();	//On prend le temps écoulé depuis l'initialisation de la SDL

    difference = temps - tempsAncien;	//On calcul la différence avec l'ancien temps

    /* Si il s'est écoulé au moins 250ms, on réinitialise les timers des sauts */
    if (difference >= 250)
    {
        timerBleue = timerVerte = false;
        tempsAncien = temps;
    }
    else if (difference < 0)	//Si la différence de temps est négative, c'est à cause des variables statiques, on remet donc le temps ancien à 0
    {
        tempsAncien = 0;
    }

    /* Si le timer n'est pas actif et que l'on appuie sur Espace ou qu'un saut était déjà en cours */
    if((pEntrees->clavier[ESPACE] || *pSautEnCoursBleue) && !timerBleue)
    {
        /* Si un saut était en cours, on le poursuit */
        if(*pSautEnCoursBleue)
        {
            savePosBleue = SautBleue(&images[BOULE_BLEUE].position[0], pSautEnCoursBleue);
            images[BOULE_BLEUE].position[0].y += savePosBleue;
        }
        /* Sinon c'est que l'on a appuyé sur Espace, on vérifie donc que l'on peut commencer un saut (on doit être posé sur le sol) */
        else if (pMap->plan[Arrondir(images[BOULE_BLEUE].position[0].x + images[BOULE_BLEUE].position[0].w/2.0 - TailleBoule*0.3) /TailleBloc][Arrondir(images[BOULE_BLEUE].position[0].y + images[BOULE_BLEUE].position[0].h +1) /TailleBloc] != VIDE
            ||
            pMap->plan[Arrondir(images[BOULE_BLEUE].position[0].x + images[BOULE_BLEUE].position[0].w/2.0 + TailleBoule*0.3) /TailleBloc][Arrondir(images[BOULE_BLEUE].position[0].y + images[BOULE_BLEUE].position[0].h +1) /TailleBloc] != VIDE)
        {
            savePosBleue = SautBleue(&images[BOULE_BLEUE].position[0], pSautEnCoursBleue);
            images[BOULE_BLEUE].position[0].y += savePosBleue;
        }

        /* Ensuite on détecte les collisions */
        CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

        /* Si on est rentré dans une des autres boules ou dans le décor, on annule le déplacement et on arrête le saut et on active le timer*/
        if((collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
        {
            images[BOULE_BLEUE].position[0].y -= savePosBleue;
            savePosBleue=0;
            *pSautEnCoursBleue=false;
            timerBleue=true;
        }
    }

    /* Il en va de même avec la boule verte */
    if((pEntrees->clavier[ESPACE] || *pSautEnCoursVerte) && !timerVerte)
    {
        if(*pSautEnCoursVerte)
        {
            savePosVerte = SautVerte(&images[BOULE_VERTE].position[0], pSautEnCoursVerte);
            images[BOULE_VERTE].position[0].y += savePosVerte;
        }
        else if (pMap->plan[Arrondir(images[BOULE_VERTE].position[0].x + images[BOULE_VERTE].position[0].w/2.0 - TailleBoule*0.3) /TailleBloc][Arrondir(images[BOULE_VERTE].position[0].y -1)/ TailleBloc] != VIDE || pMap->plan[Arrondir(images[BOULE_VERTE].position[0].x + images[BOULE_VERTE].position[0].w/2.0 + TailleBoule*0.3) /TailleBloc][Arrondir(images[BOULE_VERTE].position[0].y -1)/ TailleBloc] != VIDE)
        {
            savePosVerte = SautVerte(&images[BOULE_VERTE].position[0], pSautEnCoursVerte);
            images[BOULE_VERTE].position[0].y += savePosVerte;
        }

        CollisionDetect(images, BOULE_VERTE, pMap, &collision);

        if((collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_DECOR))
        {
            images[BOULE_VERTE].position[0].y -= savePosVerte;
            savePosVerte=0;
            *pSautEnCoursVerte=false;
            timerVerte=true;
        }
    }

    return 0;
}

int Gravite(Sprite images[], Map *pMap, unsigned char sautEnCoursBleue, unsigned char sautEnCoursVerte)
{
    int i=0, v_gx = Arrondir(0.0085*Hauteur); //Compteur + variable de vitesse de la gravité
    Collision collision= {COLL_NONE, 0};	//Structure pour gérer les collisions

    /* On applique la gravité si la boule n'est pas en train de sauter */
    if(!sautEnCoursBleue)
    {
        images[BOULE_BLEUE].position[0].y += v_gx;	//On applique la gravité

        CollisionDetect(images, BOULE_BLEUE, pMap, &collision);	//Détection des collisions

        /* On annule le déplacement de gravité si on touche le décor ou une des 2 autres boules */
        if((collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE))
        {
            images[BOULE_BLEUE].position[0].y -= v_gx;

            /* Pour éviter que la boule ne flotte dans l'air, on réapplique la gravité pixel par pixel */
            for (i=1; i<v_gx; i++)
            {
                images[BOULE_BLEUE].position[0].y += 1;

                CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

                if((collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE))
                {
                    images[BOULE_BLEUE].position[0].y -= 1;
                    break;
                }
            }
        }
    }

    if(!sautEnCoursVerte)
    {
        images[BOULE_VERTE].position[0].y -= v_gx;	//Gravité inversée

        CollisionDetect(images, BOULE_VERTE, pMap, &collision);	//Détection des collisions

        /* On annule le déplacement de gravité si on touche le décor ou une des 2 autres boules */
        if((collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE))
        {
            images[BOULE_VERTE].position[0].y += v_gx;

            /* Pour éviter que la boule ne flotte dans l'air, on réapplique la gravité pixel par pixel */
            for (i=1; i<v_gx; i++)
            {
                images[BOULE_VERTE].position[0].y -= 1;

                CollisionDetect(images, BOULE_VERTE, pMap, &collision);

                if((collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE))
                {
                    images[BOULE_VERTE].position[0].y += 1;
                    break;
                }
            }
        }
    }

    return 0;
}

int DeplacementMissiles(Sprite images[], unsigned char descente[])
{
    int i=0;	//Compteur

    /* On déplace les 5 missiles verticaux */
    for(i=0; i<5; i++)
    {
        /* On vérifie que, pour la descente, le missile ne sort pas de l'écran */
        if(images[MISSILE].position[i].y < (Hauteur-images[MISSILE].position[i].h) && descente[i])
        {
            images[MISSILE].position[i].y +=  Arrondir(0.003*Largeur);

            /* S'il sort après le déplacement, on bascule en montée */
            if (images[MISSILE].position[i].y >= Arrondir(Hauteur-images[MISSILE].position[i].h))
            {
                images[MISSILE].position[i].y = Arrondir(Hauteur-images[MISSILE].position[i].h);
                descente[i] = false;
            }
        }
        /* On vérifie que, pour la montée, le missile ne sort pas de l'écran */
        else if((images[MISSILE].position[i].y > 0) && !descente[i])
        {
            images[MISSILE].position[i].y -= Arrondir(0.003*Largeur);

            /* S'il sort après le déplacement, on bascule en descente */
            if (images[MISSILE].position[i].y <= 0)
            {
                images[MISSILE].position[i].y = 0;
                descente[i] = true;
            }
        }
    }

    /* Puis on déplace les 5 missiles horizontaux */
    for(i=5; i<10; i++)
    {
        /* On vérifie que, pour aller vers la droite, le missile ne sort pas de l'écran */
        if(images[MISSILE].position[i].x < Largeur && descente[i])
        {
            images[MISSILE].position[i].x +=  Arrondir(0.003*Largeur);

            /* S'il sort après le déplacement, on inverse le sens */
            if (images[MISSILE].position[i].x >= Largeur)
            {
                images[MISSILE].position[i].x = (int)Largeur;
                descente[i] = false;
            }
        }
        /* On vérifie que, pour aller vers la gauche, le missile ne sort pas de l'écran */
        else if((images[MISSILE].position[i].x > 0) && !descente[i])
        {
            images[MISSILE].position[i].x -= Arrondir(0.003*Largeur);

            /* S'il sort après le déplacement, on inverse le sens */
            if (images[MISSILE].position[i].x - images[MISSILE].position[i].h <= 0)
            {
                images[MISSILE].position[i].x = images[MISSILE].position[i].h;
                descente[i] = true;
            }
        }
    }

    return 0;
}

int DeplacementBoules(Sprite images[], Map *pMap, ClavierSouris *pEntrees, FMOD_SYSTEM *pMoteurSon, Sons *pSons)
{
    int i=0, enLecture=false;	//Compteur + variable pour contrôler l'état de lecture
    FMOD_CHANNEL *pChannelEnCours=NULL;	//Pour contrôler la musique
    Collision collision= {COLL_NONE, 0};	//Structure pour gérer les collisions
    static int v_x1=0, v_y1=0, v_x2=0, v_x3=0;	// Variables de vitesse

    /* On augmente progressivement la vitesse de la boule magenta dans une certaine limite */
    if(pEntrees->clavier[HAUT] && v_y1 > Arrondir(-0.002*Largeur))
    {
        v_y1--;
    }
    else if(pEntrees->clavier[BAS] && v_y1 < Arrondir(0.002*Largeur))
    {
        v_y1++;
    }
    else if(!(pEntrees->clavier[HAUT] || pEntrees->clavier[BAS]))	//Si on appuie sur aucune touche on remet à zéro la vitesse
    {
        v_y1 = 0;
    }

    /* On applique la vitesse */
    images[BOULE_MAGENTA].position[0].y += v_y1;

    CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);	//On détecte les collisions

    /* S'il y a une collision avec une autre boule, le décor ou le bord de la fenêtre en haut ou en bas */
    if ((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BORD_HAUT) || (collision.etatColl & COLL_BORD_BAS))
    {
        /* Si c'est avec une autre boules, on joue le son de collision entre boules */
        if((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE))
        {
            if(BSons)
            {
                FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOULE+10, &pChannelEnCours);
                FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);

                if (!enLecture)
                {
                    FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOULE+10, pSons->bruits[S_BOULE_BOULE], false, NULL);
                }
            }
        }

        /* On annule le déplacement car il y eu collision */
        images[BOULE_MAGENTA].position[0].y -= v_y1;

        /* Si la vitesse était positive (vers le bas) */
        if (v_y1 > 0)
        {
            /* On tente de se coller à l'obstacle, pixel par pixel */
            for (i=1; i<v_y1; i++)
            {
                images[BOULE_MAGENTA].position[0].y += 1;

                CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);

                if ((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BORD_HAUT) || (collision.etatColl & COLL_BORD_BAS))
                {
                    images[BOULE_MAGENTA].position[0].y -= 1;
                    break;
                }
            }
        }
        else	//Sinon si la vitesse était négative (vers le haut)
        {
            /* On tente de se coller à l'obstacle, pixel par pixel */
            for (i=1; i>v_y1; i--)
            {
                images[BOULE_MAGENTA].position[0].y -= 1;

                CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);

                if ((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR) || (collision.etatColl & COLL_BORD_HAUT) || (collision.etatColl & COLL_BORD_BAS))
                {
                    images[BOULE_MAGENTA].position[0].y += 1;
                    break;
                }
            }
        }
    }

    /* On augmente la vitesse de la boule magenta progressivement (pour l'axe x cette fois) */
    if(pEntrees->clavier[GAUCHE] && v_x1 > Arrondir(-0.002*Largeur))
    {
        v_x1--;
    }
    else if(pEntrees->clavier[DROITE] && v_x1 < Arrondir(0.002*Largeur))
    {
        v_x1++;
    }
    else if(!(pEntrees->clavier[GAUCHE] || pEntrees->clavier[DROITE]))	//Si on appuie sur aucune touche on remet à zéro la vitesse
    {
        v_x1 = 0;
    }

    /* On applique le déplacement */
    images[BOULE_MAGENTA].position[0].x += v_x1;

    CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);	//Détection des collisions

    /* S'il y a une collision à droite, à gauche, avec une des 2 autres boules ou avec le décor */
    if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
    {
        /* Si c'est avec une des 2 autres boules, on joue le son de collision entre boules */
        if((collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE))
        {
            if(BSons)
            {
                FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOULE+10, &pChannelEnCours);
                FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);

                if (!enLecture)
                {
                    FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOULE+10, pSons->bruits[S_BOULE_BOULE], false, NULL);
                }
            }
        }

        /* On annule le déplacement */
        images[BOULE_MAGENTA].position[0].x -= v_x1;

        /* La vitesse est positive (vers la droite) */
        if (v_x1 > 0)
        {
            /* On tente de s'approcher pixel par pixel de l'obstacle */
            for (i=1; i<v_x1; i++)
            {
                images[BOULE_MAGENTA].position[0].x += 1;

                CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);

                if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
                {
                    images[BOULE_MAGENTA].position[0].x -= 1;
                    break;
                }
            }
        }
        else	//Sinon vitesse négative (vers la gauche)
        {
            /* On tente de s'approcher pixel par pixel de l'obstacle */
            for (i=1; i>v_x1; i--)
            {
                images[BOULE_MAGENTA].position[0].x -= 1;

                CollisionDetect(images, BOULE_MAGENTA, pMap, &collision);

                if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
                {
                    images[BOULE_MAGENTA].position[0].x += 1;
                    break;
                }
            }
        }
    }

    /* On commence par augmenter progressivement la vitesse de la boule bleue (en x) dans la limite définie par la présence de bonus */
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

    /* Déplacement */
    images[BOULE_BLEUE].position[0].x += v_x2;

    CollisionDetect(images, BOULE_BLEUE, pMap, &collision);	//Détection des collisions

    /* S'il y a une collision avec le bord gauche, droit, le décor ou une des deux autres boules */
    if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
    {
        /* Si c'est avec une des 2 autres boules, on joue le son de collision entre boules */
        if((collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE))
        {
            if(BSons)
            {
                FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOULE+10, &pChannelEnCours);
                FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);

                if (!enLecture)
                {
                    FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOULE+10, pSons->bruits[S_BOULE_BOULE], false, NULL);
                }
            }
        }

        /* On annule le déplacement */
        images[BOULE_BLEUE].position[0].x -= v_x2;

        /* Si la vitesse est positive (vers la droite) */
        if (v_x2 > 0)
        {
            /* On tente de s'approcher pixel par pixel de l'obstacle */
            for (i=1; i<v_x2; i++)
            {
                images[BOULE_BLEUE].position[0].x += 1;

                CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

                if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
                {
                    images[BOULE_BLEUE].position[0].x -= 1;
                    break;
                }
            }
        }
        else	//Sinon la vitesse est négative (vers la gauche)
        {
            /* On tente de s'approcher pixel par pixel de l'obstacle */
            for (i=1; i>v_x2; i--)
            {
                images[BOULE_BLEUE].position[0].x -= 1;

                CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

                if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_VERTE) || (collision.etatColl & COLL_DECOR))
                {
                    images[BOULE_BLEUE].position[0].x += 1;
                    break;
                }
            }
        }
    }

    /* On commence par augmenter progressivement la vitesse de la boule verte (en x) dans la limite définie par la présence de bonus */
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

    /* Déplacement */
    images[BOULE_VERTE].position[0].x += v_x3;

    CollisionDetect(images, BOULE_VERTE, pMap, &collision);	//Détection des collisions

    /* S'il y a une collision avec le bord gauche, droit, le décor ou une des deux autres boules */
    if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_DECOR))
    {
        /* Si c'est avec une des 2 autres boules, on joue le son de collision entre boules */
        if((collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE))
        {
            if(BSons)
            {
                FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOULE+10, &pChannelEnCours);
                FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);

                if (!enLecture)
                {
                    FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOULE+10, pSons->bruits[S_BOULE_BOULE], false, NULL);
                }
            }
        }

        /* On annule le déplacement */
        images[BOULE_VERTE].position[0].x -= v_x3;

        /* Si la vitesse est positive (vers la droite) */
        if (v_x3 > 0)
        {
            for (i=1; i<v_x3; i++)
            {
                images[BOULE_VERTE].position[0].x += 1;

                CollisionDetect(images, BOULE_VERTE, pMap, &collision);

                if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_DECOR))
                {
                    images[BOULE_VERTE].position[0].x -= 1;
                    break;
                }
            }
        }
        else	//Sinon la vitesse est négative (vers la gauche)
        {
            for (i=1; i>v_x3; i--)
            {
                images[BOULE_VERTE].position[0].x -= 1;

                CollisionDetect(images, BOULE_VERTE, pMap, &collision);

                if ((collision.etatColl & COLL_BORD_GAUCHE) || (collision.etatColl & COLL_BORD_DROIT) || (collision.etatColl & COLL_BOULE_MAGENTA) || (collision.etatColl & COLL_BOULE_BLEUE) || (collision.etatColl & COLL_DECOR))
                {
                    images[BOULE_VERTE].position[0].x += 1;
                    break;
                }
            }
        }
    }

    return 0;
}

char VerifierMortOUGagne(Sprite images[], Map *pMap, FMOD_SYSTEM *pMoteurSon, Sons *pSons)
{
    /* Cette fonction permet de vérifier si le niveau a été gagné ou s'il le joueur est mort par un missile ou par une chute malheureuse */

    FMOD_CHANNEL *musicEnCours = NULL;
    Collision collision= {0, 0};
    int i=0;

    /* On vérifie si pour toutes les boules on est rentré dans un missile, si c'est le cas, on joue le son d'explosion du missile et on renvoie le numéro du missile qui a explosé */
    for(i=BOULE_BLEUE; i<=BOULE_VERTE; i++)
    {
        CollisionDetect(images, i, pMap, &collision);

        if(collision.etatColl & COLL_MISSILE)
        {
            if(BSons)
            {
                FMOD_System_PlaySound(pMoteurSon, S_BOULE_BOUM+10, pSons->bruits[S_BOULE_BOUM], true, NULL);
                FMOD_System_GetChannel(pMoteurSon, S_BOULE_BOUM+10, &musicEnCours);
                FMOD_Channel_SetVolume(musicEnCours, (float)(Volume/150.0));
                FMOD_Channel_SetPaused(musicEnCours,  false);
            }

            return collision.numMissile;
        }
    }

    /* On vérifie si la boule bleue n'est pas tombée en bas */
    CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

    if (collision.etatColl & COLL_BORD_BAS)
    {
        if(BSons)
        {
            FMOD_System_PlaySound(pMoteurSon, S_TOMBE+10, pSons->bruits[S_TOMBE], true, NULL);
            FMOD_System_GetChannel(pMoteurSon, S_TOMBE+10, &musicEnCours);
            FMOD_Channel_SetVolume(musicEnCours, (float)(Volume/200.0));
            FMOD_Channel_SetPaused(musicEnCours,  false);
        }

        return MORT_BORDURE;	//On renvoie la mort
    }

    /* On vérifie si la boule verte n'est pas tombée en haut */
    CollisionDetect(images, BOULE_VERTE, pMap, &collision);

    if (collision.etatColl & COLL_BORD_HAUT)
    {
        if(BSons)
        {
            FMOD_System_PlaySound(pMoteurSon, S_TOMBE+10, pSons->bruits[S_TOMBE], true, NULL);
            FMOD_System_GetChannel(pMoteurSon, S_TOMBE+10, &musicEnCours);
            FMOD_Channel_SetVolume(musicEnCours, (float)(Volume/200.0));
            FMOD_Channel_SetPaused(musicEnCours,  false);
        }

        return MORT_BORDURE;	//On renvoie la mort
    }

    /* Si la boule bleue est dans le vortex bleu, alors on vérifie que la verte se trouve dans son vortex également */
    CollisionDetect(images, BOULE_BLEUE, pMap, &collision);

    if(collision.etatColl & COLL_VORTEX_BLEU)
    {
        CollisionDetect(images, BOULE_VERTE, pMap, &collision);

        if (collision.etatColl & COLL_VORTEX_VERT)
        {
            if(BSons)
            {
                FMOD_System_PlaySound(pMoteurSon, S_SORTIE+10, pSons->bruits[S_SORTIE], true, NULL);
                FMOD_System_GetChannel(pMoteurSon, S_SORTIE+10, &musicEnCours);
                FMOD_Channel_SetVolume(musicEnCours, (float)(Volume/140.0));
                FMOD_Channel_SetPaused(musicEnCours,  false);
            }

            return GAGNE;	//On renvoie qu'on a gagné le niveau
        }
    }

    return RIEN;	//On renvoie qu'il ne se passe rien
}

int TraitementEtatDuNiveau(SDL_Renderer *pMoteurRendu, FMOD_SYSTEM *pMoteurSon, Sons *pSons, Map **ppMap, Joueur *pJoueur, Sprite images[], TTF_Font *polices[], Animation anim[], ClavierSouris *pEntrees, unsigned char descente[], int *pEtat, unsigned char *pAjoutAnim, int *pControl)
{
    /* Cette fonction va traiter la valeur renvoyée par la fonction précédente et agir en conséquence */

    char missileTouche =-1;	//Variable pour connaître le missile qui a explosé
    int etatNiveau, i=0;	//Variable pour stocker l'état du niveau qui vient d'être chargé (Chargement réussi, échoué, échoué car il n'y a plus de niveau)
    Map *pMapNew=NULL;	//Pointeur vers une structure Map
    FMOD_CHANNEL *pChannelEnCours=NULL;	//Pour gérer les channels
    char chaine[100];	//Chaîne pour travailler

    /* Si une des boules est tombée, on enlève de la vie, du score, on réinitialise le niveau et on désactive les bonus */
    if(*pEtat == MORT_BORDURE)
    {
        infos.vies--;	//On enlève une vie
        infos.score -= 150;	//On enlève du score
        infos.compteurTemps = 0;
        InitialisationPositions(images, pJoueur, infos.niveau);	//On recommence
        *ppMap = ChargementNiveau(pMoteurRendu, pJoueur, infos.niveau, &etatNiveau);
        infos.bonus &= AUCUN_BONUS;

        for(i=0; i<10; i++)
        {
            descente[i] = false;
        }
    }
    else if (*pEtat >= 0)	//Mort par un missile
    {
        /* On enlève de la vie, du score et on lance l'animation */
        infos.vies--;
        infos.score -= 150;
        infos.compteurTemps = 0;
        *pAjoutAnim = true;
        missileTouche = (char)*pEtat;	//On récupère le numéro du missile qui a explosé

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

        /* On recharge le niveau, on désactive les bonus */
        InitialisationPositions(images, pJoueur, infos.niveau);	//On recommence
        *ppMap = ChargementNiveau(pMoteurRendu, pJoueur, infos.niveau, &etatNiveau);
        infos.bonus &= AUCUN_BONUS;

        for(i=0; i<10; i++)
        {
            descente[i] = false;
        }
    }
    else if (*pEtat == GAGNE)	//Si on a gagné le niveau en cours
    {
        infos.score += 350; //On gagne 350 en score
        infos.score -= Arrondir(0.75*infos.compteurTemps);	//On perd 75% du temps mis pour effectuer le niveau

        /* On gagne 120 en score si on a toutes ses vies de départ, sinon ça dépend de combien il en reste par rapport à combien il y en avait au départ */
        infos.score += Arrondir(infos.vies*(120.0/(double)infos.viesInitiales));
        infos.niveau++;	//Niveau suivant

        /* On charge le niveau suivant dans un autre pointeur */
        pMapNew = ChargementNiveau(pMoteurRendu, pJoueur, infos.niveau, &etatNiveau);

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
                    FMOD_Channel_SetVolume(pChannelEnCours, (float)(Volume/100.0));
                    FMOD_Channel_SetPaused(pChannelEnCours, false);
                }

                Gagne(pMoteurRendu, images, *ppMap, polices);	//On affiche l'écran de fin quand on a gagné

                *pControl = JEU_FIN_GAGNE;		//On sort de la boucle principale
            }
            else if(etatNiveau == CHARGEMENT_FICHIER_CORROMPU || etatNiveau == CHARGEMENT_ERREUR) //Si c'est une erreur de chargement
            {
                *pControl = JEU_FIN_ERREUR_CHARGEMENT;	//On coupe la boucle du jeu depuis ici
            }
        }
        else	//On a chargé le niveau suivant correctement
        {
            sprintf(chaine, "Votre score est de: %ld, passage au niveau %d : %s", infos.score, infos.niveau, pMapNew->titre);
            MessageInformations(chaine, polices, pMoteurRendu, pEntrees);

            /* On ajoute le niveau précédent et son score à la liste en mode campagne */
            if(pJoueur->mode == MODE_CAMPAGNE)
            {
                sprintf(chaine, "%d:%ld;", infos.niveau-1, infos.score);
                strcat(pJoueur->autre, chaine);
            }

            DestructionMap(*ppMap);	//On libère la mémoire prise par l'ancienne structure Map
            *ppMap = pMapNew;	//On copie l'adresse de la nouvelle Map dans l'autre pointeur

            infos.compteurTemps = 0;	//On remet le compteur de temps à 0
            infos.score = 1000;	//On remet le score à 1000
            infos.vies = infos.viesInitiales;	//On remet la vie au maximum pour le prochain niveau

            InitialisationPositions(images, pJoueur, infos.niveau);	//On charge les positions

            *pEtat = RIEN;	//On réinitialise la valeur 'etat' pour que le nouveau niveau ne se termine pas instantanément

            /* On désactive tous les bonus, équivalent à "infos.bonus = infos.bonus & AUCUN_BONUS". On fait un ET bit à bit avec AUCUN_BONUS = 00000000 */
            infos.bonus &= AUCUN_BONUS;

            /* On remet le tableau de booléens 'descente' à false pour le prochain niveau */
            for(i=0; i<10; i++)
            {
                descente[i] = false;
            }
        }
    }

    return 0;
}

void TraitementBonus(FMOD_SYSTEM *pMoteurSon, Sons *pSons)
{
    /* Cette fonction s'occupe des bonus à usage unique */

    FMOD_CHANNEL *pChannelEnCours=NULL;	//Gestion de la musique
    int enLecture=false, i=0;	//Pour savoir si la lecture d'un channel est en cours
    static int dejaActif = 0x0;

    if(BSons)
    {
        /* On vérifie si le son des bonus n'est pas déjà en lecture */
        FMOD_System_GetChannel(pMoteurSon, S_BONUS+10, &pChannelEnCours);
        FMOD_Channel_IsPlaying(pChannelEnCours, &enLecture);

        /* S'il ne l'est pas et que c'est un bonus à usage unique (pas de vitesse ni de saut) alors on le joue, sinon pour les bonus permanents on vérifie, par des opérations bit à bit, qu'ils ne sont pas déjà actifs depuis la dernière fois. Ca permet d'éviter de rejouer en boucle le son*/
        if(!enLecture)
        {
            if(infos.bonus & (BONUS_VIE|BONUS_SCORE_FORT|BONUS_SCORE_FAIBLE))	//Tous les bonus à usage unique
            {
                FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
                FMOD_Channel_SetVolume(pChannelEnCours, (float)(Volume/200.0));
                FMOD_Channel_SetPaused(pChannelEnCours, false);
            }
            else if((infos.bonus & BONUS_VITESSE_BLEUE_FAIBLE) && !(dejaActif & BONUS_VITESSE_BLEUE_FAIBLE))	//Les autres
            {
                FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
                FMOD_Channel_SetVolume(pChannelEnCours, (float)(Volume/200.0));
                FMOD_Channel_SetPaused(pChannelEnCours, false);
                dejaActif |= BONUS_VITESSE_BLEUE_FAIBLE;	//On marque qu'on a déjà joué le son pour ce bonus
            }
            else if((infos.bonus & BONUS_VITESSE_BLEUE_FORT) && !(dejaActif & BONUS_VITESSE_BLEUE_FORT))
            {
                FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
                FMOD_Channel_SetVolume(pChannelEnCours, (float)(Volume/200.0));
                FMOD_Channel_SetPaused(pChannelEnCours, false);
                dejaActif |= BONUS_VITESSE_BLEUE_FORT;
            }
            else if((infos.bonus & BONUS_VITESSE_VERTE_FAIBLE) && !(dejaActif & BONUS_VITESSE_VERTE_FAIBLE))
            {
                FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
                FMOD_Channel_SetVolume(pChannelEnCours, (float)(Volume/200.0));
                FMOD_Channel_SetPaused(pChannelEnCours, false);
                dejaActif |= BONUS_VITESSE_VERTE_FAIBLE;
            }
            else if((infos.bonus & BONUS_VITESSE_VERTE_FORT) && !(dejaActif & BONUS_VITESSE_VERTE_FORT))
            {
                FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
                FMOD_Channel_SetVolume(pChannelEnCours, (float)(Volume/200.0));
                FMOD_Channel_SetPaused(pChannelEnCours, false);
                dejaActif |= BONUS_VITESSE_VERTE_FORT;
            }
            else if((infos.bonus & BONUS_SAUT_BLEUE_FAIBLE) && !(dejaActif & BONUS_SAUT_BLEUE_FAIBLE))
            {
                FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
                FMOD_Channel_SetVolume(pChannelEnCours, (float)(Volume/200.0));
                FMOD_Channel_SetPaused(pChannelEnCours, false);
                dejaActif |= BONUS_SAUT_BLEUE_FAIBLE;
            }
            else if((infos.bonus & BONUS_SAUT_BLEUE_FORT) && !(dejaActif & BONUS_SAUT_BLEUE_FORT))
            {
                FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
                FMOD_Channel_SetVolume(pChannelEnCours, (float)(Volume/200.0));
                FMOD_Channel_SetPaused(pChannelEnCours, false);
                dejaActif |= BONUS_SAUT_BLEUE_FORT;
            }
            else if((infos.bonus & BONUS_SAUT_VERTE_FAIBLE) && !(dejaActif & BONUS_SAUT_VERTE_FAIBLE))
            {
                FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
                FMOD_Channel_SetVolume(pChannelEnCours, (float)(Volume/200.0));
                FMOD_Channel_SetPaused(pChannelEnCours, false);
                dejaActif |= BONUS_SAUT_VERTE_FAIBLE;
            }
            else if((infos.bonus & BONUS_SAUT_VERTE_FORT) && !(dejaActif & BONUS_SAUT_VERTE_FORT))
            {
                FMOD_System_PlaySound(pMoteurSon, S_BONUS+10, pSons->bruits[S_BONUS], true, NULL);
                FMOD_Channel_SetVolume(pChannelEnCours, (float)(Volume/200.0));
                FMOD_Channel_SetPaused(pChannelEnCours, false);
                dejaActif |= BONUS_SAUT_VERTE_FORT;
            }
        }

        /* Dans le cas où il s'agirait d'un nouveau niveau, les bonus étant remis à 0, on oublie tous ceux qu'on avait déjà retenus. Ainsi les sons vont pouvoir être rejoués dans ce nouveau niveau */
        for (i=BONUS_SAUT_VERTE_FAIBLE; i<=BONUS_VITESSE_VERTE_FORT; i <<= 1)
        {
            if (!(infos.bonus & i) && (dejaActif & i))	//Si le bonus n'est pas actuellement actif mais qu'il est marqué comme déjà actif, on le désactive
            {
                dejaActif &= ~i;
            }
        }
    }

    /* Ensuite on s'occupe des bonus à usage unique */
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
    /* Cette fonction augmente le compteur de temps du niveau toute les secondes */

    static unsigned int tempsChrono=0, tempsAncienChrono=0;	//Variables pour retenir les temps
    static int difference=0;	//Variable pour faire la soustraction

    tempsChrono = SDL_GetTicks();	//On prend le temps

    difference = tempsChrono - tempsAncienChrono;	//On calcul la différence

    if(difference >= 1000)
    {
        infos.compteurTemps++;	// S'il s'est écoulé plus d'une seconde on augmente le compteur de 1
        tempsAncienChrono = tempsChrono;
    }
    else if(difference < 0)	//Si la différence est négative on remet tempsAncien à zéro (c'est à cause des variables statiques)
    {
        tempsAncienChrono = 0;
    }
}

int Affichage(SDL_Renderer *pMoteurRendu, Sprite images[], TTF_Font *polices[], unsigned char descente[], Map* pMap, Animation anim[], unsigned char *pAjoutAnim)
{
    SDL_Rect posFond;	//Position et taille de l'image de fond

    /* Affectation */
    posFond.h = (int)Hauteur;
    posFond.w = (int)Largeur;
    posFond.x = 0;
    posFond.y = 0;

    /* On efface l'écran */
    SDL_SetRenderDrawColor(pMoteurRendu, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(pMoteurRendu);

    SDL_RenderCopy(pMoteurRendu, pMap->fond, NULL, &posFond);	//On copie le fond

    AffichageMap(pMoteurRendu, pMap);	//On copie la map

    AffichageBonus(pMoteurRendu, pMap, images);	//On copie les bonus

    AffichageImages(pMoteurRendu, images, anim, descente, pAjoutAnim);	//On copie les images (boules, vortex et missiles)

    AffichageVies(pMoteurRendu, images);	//On copie la vie

    AffichageTextes(pMoteurRendu, polices, images[FOND_TEXTES].pTextures[0]);	//On copie le fond pour le texte de temps et de score

    SDL_RenderPresent(pMoteurRendu);         //Mise à jour de l'écran

    return 0;
}

int AffichageMap(SDL_Renderer *pMoteurRendu, Map *pMap)
{
    int i, j;	//Compteurs
    SDL_Rect pos;	//Position et taille

    /* Affectation de la taille */
    pos.h = TailleBloc;
    pos.w = TailleBloc;

    /* On parcourt la map à l'aide d'une double boucle */
    for(i=0; i< pMap->nbtiles_largeur_monde; i++)
        for(j=0; j< pMap->nbtiles_hauteur_monde; j++)
        {
            /* On affecte la position en fonction de i et j */
            pos.x = i*TailleBloc;
            pos.y = j*TailleBloc;

            /* On regarde ce qu'il y a à cet endroit et on copie ce qui correspond (il y a de nombreuses tiles inutilisées car je n'arrive pas à gérer leur collision pour l'instant) */
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

            case VIDE:	//On ne met rien
                break;
            }
        }

        return 0;
}

int AffichageVies(SDL_Renderer *pMoteurRendu, Sprite images[])
{
    int i;	//Compteur

    /* On regarde d'abord combien il y a de vie au total, puis combien il en reste */
    switch(infos.viesInitiales)
    {
    case 1:
        switch(infos.vies)
        {
        case 1:
            SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);	//Une vie entière
            break;

        case 0:
            SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[0]);	//Une vie vide
            break;
        }

        break;

    case 2:
        switch(infos.vies)
        {
        case 2:
            for (i=0; i<2; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//2 vies entières
            }

            break;

        case 1:
            SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[1]);	//Une vie vide
            SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);	//Une vie entière
            break;

        case 0:
            for (i=0; i<2; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//2 vies vides
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
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//3 vies entières
            }

            break;

        case 2:
            for (i=0; i<2; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//2 vies entières
            }

            SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[2]);	//Une vie vide
            break;

        case 1:
            for (i=1; i<3; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//2 vies vides
            }

            SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);	//Une vie entière
            break;

        case 0:
            for (i=0; i<3; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//3 vies vides
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
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//4 vies entières
            }

            break;

        case 3:
            for (i=0; i<3; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//3 vies entières
            }

            SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[3]);	//Une vie vide
            break;

        case 2:
            for (i=0; i<2; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//2 vies entières
            }

            for (i=2; i<4; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//2 vies vides
            }

            break;

        case 1:
            for (i=1; i<4; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//3 vies vides
            }

            SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);	//Une vie entière
            break;

        case 0:
            for (i=0; i<4; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//4 vies vides
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
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//5 vies entières
            }

            break;

        case 4:
            for (i=0; i<4; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//4 vies entières
            }

            SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[4]);	//Une vie vide
            break;

        case 3:
            for (i=0; i<3; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//3 vies entières
            }

            for (i=3; i<5; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//2 vies vides
            }

            break;

        case 2:
            for (i=0; i<2; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[i]);	//2 vies entières
            }

            for (i=2; i<5; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//3 vies vides
            }

            break;

        case 1:
            for (i=1; i<5; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//4 vies vides
            }

            SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[0], NULL, &images[VIE].position[0]);	//Une vie entière
            break;

        case 0:
            for (i=0; i<5; i++)
            {
                SDL_RenderCopy(pMoteurRendu, images[VIE].pTextures[1], NULL, &images[VIE].position[i]);	//5 vies vides
            }

            break;
        }

        break;
    }

    return 0;
}

int AffichageBonus(SDL_Renderer *pMoteurRendu, Map *pMap, Sprite images[])
{
    int i, j;	//Compteurs

    /* On parcourt la map des bonus avec une double boucle */
    for (i=0; i<pMap->nbtiles_largeur_monde; i++)
    {
        for (j=0; j<pMap->nbtiles_hauteur_monde; j++)
        {
            /* On affecte la position en fonction de i et j et on centre le bonus dans la case */
            images[GEMMES].position[0].x = Arrondir(i*TailleBloc + (TailleBloc * 0.3)/2.0);
            images[GEMMES].position[0].y = Arrondir(j*TailleBloc + (TailleBloc * 0.3)/2.0);

            /* Ensuite on regarde ce qu'il y a dans la case et on copie ce qui correspond */
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
            }
        }
    }

    return 0;
}

int AffichageImages(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], unsigned char descente[], unsigned char *pAjoutAnim)
{
    static unsigned int angleVortex=360;	//Angle pour faire tourner les vortex
    int i=0;	//Compteur
    static const SDL_Point pointOrigine= {0, 0};	//Point à partir duquel faire la rotation (0;0)

    angleVortex -= 10;	//On diminue l'angle du vortex pour le faire tourner

    /* S'il arrive à zéro (ou bien au dessus de 360° car ici la variable est non-signée, soit 0-1 = 4 294 967 296) on le remet à 360 */
    if (angleVortex==0 || angleVortex>360)
    {
        angleVortex=360;
    }

    /* On copie les vortex */
    SDL_RenderCopyEx(pMoteurRendu, images[VORTEX_BLEU].pTextures[0], NULL, &images[VORTEX_BLEU].position[0], angleVortex, NULL, SDL_FLIP_NONE);
    SDL_RenderCopyEx(pMoteurRendu, images[VORTEX_VERT].pTextures[0], NULL, &images[VORTEX_VERT].position[0], angleVortex, NULL, SDL_FLIP_NONE);

    for(i=0; i < MISSILE; i++)
    {
        SDL_RenderCopy(pMoteurRendu, images[i].pTextures[0], NULL, &images[i].position[0]);        //Collage des surfaces autres que les missiles
    }

    /* S'il y a une animation, on la lit */
    if(*pAjoutAnim)
    {
        if (LectureAnimation(pMoteurRendu, anim, ANIM_0) == -1)
        {
            *pAjoutAnim = false;	//Puis on désactive quand elle est finie
        }
    }
    else	//Sans animation, on copie les missiles
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
                SDL_RenderCopyEx(pMoteurRendu, images[MISSILE].pTextures[0], NULL, &images[MISSILE].position[i], 90, &pointOrigine, SDL_FLIP_NONE);	// On retourne le missile quand il va vers la droite
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
    /* Cette fonction affiche les textes de compteur de temps et de score */

    char chaineTemps[50]="", chaineScore[50]="";	//Chaînes pour contenir
    SDL_Color blancOpaque= {255, 255, 255, 255};
    static SDL_Rect posTemps, posScore, posFond;	//Différentes positions et tailles
    SDL_Surface *pSurfTemps=NULL, *pSurfScore=NULL;	//Pointeurs sur des surfaces
    SDL_Texture *pTextureTemps=NULL, *pTextureScore=NULL;	//Pointeurs sur des textures

    /* On définit les positions */
    posFond.x = posFond.y=0;
    posTemps.x = posScore.x = Arrondir(0.008*Largeur);
    posTemps.y=0;
    posScore.y = Arrondir(0.023*Largeur);

    /* On écrit les valeurs dans les chaînes */
    sprintf(chaineTemps, "%d", infos.compteurTemps);
    sprintf(chaineScore, "%ld", infos.score);

    /* On crée les surfaces, on définit les tailles en fonction de la longueur du texte */
    pSurfTemps = TTF_RenderText_Solid(polices[POLICE_ARIAL], chaineTemps, blancOpaque);
    TTF_SizeText(polices[POLICE_ARIAL], chaineTemps, &posTemps.w, &posTemps.h);
    posTemps.w = Arrondir(((double)posTemps.w/1280.0) * Largeur);
    posTemps.h = Arrondir(((double)posTemps.h/1280.0) * Largeur);

    pSurfScore = TTF_RenderText_Solid(polices[POLICE_ARIAL], chaineScore, blancOpaque);
    TTF_SizeText(polices[POLICE_ARIAL], chaineScore, &posScore.w, &posScore.h);
    posScore.w = Arrondir(((double)posScore.w/1280.0) * Largeur);
    posScore.h = Arrondir(((double)posScore.h/1280.0) * Largeur);

    /* On définit la taille du fond des textes en fonction de celle de ceux ci */
    posFond.h = posScore.y + posScore.h + Arrondir(0.008*Largeur);
    posFond.w = posScore.x + posScore.w + Arrondir(0.008*Largeur);

    /* On convertit les surfaces en textures */
    pTextureTemps = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfTemps);
    pTextureScore = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfScore);

    /* On copie les textures */
    SDL_RenderCopy(pMoteurRendu, pTextureFond, NULL, &posFond);
    SDL_RenderCopy(pMoteurRendu, pTextureTemps, NULL, &posTemps);
    SDL_RenderCopy(pMoteurRendu, pTextureScore, NULL, &posScore);

    /* On détruit les surfaces */
    SDL_FreeSurface(pSurfScore);
    SDL_FreeSurface(pSurfTemps);

    /* On détruit les textures */
    SDL_DestroyTexture(pTextureScore);
    SDL_DestroyTexture(pTextureTemps);

    return 0;
}

int InitialisationPositions(Sprite images[], Joueur *pJoueur, int level)
{
    /* Cette fonction initialise les positions et les tailles des différentes images selon le mode de jeu, l'édition d'un niveau ou la création d'un niveau */
    int i=0, j=0, k=0;	//Compteurs
    double nb;	//Nombre décimal
    char *c = NULL;	//Pointeur sur un caractère pour faire des recherches

    /* Si on est en mode campagne, en mode perso ou en mode édition on va charger depuis un fichier */
    if (pJoueur->mode == MODE_CAMPAGNE || pJoueur->mode == MODE_PERSO || (pJoueur->mode == MODE_EDITEUR && level != -1))
    {
        FILE *pFichierNiveau = NULL;
        char ligne[20] = "";

        /* Mode campagne c'est le fichier de campagne (le md5 a déjà été vérifié lors du chargement de la map, juste avant) */
        if(pJoueur->mode == MODE_CAMPAGNE)
        {
            pFichierNiveau = fopen("ressources/level.lvl", "r");
        }
        else if(pJoueur->mode == MODE_PERSO || pJoueur->mode == MODE_EDITEUR)	//Mode perso ou édition d'un niveau
        {
            pFichierNiveau = fopen("ressources/levelUser.lvl", "r");
        }

        /* Vérification de l'ouverture du fichier */
        if (pFichierNiveau == NULL)
        {
            return -1;
        }

        /* On passe tous les niveaux jusqu'à atteindre la ligne de celui juste avant celui que l'on souhaite charger */
        for (i=0; i<level-1; i++)
        {
            fgets(ligne, 20, pFichierNiveau);

            do
            {
                fgets(ligne, 20, pFichierNiveau);
            }
            while (strcmp(ligne, "##--##\n") != 0);
        }

        k=0;	//on remet k à 0

        /* On recherche la ligne où les premières positions sont enregistrées (celles des missiles V) */
        while(k++, strcmp(ligne, "#missileV\n") != 0)
        {
            fgets(ligne, 20, pFichierNiveau);
        }

        /* On lit les coordonnées des 5 missiles V */
        for(i=0; i<5; i++)
        {
            fgets(ligne, 20, pFichierNiveau);

            nb = strtod(ligne, NULL);	//On convertit la chaîne en nombre décimal

            /* Selon le système utilisé, il faut une virgule et non un point dans la chaîne pour former un nombre décimal correct */
            if(nb == 0)
            {
                c = strstr(ligne, ".");
                *c = ',';
                nb = strtod(ligne, NULL);
            }

            images[MISSILE].position[i].x = Arrondir(nb*Largeur);	//Enfin on multiplie par la largeur et on arrondit au pixel

            /* On recommence pour la coordonnée en y */
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

        /* On lit la ligne quivante qui contient normalement "#missileH\n" */
        fgets(ligne, 20, pFichierNiveau);

        /* On lit les coordonnées des 5 missiles H */
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

        /* Ensuite on vérifie bien qu'on s'apprète à lire les coordonnées des vortex */
        if(fgets(ligne, 20, pFichierNiveau), strcmp(ligne, "#vortex\n") != 0)
        {
            return -1;
        }

        /* Coordonnées du vortex bleu */
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

        /* Coordonnées du vortex vert */

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

        /* Ensuite on vérifie que c'est bien les boules */
        if (fgets(ligne, 20, pFichierNiveau), strcmp(ligne, "#boules\n") != 0)
        {
            return -1;
        }

        /* Coordonnées de la boule bleue */
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

        /* Coordonnées de la boule magenta */
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

        /* Coordonnées de la boule verte */
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

        /* On ferme le fichier */
        fclose(pFichierNiveau);

        /* On définit les positions des vies dans les deux modes de jeu */
        if(pJoueur->mode == MODE_CAMPAGNE || pJoueur->mode == MODE_PERSO)
        {
            for(i=0; i<infos.viesInitiales; i++)
            {
                images[VIE].position[i].x = Arrondir(Largeur - (double)(i+1)*0.05*Largeur);
                images[VIE].position[i].y = 0;
            }
        }

        /* Tailles des vies */
        for (i=0; i<infos.viesInitiales; i++)
        {
            images[VIE].position[i].h = images[VIE].position[i].w = Arrondir(0.04*Largeur);
        }
    }
    else if(pJoueur->mode == MODE_EDITEUR && level == -1)	//Sinon si on est en mode éditeur et qu'on ajoute un nouveau niveau
    {
        for(i=0; i<10; i++)	//On cache les missiles en dehors de la fenêtre
        {
            images[MISSILE].position[i].x =	images[MISSILE].position[i].y = -2000;
        }

        /* On définit les positions par défaut des boules et des vortex */
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

    if(pJoueur->mode == MODE_EDITEUR)	//Pour tous les éditeurs (ajout ou édition) on place les boutons et les gemmes bonus
    {
        /* On définit les positions des boutons d'ajout de missiles */
        images[AJOUTER_MISSILE_H].position[0].x = Arrondir(0.900*Largeur);
        images[AJOUTER_MISSILE_H].position[0].y = Arrondir(0.055*Hauteur);
        images[AJOUTER_MISSILE_V].position[0].x = Arrondir(0.900*Largeur);
        images[AJOUTER_MISSILE_V].position[0].y = Arrondir(0.120*Hauteur);

        for (k=1, i=0; i<3; i++)
        {
            for (j=0; j<6; j++, k++)
            {
                images[GEMMES].position[k].x = 91*j;
                images[GEMMES].position[k].y = 78*i;
            }
        }

        /* Tailles du curseur */
        images[CURSEUR].position[0].w = Arrondir(0.02*Largeur);
        images[CURSEUR].position[0].h = Arrondir(0.027*Largeur);
    }

    /* Initialisation des tailles des boules */
    images[BOULE_BLEUE].position[0].h = TailleBoule;
    images[BOULE_BLEUE].position[0].w = TailleBoule;
    images[BOULE_MAGENTA].position[0].h = TailleBoule;
    images[BOULE_MAGENTA].position[0].w = TailleBoule;
    images[BOULE_VERTE].position[0].h = TailleBoule;
    images[BOULE_VERTE].position[0].w = TailleBoule;

    /* On initialise les tailles des missiles */
    for(i=0; i<11; i++)
    {
        images[MISSILE].position[i].h = TailleMissileH;
        images[MISSILE].position[i].w = TailleMissileW;
    }

    /* Celles des vortex */
    images[VORTEX_BLEU].position[0].w = Arrondir(TailleBloc*1.7);
    images[VORTEX_BLEU].position[0].h = Arrondir(TailleBloc*1.7);
    images[VORTEX_VERT].position[0].w = Arrondir(TailleBloc*1.7);
    images[VORTEX_VERT].position[0].h = Arrondir(TailleBloc*1.7);

    /* Celles des bonus */
    images[GEMMES].position[0].w = Arrondir(TailleBloc - (TailleBloc * 0.3));
    images[GEMMES].position[0].h = Arrondir(TailleBloc - (TailleBloc * 0.3));

    /* Tailles des bonus pour le découpage sur l'image originale */
    for (i=0; i<18; i++)
    {
        images[GEMMES].position[i+1].h = 78;
        images[GEMMES].position[i+1].w = 91;
    }

    return 0;
}

int SautBleue(SDL_Rect *pPosition, unsigned char *pSautEnCours)
{
    /* Cette fonction renvoie les valeurs successives de sauts pour la boule bleue d'après une formule de physique */

    /* Les variables sont statiques pour que le saut se poursuive d'appel en appel */
    static int positionRelative=0, positionRelativeAncienne=0, positionFinale=0;
    static int positionInitiale=0, temps=0;
    double vitesse_initiale=0.0;

    /* La vitesse initiale de la boule est assignée en fonction des bonus */
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

    /* Si c'est la première fois (= commencement du saut), on retient la position à laquelle la boule doit revenir en retombant, et on met le booléen à 'true' pour que le saut se poursuive ensuite de lui même */
    if(*pSautEnCours == false)
    {
        positionInitiale = pPosition->y;
        temps = positionRelativeAncienne = 0;
        *pSautEnCours = true;
    }

    /* On calcule la position relative à celle de base (sur l'axe y) par la formule : -(G/2)*t² + vx*t + 0, que l'on divise par 600 puis que l'on multiplie par la hauteur pour faire la mise à l'échelle selon la résolution choisie */
    positionRelative = Arrondir(((double)((vitesse_initiale * temps)-((_G_ * temps * temps)/2000)) /600.0)*Hauteur);

    /* On calcule de combien de pixels la boule doit finalement montée en soustrayant de combien elle est déjà montée à l'appel précédent */
    positionFinale = positionRelative - positionRelativeAncienne;
    positionRelativeAncienne = positionRelative;	//On retient cette valeur pour la soustaire lors de l'appel suivant

    if(pPosition->y - positionRelative > positionInitiale)	//Si on est revenu à notre position de départ (ou plus bas), on coupe le saut, on remet tout à 0
    {
        temps = 0;
        *pSautEnCours = false;
        positionInitiale = 0;
        return 0;
    }

    temps += 5;	//Sinon le temps augmente de 5

    return (-positionFinale);	//On retourne - la valeur calculée car le repère de l'écran est inversé pour l'axe en y
}

int SautVerte(SDL_Rect *pPosition, unsigned char *pSautEnCours)
{
    /* Cette fonction fait la même chose que la précédente, mais pour la boule verte. Elle retourne donc une valeur positive */

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

    return positionFinale;	//Valeur positive
}

void DetectionBonus (Sprite images[], int indiceImage, Map* pMap)
{
    /* Cette fonction fait une boucle sur tous les bonus, puis regarde si un des 4 coins de l'image passée en paramètre est sur une case avec un bonus */

    int i;	//Compteur

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
    /* Cette fonction est appelée quand on a plus de vie, elle fait sa propre boucle d'affichage */

    SDL_Surface *psFondPerdu = NULL;	//Pointeur sur la surface de fond
    SDL_Texture *pFondPerdu = NULL;	//Pointeur sur la texture de fond
    Texte information;	//Structure qui contient des chaînes, des textures, des positions et des tailles

    ClavierSouris entrees;
    SDL_Color color = {255, 255, 255, SDL_ALPHA_OPAQUE};	//Couleur blanche
    int rmask, gmask, bmask, amask, differenceFPS=0, i=0;
    unsigned int tempsFPS=0, tempsAncienFPS=0;	//Temps

    /* On remplie les chaînes avec ce que l'on veut afficher */
    sprintf(information.chaines[0], "PERDU");
    sprintf(information.chaines[1], "Niveau: %d", infos.niveau);
    sprintf(information.chaines[2], "Temps mis pour ce niveau: %d", infos.compteurTemps);
    sprintf(information.chaines[3], "Score: %ld", infos.score);

    EntreesZero(&entrees);	//On initialise la structure avec les entrées

    /* Masques RGBA */
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;

    /* On crée le fond orange transparent et on le convertit en texture */
    psFondPerdu = SDL_CreateRGBSurface(0 , (int)Largeur, (int)Hauteur, 32, rmask, gmask, bmask, amask);
    SDL_FillRect(psFondPerdu, NULL, SDL_MapRGBA(psFondPerdu->format, 200, 125, 75, 128));
    pFondPerdu = SDL_CreateTextureFromSurface(pMoteurRendu, psFondPerdu);

    SDL_FreeSurface(psFondPerdu);	//Libération

    /* On crée des surfaces à partir des chaînes, on définit les tailles, les positions (on centre), on crée la texture correspondante et on libère la mémoire */
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

    /* On lance la boucle d'affichage tant qu'on appuie pas sur ESPACE, ENTREE, ECHAP et que l'on ne ferme pas */
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

    entrees.clavier[ECHAP] = entrees.clavier[ESPACE] = entrees.clavier[ENTREE] = false;	//On remet à 0 pour éviter que le message ne saute tout de suite

    MessageInformations("Oh non, les boules ont péri, brisées par les missiles !", polices, pMoteurRendu, &entrees);	//Message (boucle d'affichage interne)

    return 0;
}

int PerduAffichage(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], SDL_Texture *pFondPerdu, Map *pMap, Texte *pInformation, unsigned char *pAjoutAnim)
{
    /* Affichage de la fonction Perdu */

    static SDL_Rect pos, posFond;	//Positions du fond et des blocs pour la map en arrière plan
    int i=0;	//Compteur

    pos.h = pos.w = TailleBloc;	//Taille est celle des blocs

    /* Taille de l'écran */
    posFond.h = (int)Hauteur;
    posFond.w = (int)Largeur;
    posFond.x = posFond.y = 0;	//(0;0)

    SDL_SetRenderDrawColor(pMoteurRendu, 0, 0, 0, SDL_ALPHA_OPAQUE); //Remplissage en noir pour effacer
    SDL_RenderClear(pMoteurRendu);

    SDL_RenderCopy(pMoteurRendu, pMap->fond, NULL, &posFond);	//On copie le fond

    SDL_RenderCopy(pMoteurRendu, images[VORTEX_BLEU].pTextures[0], NULL, &images[VORTEX_BLEU].position[0]);        //Collage des surfaces des vortex
    SDL_RenderCopy(pMoteurRendu, images[VORTEX_VERT].pTextures[0], NULL, &images[VORTEX_VERT].position[0]);

    AffichageMap(pMoteurRendu, pMap);	//On affiche la map

    AffichageBonus(pMoteurRendu, pMap, images);	//On affiche les bonus

    AffichageVies(pMoteurRendu, images);	//Les vies (0)

    /* Si la dernière mort est dûe à un missile, on lit l'animation */
    if(*pAjoutAnim)
    {
        if (LectureAnimation(pMoteurRendu, anim, ANIM_0) == -1)
        {
            *pAjoutAnim = false;
        }
    }
    else	//Sinon on affiche notre fond orange transparent avec ses informations
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
    /* Cette fonction est appelée pour lire une succession d'images */

    static int temps=0, tempsAncien=0, k=0;	//Temps + compteur d'images

    temps = SDL_GetTicks();	//On prend le temps

    /* Si le temps depuis le dernier changement d'image est suffisant et qu'il y a une image valide après */
    if (anim[animNB].img[k] != NULL && temps - tempsAncien > T_ANIM)
    {
        SDL_RenderCopy(pMoteurRendu, anim[animNB].img[k], NULL, &anim[animNB].pos);	//On affiche l'image suivante
        k++;	//On passe à celle d'après
        tempsAncien = temps;
    }
    else if (anim[animNB].img[k] != NULL)	//Sinon si l'image suivante est valide mais que le temps requis ne s'est pas écoulé, on réaffiche la même
    {
        SDL_RenderCopy(pMoteurRendu, anim[animNB].img[k], NULL, &anim[animNB].pos);
    }
    else /* Si l'image suivante n'est pas correcte, l'animation est terminée, on remet k et les temps à 0 pour la prochaine fois et on signale la fin de l'animation */
    {
        k=0;
        temps = tempsAncien = 0;
        return -1;
    }

    return 0;
}

int Gagne(SDL_Renderer *pMoteurRendu, Sprite images[], Map *pMap, TTF_Font *polices[])
{
    /* Cette fonction est appelée quand on a gagné tous les niveaux */

    SDL_Surface *pSurfFondGagne = NULL;	//Surface
    SDL_Texture *pTextureFondGagne = NULL;	//Texture

    Texte information;	//Chaînes + textures + positions

    ClavierSouris entrees;
    SDL_Color color = {255, 255, 255, SDL_ALPHA_OPAQUE};
    int rmask, gmask, bmask, amask, i=0, differenceFPS=0;
    unsigned int tempsFPS=0, tempsAncienFPS=0;	//Temps

    /* On écrit les infos dans les chaînes */
    sprintf(information.chaines[0], "GAGNÉ");
    sprintf(information.chaines[1], "Niveau: %d", infos.niveau -1);
    sprintf(information.chaines[2], "Temps mis pour ce niveau: %d", infos.compteurTemps);
    sprintf(information.chaines[3], "Score: %ld", infos.score);
    sprintf(information.chaines[4], "Vies: %d sur %d", infos.vies, infos.viesInitiales);

    EntreesZero(&entrees);	//Mise à 0

    /* Masques RGBA */
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;

    /* On crée la surface de fond orangé et on la convertie en texture */
    pSurfFondGagne = SDL_CreateRGBSurface(0 , (int)Largeur, (int)Hauteur, 32, rmask, gmask, bmask, amask);
    SDL_FillRect(pSurfFondGagne, NULL, SDL_MapRGBA(pSurfFondGagne->format, 200, 125, 75, 128));
    pTextureFondGagne = SDL_CreateTextureFromSurface(pMoteurRendu, pSurfFondGagne);

    SDL_FreeSurface(pSurfFondGagne);	//Libération

    /* On crée les surfaces, puis les textures à partir des chaînes */
    for (i=0; i<5; i++)
    {
        information.surface = TTF_RenderText_Blended(polices[POLICE_SNICKY_GRAND], information.chaines[i], color);
        TTF_SizeText(polices[POLICE_SNICKY_GRAND], information.chaines[i], &information.positions[i].w, &information.positions[i].h);
        information.positions[i].w = Arrondir(((double)information.positions[i].w / 1280.0) * Largeur);
        information.positions[i].h = Arrondir(((double)information.positions[i].h / 1280.0) * Largeur);

        information.positions[i].x = Arrondir((Largeur/2.0) - (information.positions[i].w/2));
        information.positions[i].y = Arrondir(0.05*Largeur + 0.125*Largeur*(double)i);

        information.pTextures[i] = SDL_CreateTextureFromSurface(pMoteurRendu, information.surface);

        SDL_FreeSurface(information.surface);
    }

    /* Affichage */
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

    MessageInformations("Bravo! Vous avez sauvé les boules.", polices, pMoteurRendu, &entrees);	//Message

    return 0;
}

int GagneAffichage(SDL_Renderer *pMoteurRendu, Sprite images[], SDL_Texture *pTextureFondGagne, Map *pMap, Texte *pInformation)
{
    /* Affichage de la fonction Gagne */

    static SDL_Rect pos, posFond;	//Positions
    int i=0;	//Compteur

    /* On affecte les tailles et les positions */
    pos.h = pos.w = TailleBloc;

    posFond.h = (int)Hauteur;
    posFond.w = (int)Largeur;
    posFond.x = posFond.y = 0;

    SDL_SetRenderDrawColor(pMoteurRendu, 0, 0, 0, SDL_ALPHA_OPAQUE); //Remplissage en noir pour effacer le fond
    SDL_RenderClear(pMoteurRendu);

    SDL_RenderCopy(pMoteurRendu, pMap->fond, NULL, &posFond);	//On copie le fond

    SDL_RenderCopy(pMoteurRendu, images[VORTEX_BLEU].pTextures[0], NULL, &images[VORTEX_BLEU].position[0]);        //Collage des surfaces des vortex
    SDL_RenderCopy(pMoteurRendu, images[VORTEX_VERT].pTextures[0], NULL, &images[VORTEX_VERT].position[0]);

    AffichageMap(pMoteurRendu, pMap);	//Affichage de la map

    AffichageBonus(pMoteurRendu, pMap, images);	//Des bonus

    AffichageVies(pMoteurRendu, images);	//Des vies (>0)

    SDL_RenderCopy(pMoteurRendu, pTextureFondGagne, NULL, &posFond);	//Du fond orangé

    for(i=0; i<5; i++)	//Affichage des informations
    {
        SDL_RenderCopy(pMoteurRendu, pInformation->pTextures[i], NULL, &pInformation->positions[i]);
    }

    SDL_RenderPresent(pMoteurRendu);         //Mise à jour de l'écran

    return 0;
}
//Fin du fichier jeu.c
