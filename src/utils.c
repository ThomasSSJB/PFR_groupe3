/* FICHIER: utils.c
*  AUTEURS: GRELET Thomas, DEVAUD Antoine, YAHYAOUI Nidal
*  RÔLE: Définition des fonctions utilitaires
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/utils.h"

void saisir_clavier(void) {
    char buffer[512];

    printf("Entrez votre commande :\n> ");
    fgets(buffer, sizeof(buffer), stdin);

    FILE *f = fopen(CMD_FILE, "w");
    if (!f) {
        perror("data/commande.txt");
        return;
    }

    fputs(buffer, f);
    fclose(f);
}


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


int max_tableau(int* tableau, int taille) {
    int max = tableau[0];
    for (int i=1 ; i<taille ; i++) {
        if (max < tableau[i]) max = tableau[i];
    }
    return max;
}


int max_matrice(int** matrice, int largeur, int hauteur) {
    int max = matrice[0][0];
    for (int i=0 ; i<largeur ; i++) {
        for (int j=0 ; j<hauteur ; j++) {
            if (matrice[i][j] > max) max = matrice[i][j];
        }
    }
    return max;
}