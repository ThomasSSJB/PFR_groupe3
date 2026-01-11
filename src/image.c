#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/image.h"
#include "../include/forme.h"

struct Image {
    int largeur;
    int hauteur;
    int mat_rouge[LARGEUR_MAX][HAUTEUR_MAX];
    int mat_vert[LARGEUR_MAX][HAUTEUR_MAX];
    int mat_bleu[LARGEUR_MAX][HAUTEUR_MAX];
};

enum Couleur {ROUGE, VERT, BLEU, JAUNE};


Image init_image(void) {
    Image image = malloc(sizeof(struct Image));
    image->largeur = 0;
    image->hauteur = 0;
    return image;
}


void lire_image(Image *ptr_image) {
    int largeur, hauteur, tmp, valeur_pixel;

    /* Récupération de la hauteur et de la largeur */
    scanf("%d%d%d", &largeur, &hauteur, &tmp);
    (*ptr_image)->largeur = largeur;
    (*ptr_image)->hauteur = hauteur;

    for (int k=0 ; k<3 ; k++) {
        for (int i=0 ; i<largeur ; i++) {
            for (int j=0 ; j<hauteur ; j++) {
                /* Récupération de la valeur du pixel */
                scanf("%d", &valeur_pixel);

                /* Remplit la matrice rouge */
                if (k == 0) (*ptr_image)->mat_rouge[i][j] = valeur_pixel;

                /* Remplit la matrice verte */
                if (k == 1) (*ptr_image)->mat_vert[i][j] = valeur_pixel;

                /* Remplit la matrice bleue */
                if (k == 2) (*ptr_image)->mat_bleu[i][j] = valeur_pixel;
            }
        }
    }
}


void afficher_image_pixels(Image image) {
    for (int i=0 ; i<image->largeur ; i++) {
        for (int j=0 ; j<image->hauteur ; j++) {
            printf("(%d, %d, %d) ", image->mat_rouge[i][j], image->mat_vert[i][j], image->mat_bleu[i][j]);
        }
        printf("\n");
    }
}


void afficher_image_matrices(Image image) {
    for (int k=0 ; k<3 ; k++) {

        if (k == 0) printf("\n--- Matrice Rouge ---\n");
        if (k == 1) printf("\n--- Matrice Bleue ---\n");
        if (k == 2) printf("\n--- Matrice Verte ---\n");

        for (int i=0 ; i<image->largeur ; i++) {
            for (int j=0 ; j<image->hauteur ; j++) {
                if (k == 0) printf("%d ", image->mat_rouge[i][j]);
                if (k == 1) printf("%d ", image->mat_bleu[i][j]);
                if (k == 2) printf("%d ", image->mat_vert[i][j]);
            }
            printf("\n");
        }
    }
}


void afficher_image_binaire(int** img_bin, int largeur, int hauteur) {
    printf("%d %d\n", largeur, hauteur);
    for (int i = 0; i < largeur; i++) {
        for (int j = 0; j < hauteur; j++) {
            printf("%d ", img_bin[i][j]);
        }
        printf("\n");
    }
}


void afficher_image_boite_englobante(Image image, int delta) {
    /* Binarisation de l'image */
    int** img_bin = binariser_image(image, 60);

    /* Récupération des coordonnées des 'coins' de l'objet */
    int* tab = trouver_coordonnees_forme(image);
    
    /* Initialisation des variables */
    int i_min = tab[0] - delta;
    int j_min = tab[1] - delta;
    int i_max = tab[2] + delta;
    int j_max = tab[3] + delta;

    /* Test si le cadre n'est pas trop grand */
    if ((i_min <= 0 || j_min <= 0 || i_max >= image->largeur || j_max >= image->hauteur)) fprintf(stderr, "Erreur: cadre trop grand.\n");

    /* Délimite le cadre dans la matrice binaire */
    for (int i=0 ; i<image->largeur ; i++) {
        for (int j=0 ; j<image->hauteur ; j++) {
            if ((i == i_min) && (j >= j_min) && (j <= j_max)) img_bin[i][j] = 7;    // côté supérieur
            else if ((i == i_max) && (j >= j_min) && (j <= j_max)) img_bin[i][j] = 7;    // côté inférieur
            else if ((j == j_min) && (i >= i_min) && (i <= i_max)) img_bin[i][j] = 7;    // côté gauche
            else if ((j == j_max) && (i >= i_min) && (i <= i_max)) img_bin[i][j] = 7;    // côté gauche
        }
    }

    /* Affichage de l'image binarisée avec la boîte englobante  */
    afficher_image_binaire(img_bin, image->largeur, image->hauteur);
}


Image niveau_gris_image(Image image, int niveau_gris) {
    /* Vérification du niveau de gris */
    if ((niveau_gris < 1) || (niveau_gris > 255)) {
        fprintf(stderr, "Erreur : le niveau de gris n’est pas valide\n");
    } else if (log2(niveau_gris) != (int)log2(niveau_gris)) {
        fprintf(stderr, "Erreur : le niveau de gris n’est pas une puissance de 2\n");
    } else {
        /* Transformation des 3 matrices de l'image en niveau de gris */
        for (int k=0 ; k<3 ; k++) {
            for (int i=0 ; i<image->largeur ; i++) {
                for (int j=0 ; j<image->hauteur ; j++) {
                    if (k==0) image->mat_rouge[i][j] = (int) (image->mat_rouge[i][j] / (256 / (float) niveau_gris));
                    if (k==1) image->mat_bleu[i][j] = (int) (image->mat_bleu[i][j] / (256 / (float) niveau_gris));
                    if (k==2) image->mat_vert[i][j] = (int) (image->mat_vert[i][j] / (256 / (float) niveau_gris));
                }
            }
        }
    }
    return image;
}


int** binariser_image(Image image, int seuil_saturation) {
    /* Allocation de la matrice binaire */
    int** img_bin = (int**) malloc(image->largeur * sizeof(int*));
    for (int i = 0; i < image->largeur; i++) {
        img_bin[i] = (int*) calloc(image->hauteur, sizeof(int));
    }
    
    /* Parcours de tous les pixels de l'image */
    for (int i = 0; i < image->largeur; i++) {
        for (int j = 0; j < image->hauteur; j++) {
            /* Récupération des composantes RGB du pixel */
            int R = image->mat_rouge[i][j];
            int G = image->mat_vert[i][j];
            int B = image->mat_bleu[i][j];
            
            /* Calcul du maximum des 3 composantes */
            int max = R;
            if (G > max) { max = G; }
            if (B > max) { max = B; }
            
            /* Calcul du minimum des 3 composantes */
            int min = R;
            if (G < min) { min = G; }
            if (B < min) { min = B; }
            
            /* Calcul de la saturation */
            int saturation = max - min;
            
            /* Binarisation selon le seuil */
            if (saturation > seuil_saturation) {
                img_bin[i][j] = 1;  /* Pixel coloré = objet */
            } else {
                img_bin[i][j] = 0;  /* Pixel neutre = fond */
            }
        }
    }
    
    return img_bin;
}


int quantifier_pixel(int pixel[3], int seuil_quantif) {
    /* Initialisation de la chaîne quantifiée et autres variables */
    char bin_quantif[3*seuil_quantif+1];
    int dec_quantif, index = 0;

    /* Conversion des valeurs des pixels en binaire */
    char* bin_rouge = decimal_en_binaire(pixel[0]);
    char* bin_vert = decimal_en_binaire(pixel[1]);
    char* bin_bleu = decimal_en_binaire(pixel[2]);

    /* Remplit la chaîne quantifiée */
    for (int k=0 ; k<3 ; k++) {
        for (int i=0 ; i<seuil_quantif ; i++) {
            if (k==0) bin_quantif[index] = bin_rouge[i];
            if (k==1) bin_quantif[index] = bin_vert[i];
            if (k==2) bin_quantif[index] = bin_bleu[i];
            index++;
        }
    }
    bin_quantif[index] = '\0';

    /* Conversion de la valeur du pixel quantifié en décimal */
    dec_quantif = binaire_en_decimal(bin_quantif);

    return dec_quantif;
}


int** quantifier_image(Image image) {
    /* Initialisation de la matrice des pixels quantifiés */
    int** mat_quantif = (int**) malloc((image->largeur) * sizeof(int*));
    for (int i=0 ; i<image->largeur ; i++) {
        mat_quantif[i] = (int*) malloc((image->hauteur) * sizeof(int));
    }

    /* Initialisation du pixel courant */
    int pixel[3];

    /* Remplissage de la matrice */
    for (int i=0 ; i<image->largeur ; i++) {
        for (int j=0 ; j<image->hauteur ; j++) {

            /* Remplissage du pixel courant */
            pixel[0] = image->mat_rouge[i][j];
            pixel[1] = image->mat_vert[i][j];
            pixel[2] = image->mat_bleu[i][j];

            /* Ajout du pixel quantifié à la matrice */
            mat_quantif[i][j] = quantifier_pixel(pixel, 2);
        } 
    }

    return mat_quantif;
}


int get_largeur(Image image) {
    return image->largeur;
}


int get_hauteur(Image image) {
    return image->hauteur;
}


Histogramme histogramme_image(Image image) {
    int** matrice = quantifier_image(image);   /* récupère la matrice quantifiée */

    int indice;
    int seuil = 2;   /* à remplacer avec la lecture du vrai seuil */
    int taille = (int) pow(2, 3*seuil);
    /* tableau avec une taille de 2^(3*seuil) : 64, 512, 4096... */
    int* tab = (int*) malloc(taille*sizeof(int));

    /* remplit le tableau avec des 0 (initialisation) */
    for (int i=0; i<taille ; i++) {
        tab[i] = 0;
    }

    /* calcul de l'histogramme */
    for (int i=0 ; i<image->largeur ; i++) {
        for (int j=0 ; j<image->hauteur; j++) {
            indice = matrice[i][j];
            tab[indice]++;
        }
    }

    /* libération de la mémoire*/
    for (int i=0 ; i<taille ; i++) {
        free(matrice[i]);
    }
    free(matrice);

    return tab;
}


/* =========== à déplacer =========== */

/* Convertir un nombre décimal en binaire */
char* decimal_en_binaire(int decimal) {
    char* binaire = (char*)malloc(9 * sizeof(char));
    int bit;
    
    for (int i=7 ; i>=0 ; i--) {
        bit = decimal % 2;
        binaire[i] = bit + '0';
        decimal = decimal / 2;
    }
    
    binaire[8] = '\0';
    return binaire;
}

/* Convertir un nombre binaire en décimal */
int binaire_en_decimal(char* binaire) {
    int decimal = 0;
    int puissance = 0;
    int longueur = strlen(binaire);
    
    /* Lire de droite à gauche */
    for (int i = longueur-1; i >= 0; i--) {
        if (binaire[i] == '1') {
            decimal = decimal + (int)pow(2, puissance);
        }
        puissance++;
    }
    
    return decimal;
}
