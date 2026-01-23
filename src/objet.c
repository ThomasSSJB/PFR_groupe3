#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/objet.h"
#include "../include/image.h"
#include "../include/config.h"

struct Objet {
  int id_objet;
  Couleur couleur;
  char* nature_forme;
  int x_dep;
  int y_dep;
  int x_arr;
  int y_arr;
};


Objet init_objet(int id) {
  Objet objet = malloc(sizeof(struct Objet));
  objet->id_objet = id;
  return objet;
}


void afficher_coordonnees_objet(Objet objet) {
  printf("Coordonnées de l'objet : de (%d, %d) à (%d, %d)\n", objet->x_dep, objet->y_dep, objet->x_arr, objet->y_arr);
}


void afficher_nature_objet(Objet objet) {
  if (strcmp(objet->nature_forme, "Autre") == 0) {
    printf("Forme : indéterminée\n");
  } else {
    printf("Forme : %s\n", objet->nature_forme);
  }
}


void afficher_couleur_objet(Objet objet) {
  if (objet->couleur == ROUGE) printf("Couleur : ROUGE\n");
  else if (objet->couleur == VERT) printf("Couleur : VERT\n");
  else if (objet->couleur == BLEU) printf("Couleur : BLEU\n");
  else if (objet->couleur == JAUNE) printf("Couleur : JAUNE\n");
  else if (objet->couleur == GRIS) printf("Couleur indéterminée\n");
}


Image sous_image_objet(Objet objet, Image image, int delta) {
  /* Initialisation et allocation des variables */
  int x_min_boite = objet->x_dep - delta;
  int y_min_boite = objet->y_dep - delta;
  int x_max_boite = objet->x_arr + delta;
  int y_max_boite = objet->y_arr + delta;
  

  /* Application du delta en vérifiant que l'on ne sort pas du cadre */
  if (x_min_boite < 0) x_min_boite = 0;
  else x_min_boite -= delta;

  if (y_min_boite < 0) y_min_boite = 0;
  else y_min_boite -= delta;

  if (x_max_boite > get_largeur(image)) x_max_boite = get_largeur(image)-1;
  else x_max_boite += delta;

  if (y_max_boite > get_hauteur(image)) y_max_boite = get_hauteur(image)-1;
  else y_max_boite += delta;

  /* Définitions de la largeur et hauteur de la sous-image, puis initialisation */
  int largeur_sous_image = x_max_boite - x_min_boite + 1;
  int hauteur_sous_image = y_max_boite - y_min_boite + 1;

  Image sous_image = init_image(largeur_sous_image, hauteur_sous_image);

  /* Allocation mémoire des matrices RVB de la sous-image */
  int** sous_mat_rouge = (int**) malloc(largeur_sous_image*sizeof(int*));
  int** sous_mat_vert = (int**) malloc(largeur_sous_image*sizeof(int*));
  int** sous_mat_bleu = (int**) malloc(largeur_sous_image*sizeof(int*));
  for (int i=0 ; i<largeur_sous_image ; i++) {
    sous_mat_rouge[i] = (int*) malloc(hauteur_sous_image*sizeof(int));
    sous_mat_vert[i] = (int*) malloc(hauteur_sous_image*sizeof(int));
    sous_mat_bleu[i] = (int*) malloc(hauteur_sous_image*sizeof(int));
  }

  /* Remplit les matrices RGB de la sous-image */
  for (int i=0 ; i<largeur_sous_image ; i++) {
    for (int j=0 ; j<hauteur_sous_image ; j++) {
      sous_mat_rouge[i][j] = get_mat_rouge(image)[i+x_min_boite][j+y_min_boite];
      sous_mat_vert[i][j] = get_mat_vert(image)[i+x_min_boite][j+y_min_boite];
      sous_mat_bleu[i][j] = get_mat_bleu(image)[i+x_min_boite][j+y_min_boite];
    }
  }

  /* Affecte les nouvelles matrices à la sous-image */
  set_mat_rouge(sous_image, sous_mat_rouge);
  set_mat_vert(sous_image, sous_mat_vert);
  set_mat_bleu(sous_image, sous_mat_bleu);

  /* Libération mémoire */
  for (int i=0; i<largeur_sous_image; i++) {
    free(sous_mat_rouge[i]);
    free(sous_mat_vert[i]);
    free(sous_mat_bleu[i]);
  }
  free(sous_mat_rouge);
  free(sous_mat_vert);
  free(sous_mat_bleu);

  /* Retourne la sous-image */
  return sous_image;
}


void trouver_coordonnees_objet(Objet objet, Image image) {
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
      if (matrice_labellisee[i][j] == objet->id_objet) {
        if (i < i_min) i_min = i;
        if (j < j_min) j_min = j;
        if (i > i_max) i_max = i;
        if (j > j_max) j_max = j;
      }
    }
  }

  /* Affectation des valeurs à l'objet */
  objet->x_dep = i_min;
  objet->y_dep = j_min;
  objet->x_arr = i_max;
  objet->y_arr = j_max;
}


void trouver_nature_objet(Objet objet, Image image) {
  /* Charge la valeur du seuil pour la forme (cercle) depuis la configuration */
  float seuil_forme = lire_valeur_json("seuil_forme", config_json) / (float) 100;
  // printf("[TRACE] seuil_forme = %.2f\n", seuil_forme);

  /* Initialisation des variables */
  char* nature_forme = "Autre";
  int** matrice_binarisee = binariser_image(image);
  int** matrice_labellisee = labelliser_image_binaire(matrice_binarisee, get_largeur(image), get_hauteur(image));
  
  int aire = 0;
  int perimetre = 0;
  float circularite;

  for (int i=0 ; i<get_largeur(image) ; i++) {
    for (int j=0 ; j<get_hauteur(image) ; j++) {
      if (matrice_labellisee[i][j] == objet->id_objet) {
        aire++;
        
        // Vérifier si c'est un pixel de contour
        if (i == 0 || j == 0 || i == get_largeur(image)-1 || j == get_hauteur(image)-1) {
          perimetre++;
        }
        else if (matrice_labellisee[i-1][j] != objet->id_objet || 
                 matrice_labellisee[i+1][j] != objet->id_objet || 
                 matrice_labellisee[i][j-1] != objet->id_objet || 
                 matrice_labellisee[i][j+1] != objet->id_objet) {
          perimetre++;
        }
      }
    }
  }

  /* Calcul de la circularité de l'forme */
  if (perimetre > 0) circularite = 4*PI*aire / (float) (perimetre*perimetre);
  else circularite = 0;

  // printf("[TRACE] perimetre = %d\n", perimetre);
  // printf("[TRACE] aire = %d\n", aire);
  // printf("[TRACE] circularite = %.2f\n", circularite);

  /* Détermination de la nature de la forme (cercle ou pas) */
  if (circularite > seuil_forme && circularite < (1+seuil_forme)) {
    nature_forme = "Cercle";
  }

  /* Libération mémoire */
  for (int i = 0 ; i < get_largeur(image) ; i++)
    free(matrice_binarisee[i]);
  free(matrice_binarisee);

  
  /* Affecte la nature de la forme à l'objet */
  objet->nature_forme = nature_forme;
}


void trouver_couleur_objet(Objet objet, Image image) {
  /* Chargement du nombre de bits de quantification */
  int seuil_quantif = lire_valeur_json("quantification_bits", config_json);
  int taille_hist = (int) pow(2, 3 * seuil_quantif);
  
  /* Calcul de l'histogramme */
  Image sous_image = sous_image_objet(objet, image, 1);
  Histogramme hist = histogramme_image(sous_image);
  

  /* ===== ETAPE 1 : Trouver la valeur la plus fréquente ===== */
  int indice_max = 0;
  int freq_max = hist[0];
  
  for (int i = 1 ; i<taille_hist ; i++) {
    if (hist[i] > freq_max) {
      freq_max = hist[i];
      indice_max = i;
    }
  }

  
  /* ===== ETAPE 2 : Décoder en R, V, B ===== */
  int masque = (1 << seuil_quantif) - 1;
    
  int R = (indice_max >> (2 * seuil_quantif)) & masque;
  int V = (indice_max >> seuil_quantif) & masque;
  int B = indice_max & masque;

  // printf("[TRACE] indice_max = %d\n", indice_max);
  // printf("[TRACE] masque = %d\n", masque);
  // printf("[TRACE] R = %d, V = %d, B = %d\n", R, V, B);

  
  /* ===== ETAPE 3 : Classifier la couleur ===== */
  int valeur_max = (int) pow(2, seuil_quantif) - 1;
  int seuil_fort = valeur_max * 2 / 3;  /* Environ 2/3 du max */

  // printf("[TRACE] valeur_max = %d\n", valeur_max);
  // printf("[TRACE] seuil_fort = %d\n", seuil_fort);
  
  Couleur couleur_objet;
  int tolerance_jaune = valeur_max / 6;
  
  /* ROUGE : R fort, V et B faibles */
  if (R >= seuil_fort && V < seuil_fort && B < seuil_fort) {
    couleur_objet = ROUGE;
  }
  /* VERT : V fort, R et B faibles */
  else if (V >= seuil_fort && R < seuil_fort && B < seuil_fort) {
    couleur_objet = VERT;
  }
  /* BLEU : B fort, R et V faibles */
  else if (B >= seuil_fort && R < seuil_fort && V < seuil_fort) {
    couleur_objet = BLEU;
  }
  /* JAUNE : R et V forts, B faible */
  else if (R >= seuil_fort && V >= seuil_fort && B < seuil_fort) {
    couleur_objet = JAUNE;
  }
  /* Cas indéterminé */
  else {
    /* Par défaut, vérifier le jaune sinon prendre la composante la plus forte */
    if (R == V && abs(R - V) < tolerance_jaune && (B < (R+V)/4 || B < seuil_fort)) {
      couleur_objet = JAUNE;
    } else if (R >= V && R >= B) {
      couleur_objet = ROUGE;
    } else if (V >= R && V >= B) {
      couleur_objet = VERT;
    } else {
      couleur_objet = BLEU;
    }
  }
  
  /* Libération */
  free(hist);
  
  /* Affecte la couleur à l'objet */
  objet->couleur = couleur_objet;
}


char* trouver_direction_objet(Objet objet, Image image) {
  char* direction = "";

  float x_centre_image = get_largeur(image) / (float) 2;
  float x_centre_objet = ( objet->x_dep + objet->x_arr ) / (float) 2;
  
  float marge = x_centre_image / (float) 5;  // marge arbitraire

  // printf("[TRACE] x_centre_image = %.2f\n", x_centre_image);
  // printf("[TRACE] x_centre_objet = %.2f\n", x_centre_objet);
  // printf("[TRACE] marge = %.2f\n", marge);

  if (x_centre_objet < (x_centre_image - marge)) direction = "gauche";
  else if (x_centre_objet > (x_centre_image + marge)) direction = "droite";
  else direction = "milieu";

  return direction;
}


int trouver_distance_objet(Objet objet, Image image) {
  float distance_sur_image;
  int distance_reelle_arrondi;

  float x_vue_robot = get_largeur(image) / (float) 2;
  float y_vue_robot = (float) get_hauteur(image);

  float x_centre_objet = ( objet->x_dep + objet->x_arr ) / (float) 2;
  float y_centre_objet = ( objet->y_dep + objet->y_arr ) / (float) 2;

  distance_sur_image = sqrt( pow((x_vue_robot-x_centre_objet), 2) + pow((y_vue_robot-y_centre_objet), 2) );
  distance_reelle_arrondi = (int) roundf(distance_sur_image / 120); // arrondit à l'entier le plus proche

  return distance_reelle_arrondi;
}


int trouver_angle_objet(Objet objet, Image image) {
  float angle;
  int angle_arrondi;

  float x_vue_robot = get_largeur(image) / (float) 2;
  float x_centre_objet = ( objet->x_dep + objet->x_arr ) / (float) 2;
  float valeur = (get_hauteur(image) - objet->y_dep) / ( abs(x_centre_objet-x_vue_robot) );

  angle = atan(valeur);
  angle = angle * 180 / PI;  // pour passer du radian au degré
  angle = 90 - angle;
  
  angle_arrondi = (int) roundf(angle); // arrondit à l'entier le plus proche

  return angle_arrondi;
}


Couleur get_couleur_objet(Objet objet) {
  return objet->couleur;
}


char* get_nature_forme_objet(Objet objet) {
  return objet->nature_forme;
}


int get_x_dep(Objet objet) {
  return objet->x_dep;
}


int get_y_dep(Objet objet) {
  return objet->y_dep;
}


int get_x_arr(Objet objet) {
  return objet->x_arr;
}


int get_y_arr(Objet objet) {
  return objet->y_arr;
}


void commande_balle(const char* direction, int angle, int distance)
{
  FILE* f = fopen("action.txt", "w");
  if (!f) return;

  if (strcmp(direction, "milieu") == 0)
  {
    int d = distance - 5;
    fprintf(f, "\"advance\" %d \"meters\"", d);
  }
  else if (strcmp(direction, "gauche") == 0)
  {
    fprintf(f, "turn left %d degrees advance %d meters", abs(angle), distance);
  }
  else if (strcmp(direction, "droite") == 0)
  {
    fprintf(f, "\"turn\" \"right\" %d \"degrees\" \"advance\" %d \"meters\"", abs(angle), distance);
  }

  fclose(f);
}