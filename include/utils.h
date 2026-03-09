/* FICHIER: utils.h
*  AUTEUR: GRELET Thomas, DEVAUD Antoine, YAHYAOUI Nidal
*  RÔLE: Déclaration des fonctions utilitaires
**/

#ifndef UTILS_H
#define UTILS_H

#define CMD_FILE "data/commande.txt"

/* Permet de saisir du texte au clavier */
void saisir_clavier(void);

/* Permet de passer d'une valeur décimale à une valeur binaire (sur 8 bits) */
char* decimal_en_binaire(int decimal);

/* Permet de passer d'une valeur binaire à une valeur décimale */
int binaire_en_decimal(char* binaire);

/* Renvoie la valeur maximale d'un tableau */
int max_tableau(int* tableau, int taille);

/* Renvoie la valeur maximale d'une matrice */
int max_matrice(int** matrice, int largeur, int hauteur);

#endif