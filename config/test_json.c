#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------- Fonctions utilitaires ----------------
int est_espace(char c) {
    return (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\v' || c == '\f');
}

char* sauter_espaces(char *p) {
    while (*p && est_espace(*p))
        p++;
    return p;
}

// Lire un entier après "nom_cle"
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

// Lire une chaîne après "nom_cle"
int lire_chaine_json(const char *nom_cle, const char *texte_json, char resultat[128]) {
    char recherche[64];
    sprintf(recherche, "\"%s\"", nom_cle);
    char *pos = strstr(texte_json, recherche);
    if (!pos) return 0;
    pos = strchr(pos, ':');
    if (!pos) return 0;
    pos++;
    pos = sauter_espaces(pos);
    if (*pos == '"') pos++; // début chaîne
    int i = 0;
    while (*pos && *pos != '"' && i < 127) resultat[i++] = *pos++;
    resultat[i] = '\0';
    return 1;
}

// Lire un tableau de chaînes JSON ["a","b",...] pour une clé donnée
int lire_tableau_chaines_json(const char *nom_cle, const char *texte_json, char resultat[][64], int *nb) {
    char recherche[64];
    sprintf(recherche, "\"%s\"", nom_cle);
    char *pos = strstr(texte_json, recherche);
    if (!pos) return 0;
    pos = strchr(pos, '[');
    if (!pos) return 0;
    pos++; // après '['
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
        pos = strchr(pos, '"'); // chercher fin de chaîne
        if (!pos) break;
        pos++; // après '"'
        pos = strchr(pos, ','); // chercher prochaine virgule
        if (!pos) break;
        pos++; // après ','
    }
    *nb = i;
    return 1;
}

// Charger fichier JSON
char* charger_fichier_json(const char *nom_fichier) {
    FILE *f = fopen(nom_fichier, "rb");
    if (!f) { printf("Erreur : impossible d'ouvrir %s\n", nom_fichier); return NULL; }
    fseek(f, 0, SEEK_END);
    long taille = ftell(f);
    rewind(f);
    char *texte = malloc(taille + 1);
    fread(texte, 1, taille, f);
    texte[taille] = '\0';
    fclose(f);
    return texte;
}

// ---------------- Programme principal ----------------
int main() {
    // Charger config principale
    char *texte_json = charger_fichier_json("config_base.json");
    if (!texte_json) return 1;

    int seuil_forme = lire_valeur_json("seuil_forme", texte_json);
    int seuil_couleur = lire_valeur_json("seuil_couleur", texte_json);
    printf("seuil_forme   = %d\n", seuil_forme);
    printf("seuil_couleur = %d\n", seuil_couleur);

    // Choix de la langue
    printf("Langues disponibles :\n1. Français (fr)\n2. Anglais (en)\n3. Espagnol (es)\n");
    int choix = 0;
    printf("Choisissez une langue (1-3) : ");
    scanf("%d", &choix);

    char langue[3];
    switch (choix) {
        case 1: strcpy(langue, "fr"); break;
        case 2: strcpy(langue, "en"); break;
        case 3: strcpy(langue, "es"); break;
        default: printf("Choix invalide.\n"); free(texte_json); return 1;
    }
    printf("Vous avez choisi : %s\n", langue);

    // Charger JSON langue
    char fichier_langue[16];
    sprintf(fichier_langue, "%s.json", langue);
    char *texte_langue = charger_fichier_json(fichier_langue);
    if (!texte_langue) { free(texte_json); return 1; }

    // Lire message bienvenue
    char message_bienvenue[128];
    if (strcmp(langue, "fr")==0)
        lire_chaine_json("texte_bienvenue", texte_langue, message_bienvenue);
    else if (strcmp(langue, "en")==0)
        lire_chaine_json("welcome_text", texte_langue, message_bienvenue);
    else if (strcmp(langue, "es")==0)
        lire_chaine_json("texto_bienvenida", texte_langue, message_bienvenue);
    printf("%s\n", message_bienvenue);

    // Afficher toutes les commandes pour avancer
    char commandes_avancer[10][64];
    int nb_commandes = 0;
    if (strcmp(langue, "fr")==0)
        lire_tableau_chaines_json("avance", texte_langue, commandes_avancer, &nb_commandes);
    else if (strcmp(langue, "en")==0)
        lire_tableau_chaines_json("forward", texte_langue, commandes_avancer, &nb_commandes);
    else if (strcmp(langue, "es")==0)
        lire_tableau_chaines_json("avanzar", texte_langue, commandes_avancer, &nb_commandes);

    printf("Commandes pour avancer (%s) :\n", langue);
    for (int i = 0; i < nb_commandes; i++) {
        printf("- %s\n", commandes_avancer[i]);
    }

    // Libération mémoire
    free(texte_json);
    free(texte_langue);

    return 0;
}
