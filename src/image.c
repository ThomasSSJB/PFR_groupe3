/* FICHIER: image.c
*  AUTEUR: GRELET Thomas
*  RÔLE: Analyse et transformation d'image : binarisation, labellisation, histogramme, quantification
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/image.h"
#include "../include/objet.h"
#include "../include/config.h"
#include "../include/utils.h"

struct Image {
    int largeur;
    int hauteur;
    int** mat_rouge;
    int** mat_vert;
    int** mat_bleu;
};


Image init_image(int largeur_image, int hauteur_image) {
    Image image = malloc(sizeof(struct Image));
    image->largeur = largeur_image;
    image->hauteur = hauteur_image;

    image->mat_rouge = (int**) malloc(largeur_image*sizeof(int*));
    image->mat_vert = (int**) malloc(largeur_image*sizeof(int*));
    image->mat_bleu = (int**) malloc(largeur_image*sizeof(int*));

    for (int i=0 ; i<largeur_image ; i++) {
        image->mat_rouge[i] = (int*) malloc(hauteur_image*sizeof(int));
        image->mat_vert[i] = (int*) malloc(hauteur_image*sizeof(int));
        image->mat_bleu[i] = (int*) malloc(hauteur_image*sizeof(int));
    }
    return image;
}


Image lire_image(void) {
    Image image;
    int largeur, hauteur, tmp, valeur_pixel;

    /* Choix de l'image */
    int numero_image;
    printf("\nEntrer le numéro de l'image à charger (image recommandée : 5402)\n");
    printf("> ");
    scanf("%d", &numero_image);

    FILE* fichier = NULL;
    char chemin[50];
    sprintf(chemin, "image/IMG_%d.txt", numero_image);

    // On ouvre le fichier en mode lecture ("r")
    fichier = fopen(chemin, "r");

    if (fichier == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier %s\n", chemin);
        exit(0);
    }
    printf("L'image IMG_%d.txt a été chargée.\n", numero_image);
    
    /* Récupération de la hauteur et de la largeur avec fscanf */
    fscanf(fichier, "%d %d %d", &largeur, &hauteur, &tmp);
    
    /* Initialise l'image */
    image = init_image(largeur, hauteur);

    for (int k=0 ; k<3 ; k++) {
        for (int i=0 ; i<largeur ; i++) {
            for (int j=0 ; j<hauteur ; j++) {
                /* Récupération de la valeur du pixel dans le fichier */
                if (fscanf(fichier, "%d", &valeur_pixel) != EOF) {
                    /* Remplit les matrices */
                    if (k == 0) image->mat_rouge[i][j] = valeur_pixel;
                    if (k == 1) image->mat_vert[i][j] = valeur_pixel;
                    if (k == 2) image->mat_bleu[i][j] = valeur_pixel;
                }
            }
        }
    }

    fclose(fichier);

    return image;
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


void afficher_image_boites_englobantes(Image image, int delta) {
    /* Binarisation et Labellisation de l'image */
    int** img_bin = binariser_image(image);
    int** img_labelisee = labelliser_image_binaire(img_bin, image->largeur, image->hauteur);

    /* Trouve le nombre d'objets sur l'image */
    int nb_objets = max_matrice(img_labelisee, image->largeur, image->hauteur);

    /* Pour chaque objet */
    for (int k=1 ; k<nb_objets+1 ; k++) {
        /* Défini l'objet courant */
        Objet objet_courant = init_objet(k);

        /* Récupération des coordonnées des 'coins' de l'objet */
        trouver_coordonnees_objet(objet_courant, image);
        
        /* Initialisation des variables */
        int i_min = get_i_min(objet_courant) - delta;
        int j_min = get_j_min(objet_courant) - delta;
        int i_max = get_i_max(objet_courant) + delta;
        int j_max = get_j_max(objet_courant) + delta;

        /* Test si le cadre n'est pas trop grand */
        if (i_min < 0) i_min = 0;
        if (j_min < 0) j_min = 0;
        if (i_max > image->largeur) i_max = image->largeur;
        if (j_max > image->hauteur) j_max = image->hauteur;
        
        /* Délimite le cadre dans la matrice binaire */
        for (int i=0 ; i<image->largeur ; i++) {
            for (int j=0 ; j<image->hauteur ; j++) {
                if ((i == i_min) && (j >= j_min) && (j <= j_max)) img_labelisee[i][j] = 7;    // côté supérieur
                else if ((i == i_max) && (j >= j_min) && (j <= j_max)) img_labelisee[i][j] = 7;    // côté inférieur
                else if ((j == j_min) && (i >= i_min) && (i <= i_max)) img_labelisee[i][j] = 7;    // côté gauche
                else if ((j == j_max) && (i >= i_min) && (i <= i_max)) img_labelisee[i][j] = 7;    // côté gauche
            }
        }
    }

    /* Affichage de l'image binarisée avec la (ou les) boîte(s) englobante(s)  */
    afficher_image_binaire(img_labelisee, image->largeur, image->hauteur);
}


void afficher_pourcentages_histogramme(Histogramme hist, int taille_image) {
    int seuil_quantif = lire_valeur_json("quantification_bits", config_json);
    int taille_hist = (int) pow(2, 3*seuil_quantif);
    float valeur;

    for (int i=0 ; i<taille_hist ; i++) {
        if (hist[i] != 0) {
            valeur = (hist[i]/ (float) taille_image) * 100;
            printf("* %d -> %.2f%%\n", i, valeur);
        }
    }
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


int** binariser_image(Image image) {
    /* Charge le seuil de couleur (appellé dans ce contexte "seuil de saturation") depuis la config */
    int seuil_saturation = lire_valeur_json("seuil_binarisation", config_json);
    int intensite_min_rouge = lire_valeur_json("intensite_min_rouge", config_json);
    int intensite_min_vert = lire_valeur_json("intensite_min_vert", config_json);
    int intensite_min_bleu = lire_valeur_json("intensite_min_bleu", config_json);

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

            if (R < intensite_min_rouge && G < intensite_min_vert && B < intensite_min_bleu) {
                img_bin[i][j] = 0;
            } else {
                /* Calcul du maximum des 3 composantes */
                int max = R;
                if (G > max) max = G;
                if (B > max) max = B;
                
                /* Calcul du minimum des 3 composantes */
                int min = R;
                if (G < min) min = G;
                if (B < min) min = B;
                
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
    }
    
    return img_bin;
}

int** labelliser_image_binaire(int** img_bin, int largeur, int hauteur) {
    /* Allocation et initialisation */
    int** img_labelisee = (int**) malloc(largeur * sizeof(int*));
    for (int i = 0; i < largeur; i++) {
        img_labelisee[i] = (int*) calloc(hauteur, sizeof(int));
    }
    
    int label_actuel = 1;
    
    /* PASS 1 : Labellisation provisoire*/
    for (int i = 0; i < largeur; i++) {
        for (int j = 0; j < hauteur; j++) {
            /* Ignorer le fond */
            if (img_bin[i][j] == 0) {
                continue;
            }
            
            /* Récupérer les labels des voisins déjà traités */
            int label_haut = 0;
            int label_gauche = 0;
            
            if (i > 0) {
                label_haut = img_labelisee[i-1][j];
            }
            
            if (j > 0) {
                label_gauche = img_labelisee[i][j-1];
            }
            
            /* CAS 1 : Aucun voisin → Nouveau label*/
            if (label_haut == 0 && label_gauche == 0) {
                label_actuel++;
                img_labelisee[i][j] = label_actuel;
            }
            /* CAS 2 : Voisin haut seulement */
            else if (label_haut != 0 && label_gauche == 0) {
                img_labelisee[i][j] = label_haut;
            }
            /* CAS 3 : Voisin gauche seulement*/
            else if (label_haut == 0 && label_gauche != 0) {
                img_labelisee[i][j] = label_gauche;
            }
            /* CAS 4 : Les deux voisins*/
            else {
                /* Prendre le minimum des deux */
                int min_label;
                int max_label;
                
                if (label_haut < label_gauche) {
                    min_label = label_haut;
                    max_label = label_gauche;
                } else {
                    min_label = label_gauche;
                    max_label = label_haut;
                }
                
                img_labelisee[i][j] = min_label;
                
                /* Si différents, propager le minimum */
                if (label_haut != label_gauche) {
                    /* Remplacer tous les max_label par min_label */
                    for (int ii = 0; ii <= i; ii++) {
                        for (int jj = 0; jj < hauteur; jj++) {
                            if (img_labelisee[ii][jj] == max_label) {
                                img_labelisee[ii][jj] = min_label;
                            }
                        }
                    }
                }
            }
        }
    }
    
    /* PASS 2 : Renumérote pour avoir 1, 2, 3...*/
    int correspondance[256] = {0};
    int nouveau_label = 0;
    
    for (int i = 0; i < largeur; i++) {
        for (int j = 0; j < hauteur; j++) {
            int ancien = img_labelisee[i][j];
            if (ancien > 0) {
                if (correspondance[ancien] == 0) {
                    nouveau_label++;
                    correspondance[ancien] = nouveau_label;
                }
                img_labelisee[i][j] = correspondance[ancien];
            }
        }
    }
    
    return img_labelisee;
}



int quantifier_pixel(int pixel[3]) {
    /* Initialisation de la chaîne quantifiée et autres variables */
    int seuil_quantif = lire_valeur_json("quantification_bits", config_json);
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
            pixel[0] = image->mat_rouge[i][j];  // R
            pixel[1] = image->mat_vert[i][j];   // V
            pixel[2] = image->mat_bleu[i][j];   // B

            /* Ajout du pixel quantifié à la matrice */
            mat_quantif[i][j] = quantifier_pixel(pixel);
        } 
    }

    return mat_quantif;
}


Histogramme histogramme_image(Image image) {
    int** matrice = quantifier_image(image);   /* récupère la matrice quantifiée */
    int seuil_quantif = lire_valeur_json("quantification_bits", config_json);   /* charge le nombre de bits pour la quantification */

    int indice;
    int taille = (int) pow(2, 3*seuil_quantif);

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
    for (int i=0 ; i<image->largeur ; i++) {
        free(matrice[i]);
    }
    free(matrice);

    return tab;
}


int nombre_objets_image(Image image) {
    int** matrice_binarisee = binariser_image(image);
    int** matrice_labellisee = labelliser_image_binaire(matrice_binarisee, image->largeur, image->hauteur);
    return max_matrice(matrice_labellisee, image->largeur, image->hauteur);
}


int get_largeur(Image image) {
    return image->largeur;
}


int get_hauteur(Image image) {
    return image->hauteur;
}


int** get_mat_rouge(Image image) {
    return image->mat_rouge;
}


int** get_mat_vert(Image image) {
    return image->mat_vert;
}


int** get_mat_bleu(Image image) {
    return image->mat_bleu;
}


void set_largeur(Image image, int new_largeur) {
    image->largeur = new_largeur;
}

void set_hauteur(Image image, int new_hauteur) {
    image->hauteur = new_hauteur;
}


void set_mat_rouge(Image image, int** new_mat_rouge) {
    for (int i=0 ; i<image->largeur ; i++) {
        for (int j=0 ; j<image->hauteur ; j++) {
            image->mat_rouge[i][j] = new_mat_rouge[i][j];
        }
    }
}


void set_mat_vert(Image image, int** new_mat_vert) {
    for (int i=0 ; i<image->largeur ; i++) {
        for (int j=0 ; j<image->hauteur ; j++) {
            image->mat_vert[i][j] = new_mat_vert[i][j];
        }
    }
}


void set_mat_bleu(Image image, int** new_mat_bleu) {
    for (int i=0 ; i<image->largeur ; i++) {
        for (int j=0 ; j<image->hauteur ; j++) {
            image->mat_bleu[i][j] = new_mat_bleu[i][j];
        }
    }
}