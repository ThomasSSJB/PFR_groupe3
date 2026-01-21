#ifndef FORME_H
#define FORME_H

#include "image.h"
#define PI 3.14

typedef struct Forme * Forme;

/* Initialise une forme (allocation mémoire) */
Forme init_forme(char* nature, int x_dep, int y_dep, int x_arr, int y_arr);

/* Calcul les coordonnées du coin supérieur gauche et du coin inférieur droit de la boîte englobante d'un objet sur une image */
int* trouver_coordonnees_forme(Image image, int valeur_objet);

/* Permet de savoir si un objet est un cercle ou non */
char* trouver_nature_forme(Image image);

/* getteurs */
int get_x_dep(Forme forme);
int get_y_dep(Forme forme);
int get_x_arr(Forme forme);
int get_y_arr(Forme forme);

#endif