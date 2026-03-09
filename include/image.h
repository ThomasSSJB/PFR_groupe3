/* FICHIER: image.h
*  AUTEUR: GRELET Thomas, DEVAUD Antoine
*  RÔLE: Déclaration des fonctions d'analyse et de transformation d'image
**/

#ifndef IMAGE_H
#define IMAGE_H

#define LARGEUR_MAX 300
#define HAUTEUR_MAX 300

typedef struct Image * Image;
typedef int* Histogramme;
typedef enum Couleur Couleur;

enum Couleur {
  ROUGE,
  VERT,
  BLEU,
  JAUNE,
  GRIS
};

/* Initialise une image (allocation mémoire) */
Image init_image(int largeur_image, int hauteur);

/* Lit et remplit une variable de type Image */
Image lire_image(void);

/* Affiche une image par pixels (R,V,B) */
void afficher_image_pixels(Image image);

/* Affiche successivement les 3 matrices d'une image */
void afficher_image_matrices(Image image);

/* Affiche une image binaire (0 pour le fond, 1 pour l'objet) */
void afficher_image_binaire(int** img_bin, int largeur, int hauteur);

/* Affiche la boîte englobante d'un objet sur une image binarisée */
void afficher_image_boites_englobantes(Image image, int delta);

/* Affiche l'histogramme d'une image en pourcentages */
void afficher_pourcentages_histogramme(Histogramme hist, int taille_image);

/* Transforme les 3 matrices d'une image pour les passer à un certain niveau de gris (passé en paramètre) */
Image niveau_gris_image(Image image, int niveau_gris);

/* Binarise une image selon la saturation des couleurs et renvoie la matrice binarisée */
int** binariser_image(Image image);

/* Labellise une image binaire -> Sépare chaque objet détecté  en utilisant un algorithme "Two Pass"*/
int** labelliser_image_binaire(int** img_bin, int largeur, int hauteur);

/* Retourne la valeur décimal d'un pixel quantifié (moyenne: un pixel = (R+G+B)/3)*/
int quantifier_pixel(int pixel[3]);

/* Retourne la matrice des pixels quantifiés */
int** quantifier_image(Image image);

/* Retourne un tableau contenant l'histogramme d'une image passée en paramètre */
Histogramme histogramme_image(Image image);

/* Retourne le nombre d'objets présents sur une image */
int nombre_objets_image(Image image);

/* getteurs */
int get_largeur(Image image);
int get_hauteur(Image image);
int** get_mat_rouge(Image image);
int** get_mat_vert(Image image);
int** get_mat_bleu(Image image);

/* setteurs */
void set_largeur(Image image, int new_largeur);
void set_hauteur(Image image, int new_hauteur);
void set_mat_rouge(Image image, int **new_mat_rouge);
void set_mat_vert(Image image, int **new_mat_vert);
void set_mat_bleu(Image image, int **new_mat_bleu);

#endif