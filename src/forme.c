#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/image.h"
#include "../include/forme.h"

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


int* trouver_coordonnees_forme(Image image) {
  /* Initialisation du tableau à retourner */
  int* tab = (int*) malloc(4*sizeof(int));

  /* Initialisation des "coins" de l'objet */
  int i_min = get_largeur(image);
  int j_min = get_hauteur(image);
  int i_max = -1;
  int j_max = -1;

  /* Récupération de la matrice binarisée */
  int** matrice_binarisee = binariser_image(image, 60);

  /* Calcul des coordonnées des "coins" de l'objet */
  for (int i=0 ; i<get_largeur(image); i++) {
    for (int j=0 ; j<get_hauteur(image) ; j++) {
      if (matrice_binarisee[i][j]) {
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