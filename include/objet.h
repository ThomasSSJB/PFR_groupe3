#ifndef OBJET_H
#define OBJET_H

#include "../include/image.h"
#include "../include/forme.h"

typedef struct Objet * Objet;

/* Initialise un objet (allocation mémoire) */
Objet init_objet(Couleur couleur, Forme forme);

/* Renvoie la sous-image centrée sur l'objet */
Image sous_image_objet(Image image, int valeur_objet, int delta);

/* Trouve la couleur d'un objet */
Couleur trouver_couleur_objet(Image sous_image);


void commande_balle(const char* direction, float angle, float distance, char L[][64], int* nb_cmd);
#endif