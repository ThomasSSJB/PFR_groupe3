/* FICHIER: config.h
*  AUTEUR: GRELET Thomas (basé sur le travail de DEVAUD Antoine dans test_json.c)
*  RÔLE: Déclaration des fonctions de chargement et de lecture de fichiers JSON
**/

#ifndef CONFIG_H
#define CONFIG_H

/* ========== VARIABLE GLOBALE ========== */

/* Texte JSON chargé en mémoire */
extern char* config_json;


/* ========== FONCTIONS DE CHARGEMENT ========== */

/* Charge la configuration depuis un fichier JSON */
int charger_config(const char* fichier);

/* Libère la mémoire de la configuration */
void liberer_config();


/* ========== FONCTIONS UTILITAIRES JSON ========== */

/* Charge un fichier JSON en mémoire */
char* charger_fichier_json(const char *nom_fichier);

/* Lit une valeur entière depuis le JSON */
int lire_valeur_json(const char *nom_cle, const char *texte_json);

/* Lit une chaîne de caractères depuis le JSON */
int lire_chaine_json(const char *nom_cle, const char *texte_json, char resultat[128]);

/* Lit un tableau de chaînes depuis le JSON */
int lire_tableau_chaines_json(const char *nom_cle, const char *texte_json, char resultat[][64], int *nb);

#endif