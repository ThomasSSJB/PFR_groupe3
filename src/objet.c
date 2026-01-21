#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/objet.h"
#include "../include/forme.h"
#include "../include/image.h"
#include "../include/config.h"

struct Objet {
  Couleur couleur;
  Forme forme;
};


Objet init_objet(Couleur couleur, Forme forme) {
  Objet objet = malloc(sizeof(struct Objet));
  objet->couleur = couleur;
  objet->forme = forme;
  return objet;
}


Image sous_image_objet(Image image, int valeur_objet, int delta) {
  /* Initialisation et allocation des variables */
  int* tab_coord = trouver_coordonnees_forme(image, valeur_objet);

  /* Application du delta en vérifiant que l'on ne sort pas du cadre */
  if ((tab_coord[0] - delta) < 0) tab_coord[0] = 0;
  else tab_coord[0] -= delta;

  if ((tab_coord[1] - delta) < 0) tab_coord[1] = 0;
  else tab_coord[1] -= delta;

  if ((tab_coord[2] + delta) > get_largeur(image)) tab_coord[2] = get_largeur(image)-1;
  else tab_coord[2] += delta;

  if ((tab_coord[3] + delta) > get_hauteur(image)) tab_coord[3] = get_hauteur(image)-1;
  else tab_coord[3] += delta;

  /* Définitions de la largeur et hauteur de la sous-image, puis initialisation */
  int largeur_sous_image = tab_coord[2] - tab_coord[0] + 1;
  int hauteur_sous_image = tab_coord[3] - tab_coord[1] + 1;

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
      sous_mat_rouge[i][j] = get_mat_rouge(image)[i+tab_coord[0]][j+tab_coord[1]];
      sous_mat_vert[i][j] = get_mat_vert(image)[i+tab_coord[0]][j+tab_coord[1]];
      sous_mat_bleu[i][j] = get_mat_bleu(image)[i+tab_coord[0]][j+tab_coord[1]];
    }
  }

  /* Affecte les nouvelles matrices à la sous-image */
  set_mat_rouge(sous_image, sous_mat_rouge);
  set_mat_vert(sous_image, sous_mat_vert);
  set_mat_bleu(sous_image, sous_mat_bleu);

  /* Retourne la sous-image */
  return sous_image;
}


Couleur trouver_couleur_objet(Image sous_image) {
  /* Chargement du nombre de bits de quantification */
  int seuil_quantif = lire_valeur_json("quantification_bits", config_json);
  int taille_hist = (int) pow(2, 3 * seuil_quantif);
  
  /* Calcul de l'histogramme */
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
  
  /* Retourne la couleur trouvée */
  return couleur_objet;
}