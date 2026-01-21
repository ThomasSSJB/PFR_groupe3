#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/image.h"
#include "../include/forme.h"
#include "../include/config.h"

struct Forme {
  char* nature;
  int x_dep;
  int y_dep;
  int x_arr;
  int y_arr;
};


Forme init_forme(char* nature, int x_dep, int y_dep, int x_arr, int y_arr) {
  Forme forme = malloc(sizeof(struct Forme));
  forme->nature = nature;
  forme->x_dep = x_dep;
  forme->y_dep = y_dep;
  forme->x_arr = x_arr;
  forme->y_arr = y_arr;
  return forme;
}


int* trouver_coordonnees_forme(Image image, int valeur_objet) {
  /* Initialisation du tableau à retourner */
  int* tab = (int*) malloc(4*sizeof(int));

  /* Initialisation des "coins" de l'objet */
  int i_min = get_largeur(image);
  int j_min = get_hauteur(image);
  int i_max = -1;
  int j_max = -1;

  /* Récupération de la matrice binarisée */
  int** matrice_binarisee = binariser_image(image);
  int** matrice_labellisee = labelliser_image_binaire(matrice_binarisee, get_largeur(image), get_hauteur(image));

  /* Calcul des coordonnées des "coins" de l'objet */
  for (int i=0 ; i<get_largeur(image); i++) {
    for (int j=0 ; j<get_hauteur(image) ; j++) {
      if (matrice_labellisee[i][j] == valeur_objet) {
        if (i < i_min) { i_min = i; }
        else if (j < j_min) { j_min = j; }
        else if (i > i_max) { i_max = i; }
        else if (j > j_max) { j_max = j; }
      }
    }
  }

  /* Affectation des valeurs au tableau */
  tab[0] = i_min;
  tab[1] = j_min;
  tab[2] = i_max;
  tab[3] = j_max;

  return tab;
}


char* trouver_nature_forme(Image image) {
  /* Charge la valeur du seuil pour la forme (cerlce) depuis la configuration */
  float seuil_forme = lire_valeur_json("seuil_forme", config_json) / (float) 100;
  // printf("[TRACE] seuil_forme = %.2f\n", seuil_forme);

  /* Initialisation des variables */
  char* nature_forme = "Autre";
  int** matrice_binarisee = binariser_image(image);
  int aire = 0;
  int perimetre = 0;
  float circularite;

  /* Calcul de l'aire (nombre de pixel à 1) et du périmètre (nombre de pixels à 1 sur les contours) */
  for (int i=0 ; i<get_largeur(image) ; i++) {
    for (int j=0 ; j<get_hauteur(image) ; j++) {
      if (matrice_binarisee[i][j]) {
        aire++;
        if (i == 0 || j == 0 || i == get_largeur(image)-1 || j == get_hauteur(image)-1) perimetre++;
        else if (matrice_binarisee[i-1][j] == 0 || matrice_binarisee[i+1][j] == 0 || matrice_binarisee[i][j-1] == 0 || matrice_binarisee[i][j+1] == 0) perimetre++;
      }
    }
  }

  /* Calcul de la circularité de l'forme */
  if (perimetre > 0) circularite = 4*PI*aire / (float) (perimetre*perimetre);
  else circularite = 0;

  // printf("[TRACE] perimetre = %d\n", perimetre);
  // printf("[TRACE] aire = %d\n", aire);
  // printf("[TRACE] circularite = %.2f\n", circularite);

  /* Détermination de la nature de l'forme (cercle ou pas) */
  if (circularite > seuil_forme && circularite < (1+seuil_forme)) {
    nature_forme = "Cercle";
  }

  /* Libération mémoire */
  for (int i = 0 ; i < get_largeur(image) ; i++)
    free(matrice_binarisee[i]);
  free(matrice_binarisee);

  
  /* Retourne la nature de la forme */
  return nature_forme;
}


int get_x_dep(Forme forme) {
  return forme->x_dep;
}


int get_y_dep(Forme forme) {
  return forme->y_dep;
}


int get_x_arr(Forme forme) {
  return forme->x_arr;
}


int get_y_arr(Forme forme) {
  return forme->y_arr;
}