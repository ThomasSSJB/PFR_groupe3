/* FICHIER: objet.c
*  AUTEUR: GRELET Thomas
*  RÔLE: Analyse d'objet : coordonnées, forme, couleur
**/

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
  int i_min;
  int j_min;
  int i_max;
  int j_max;
};


Objet init_objet(int id) {
  Objet objet = malloc(sizeof(struct Objet));
  objet->id_objet = id;
  return objet;
}


void afficher_coordonnees_objet(Objet objet) {
  printf("Coordonnées de l'objet : de (%d, %d) à (%d, %d)\n", objet->i_min, objet->j_min, objet->i_max, objet->j_max);
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
  int x_min_boite = objet->i_min - delta;
  int y_min_boite = objet->j_min - delta;
  int x_max_boite = objet->i_max + delta;
  int y_max_boite = objet->j_max + delta;
  

  /* Application du delta en vérifiant que l'on ne sort pas du cadre */
  if (x_min_boite <= 0) x_min_boite = 0;
  if (y_min_boite <= 0) y_min_boite = 0;
  if (x_max_boite >= get_largeur(image)) x_max_boite = get_largeur(image)-1;
  if (y_max_boite >= get_hauteur(image)) y_max_boite = get_hauteur(image)-1;

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
  objet->i_min = i_min;
  objet->j_min = j_min;
  objet->i_max = i_max;
  objet->j_max = j_max;

  /* Libération mémoire */
  for (int i = 0; i < get_largeur(image); i++) {
    free(matrice_binarisee[i]);
    free(matrice_labellisee[i]);
  }
  free(matrice_binarisee);
  free(matrice_labellisee);
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
  for (int i = 0; i < get_largeur(image); i++) {
    free(matrice_binarisee[i]);
    free(matrice_labellisee[i]);
  }
  free(matrice_binarisee);
  free(matrice_labellisee);

  
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
  // afficher_pourcentages_histogramme(hist, get_largeur(sous_image)*get_hauteur(sous_image));
  

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
  /* Initialisation de la chaîne à retourner */
  char* direction = "";

  /* Coordonnée j de la vue du robot */
  float j_vue_robot = get_largeur(image) / (float) 2;

  /* Coordonnées j du centre de l'objet */
  float j_centre_objet = (objet->j_min + objet->j_max) / (float) 2;

  /* Calcul de la direction de l'objet */
  float marge = get_largeur(image) / (float) 5;   // marge arbitraire à 5
  if (j_centre_objet > (j_vue_robot + marge)) direction = "droite";
  else if (j_centre_objet < (j_vue_robot - marge)) direction = "gauche";
  else direction = "milieu";

  /* La direction est renvoyée */
  return direction;
}


int trouver_distance_objet(Objet objet, Image image) {
  /* Initialisation de la distance à renvoyer */
  float distance_sur_image;
  int distance_arrondie;

  /* Coordonnées (i, j) de la vue du robot */
  float i_vue_robot = get_hauteur(image);
  float j_vue_robot = get_largeur(image) / (float) 2;

  /* Coordonnées (i, j) du centre de l'objet */
  float i_centre_objet = (objet->i_min + objet->i_max) / (float) 2;
  float j_centre_objet = (objet->j_min + objet->j_max) / (float) 2;

  /* Calcul de la distance */
  distance_sur_image = sqrt( pow(i_vue_robot - i_centre_objet, 2) + pow(j_vue_robot - j_centre_objet, 2) );
  distance_arrondie = (int) roundf(distance_sur_image);  // arrondi à l'entier le plus proche

  /* La distance (arrondie) est renvoyée */
  return distance_arrondie;
}


int trouver_angle_objet(Objet objet, Image image) {
  /* Initialisation de l'angle à renvoyer */
  float angle;
  int angle_arrondi;

  /* Coordonnées (i, j) de la vue du robot */
  float i_vue_robot = get_hauteur(image);
  float j_vue_robot = get_largeur(image) / (float) 2;

  /* Coordonnées (i, j) du centre de l'objet */
  float i_centre_objet = (objet->i_min + objet->i_max) / (float) 2;
  float j_centre_objet = (objet->j_min + objet->j_max) / (float) 2;

  /* Calcul des côtés */
  int cote_oppose = (int) fabsf( j_vue_robot - j_centre_objet );
  int cote_adjacent = (int) fabsf( i_vue_robot - i_centre_objet );

  /* Calcul de l'angle */
  angle = atan(cote_oppose / (float) cote_adjacent);
  angle = angle * 180 / PI;  // pour passer du radian au degré
  angle_arrondi = (int) roundf(angle);  // arrondi à l'entier le plus proche

  /* L'angle (arrondi) est renvoyé */
  return angle_arrondi;
}



void commande_balle(const char* direction, int angle, int distance) {
  FILE* f = fopen("data/action.txt", "w");
  if (!f) return;

  if (strcmp(direction, "milieu") == 0) {
    int d = distance - 5;
    fprintf(f, "\"advance\" %d \"meters\"", d);
  }
  else if (strcmp(direction, "gauche") == 0) {
    fprintf(f, "turn left %d degrees advance %d meters", abs(angle), distance);
  }
  else if (strcmp(direction, "droite") == 0) {
    fprintf(f, "\"turn\" \"right\" %d \"degrees\" \"advance\" %d \"meters\"", abs(angle), distance);
  }

  fclose(f);
}


void trouver_informations_balles(Image image) {
  /* Ouverture du fichier informations_balles.txt */
  FILE* f = fopen("data/informations_balles.txt", "w");
  if (!f) return;

  int i_centre_balle;
  int j_centre_balle;

  int nb_objets = nombre_objets_image(image);

  /* Pour chaque objet de l'image */
  for (int k=1 ; k<nb_objets+1 ; k++) {
    Objet objet_courant = init_objet(k);

    /* Trouve les coordonnées de l'objet courant */
    trouver_coordonnees_objet(objet_courant, image);

    /* Trouve la nature de l'objet courant pour ensuite vérifier si c'est une balle ou non */
    trouver_nature_objet(objet_courant, image);

    if (strcmp(objet_courant->nature_forme, "Cercle") == 0) {
      /* Gère les coordonnées de la balle */
      i_centre_balle = (objet_courant->i_min + objet_courant->i_max) / 2;
      j_centre_balle = (objet_courant->j_min + objet_courant->j_max) / 2;
      fprintf(f, "%d %d ", i_centre_balle, j_centre_balle);

      /* Gère la couleur de la balle */
      trouver_couleur_objet(objet_courant, image);

      if (objet_courant->couleur == ROUGE) fprintf(f, "red\n");
      else if (objet_courant->couleur == VERT) fprintf(f, "green\n");
      else if (objet_courant->couleur == BLEU) fprintf(f, "blue\n");
      else if (objet_courant->couleur == JAUNE) fprintf(f, "yellow\n");
      else fprintf(f, "gray\n");
    }
  }

  /* Fermeture du fichier */
  fclose(f);
}


Couleur get_couleur_objet(Objet objet) {
  return objet->couleur;
}


char* get_nature_forme_objet(Objet objet) {
  return objet->nature_forme;
}


int get_i_min(Objet objet) {
  return objet->i_min;
}


int get_j_min(Objet objet) {
  return objet->j_min;
}


int get_i_max(Objet objet) {
  return objet->i_max;
}


int get_j_max(Objet objet) {
  return objet->j_max;
}