#ifndef OBJET_H
#define OBJET_H

#define PI 3.14

#include "../include/image.h"

typedef struct Objet * Objet;

/* Initialise un objet (allocation mémoire) */
Objet init_objet(int id);

/* Affiche les coordonées d'un objet */
void afficher_coordonnees_objet(Objet objet);

/* Affiche la nature de la forme de l'objet */
void afficher_nature_objet(Objet objet);

/* Affiche la couleur de l'objet */
void afficher_couleur_objet(Objet objet);

/* Renvoie la sous-image centrée sur l'objet */
Image sous_image_objet(Objet objet, Image image, int delta);

/* Calcul les coordonnées du coin supérieur gauche et du coin inférieur droit de la boîte englobante d'un objet sur une image */
void trouver_coordonnees_objet(Objet objet, Image image);

/* Affecte à un objet la nature de sa forme : 'Cercle' ou 'Autre' */
void trouver_nature_objet(Objet objet, Image image);

/* Trouve la couleur d'un objet */
void trouver_couleur_objet(Objet objet, Image image);

/* Trouve où se situe l'objet sur l'image (renvoie "gauche", "milieu" ou "droite") */
char* trouver_direction_objet(Objet objet, Image image);

/* Trouve la distance réelle en mètre(approximation) entre la prise de la photo et un objet */
int trouver_distance_objet(Objet objet, Image image);

/* Trouve l'angle réelle en dégré (approximation) entre la prise de la photo et un objet */
int trouver_angle_objet(Objet objet, Image image);

/* getteurs */
Couleur get_couleur_objet(Objet objet);
char* get_nature_forme_objet(Objet objet);
int get_x_dep(Objet objet);
int get_y_dep(Objet objet);
int get_x_arr(Objet objet);
int get_y_arr(Objet objet);


void commande_balle(const char* direction, int angle, int distance);

#endif