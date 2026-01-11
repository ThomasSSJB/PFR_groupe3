#ifndef IMAGE_H
#define IMAGE_H

#define LARGEUR_MAX 300
#define HAUTEUR_MAX 300

typedef struct Image * Image;
typedef int* Histogramme;
typedef enum Couleur * Couleur;

/* Initialise une image (allocation mémoire) */
Image init_image(void);

/* Lit et remplit une variable de type Image */
void lire_image(Image *ptr_image);

/* Affiche une image par pixels (R,V,B) */
void afficher_image_pixels(Image image);

/* Affiche successivement les 3 matrices d'une image */
void afficher_image_matrices(Image image);

/* Affiche une image binaire (0 pour le fond, 1 pour l'objet) */
void afficher_image_binaire(int** img_bin, int largeur, int hauteur);

/* Affiche la boîte englobante d'un objet sur une image binarisée */
void afficher_image_boite_englobante(Image image, int delta);

/* Transforme les 3 matrices d'une image pour les passer à un certain niveau de gris (passé en paramètre) */
Image niveau_gris_image(Image image, int niveau_gris);

/* Binarise une image selon la saturation des couleurs et renvoie la matrice binarisée */
int** binariser_image(Image image, int seuil);

/* Retourne la valeur décimal d'un pixel quantifié (moyenne: un pixel = (R+G+B)/3)*/
int quantifier_pixel(int pixel[3], int niveau_quantification);

/* Retourne la matrice des pixels quantifiés */
int** quantifier_image(Image image);

/* Retourne un tableau contenant l'histogramme d'une image passée en paramètre */
Histogramme histogramme_image(Image image);

/* à coder */
Couleur trouver_couleur_objet(Histogramme hist);

/* à coder */
Couleur pixel_vers_couleur(int pixel[3], int seuil);

/* getteurs */
int get_largeur(Image image);
int get_hauteur(Image image);

/* Convertit un nombre décimal en binaire (sur 8 bits) */
char* decimal_en_binaire(int decimal);

/* Convertit un nombre binaire (sur 8 bits) en décimal  */
int binaire_en_decimal(char* binaire);

#endif