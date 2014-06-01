/*
Projet-ISN

Fichier: collision.h

Contenu: Prototypes des fonctions contenues dans collision.c.

Actions: Permet à l'ordinateur de connaître toutes les fonctions présentes dans le programme ainsi que leurs arguments. Ici se trouvent les fonctions qui s'occupent de gérer les collisions.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef DEF_COLLISION_H	//Protection contre les inclusions infinies
#define DEF_COLLISION_H

unsigned int CollisionBordure (Sprite images[], int indiceImage);
unsigned int CollisionDecor (Sprite images[], int indiceImage, Map* pMap);
void CollisionDetect(Sprite images[], int indiceImage, Map *pMap, Collision *pCollision);
int CollisionImage (Sprite images[], int indiceImage, Collision *pCollision);

#endif

//Fin du fichier collision.h