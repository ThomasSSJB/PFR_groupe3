/* FICHIER: config.c
*  AUTEUR: GRELET Thomas (basé sur le travail de DEVAUD Antoine dans test_json.c)
*  RÔLE: Définition des fonctions de chargement et de lecture de fichiers JSON
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/config.h"

/* ========== VARIABLE GLOBALE ========== */
char* config_json = NULL;

/* ========== FONCTIONS UTILITAIRES ========== */

int est_espace(char c) {
    return (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\v' || c == '\f');
}

char* sauter_espaces(char *p) {
    while (*p && est_espace(*p))
        p++;
    return p;
}

char* charger_fichier_json(const char *nom_fichier) {
    FILE *f = fopen(nom_fichier, "rb");
    if (!f) { 
        fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", nom_fichier);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long taille = ftell(f);
    rewind(f);
    char *texte = malloc(taille + 1);
    fread(texte, 1, taille, f);
    texte[taille] = '\0';
    fclose(f);
    return texte;
}

int lire_valeur_json(const char *nom_cle, const char *texte_json) {
    char recherche[64];
    sprintf(recherche, "\"%s\"", nom_cle);
    char *pos = strstr(texte_json, recherche);
    if (!pos) return -1;
    pos = strchr(pos, ':');
    if (!pos) return -1;
    pos++;
    pos = sauter_espaces(pos);

    int valeur = 0, negatif = 0;
    if (*pos == '-') { negatif = 1; pos++; }
    while (*pos >= '0' && *pos <= '9') {
        valeur = valeur * 10 + (*pos - '0');
        pos++;
    }
    return negatif ? -valeur : valeur;
}

int lire_chaine_json(const char *nom_cle, const char *texte_json, char resultat[128]) {
    char recherche[64];
    sprintf(recherche, "\"%s\"", nom_cle);
    char *pos = strstr(texte_json, recherche);
    if (!pos) return 0;
    pos = strchr(pos, ':');
    if (!pos) return 0;
    pos++;
    pos = sauter_espaces(pos);
    if (*pos == '"') pos++;
    int i = 0;
    while (*pos && *pos != '"' && i < 127) resultat[i++] = *pos++;
    resultat[i] = '\0';
    return 1;
}

int lire_tableau_chaines_json(const char *nom_cle, const char *texte_json, char resultat[][64], int *nb) {
    char recherche[64];
    sprintf(recherche, "\"%s\"", nom_cle);
    char *pos = strstr(texte_json, recherche);
    if (!pos) return 0;
    pos = strchr(pos, '[');
    if (!pos) return 0;
    pos++;
    int i = 0;
    while (*pos && *pos != ']') {
        pos = sauter_espaces(pos);
        if (*pos == '"') pos++;
        int j = 0;
        while (*pos && *pos != '"' && j < 63) {
            resultat[i][j++] = *pos++;
        }
        resultat[i][j] = '\0';
        i++;
        pos = strchr(pos, '"');
        if (!pos) break;
        pos++;
        pos = strchr(pos, ',');
        if (!pos) break;
        pos++;
    }
    *nb = i;
    return 1;
}

/* ========== FONCTIONS DE GESTION CONFIG ========== */

int charger_config(const char* fichier) {
    /* Libérer l'ancienne config si elle existe */
    if (config_json != NULL) {
        free(config_json);
    }
    
    /* Charger le nouveau fichier */
    config_json = charger_fichier_json(fichier);
    if (!config_json) {
        fprintf(stderr, "Erreur : impossible de charger %s\n", fichier);
        return 0;
    }
    
    return 1;
}

void liberer_config() {
    if (config_json) {
        free(config_json);
        config_json = NULL;
    }
}