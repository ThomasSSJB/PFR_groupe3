#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "commande_vocale.h"

#define FICHIER_COMMANDE "commande.txt"
#define FICHIER_ACTION "action.txt"

/* fichiers JSON contenant le vocabulaire */
#define FR_JSON "config/fr.json"
#define EN_JSON "config/en.json"
#define ES_JSON "config/es.json"

#define TAILLE_MAX 4096
#define TAILLE_CMD 512
/* Met toute la chaîne en minuscules */
void mettre_en_minuscules(char *texte) {
    for (int i = 0; texte[i]; i++) {
        texte[i] = tolower((unsigned char)texte[i]);
    }
}

void effacer_commande(void)
{
    FILE *f = fopen(FICHIER_COMMANDE, "w");
    if (f != NULL)
    {
        fclose(f);
    }
}

/* Vérifie si un mot est inutile*/
int mot_inutile(const char *mot) {
    const char *liste[] = {
        "bonjour", "salut", "hello", "hola",
        "tu", "vous", "peux", "please",
        "de", "a", "et", "the", "and",
        "le", "la", "les", "el", "los",
        "si", "sil", "te",
        NULL
    };

    for (int i = 0; liste[i]; i++) {
        if (strcmp(mot, liste[i]) == 0)
            return 1;
    }
    return 0;
}

int lancer_python() {
    int res = system("python python/assistant_vocal.py");
    if (res != 0) {
        printf("[ERREUR] Python n'a pas pu s'exécuter !\n");
        return 0;
    }
    return 1;
}

int lancer_simulation() {
    int res = system("python python/simulation.py");
    if (res != 0) {
        printf("[ERREUR] simulation n'a pas pu s'exécuter !\n");
        return 0;
    }
    return 1;
}

/* Lit la commande écrite par Python                 */
int lire_commande(char *buffer, int taille) {
    FILE *f = fopen(FICHIER_COMMANDE, "r");
    if (!f) {
        printf("[ERREUR] Impossible d'ouvrir %s\n", FICHIER_COMMANDE);
        return 0;
    }

    fgets(buffer, taille, f);
    fclose(f);

    buffer[strcspn(buffer, "\n")] = '\0';
    mettre_en_minuscules(buffer);
    
    printf("[TRACE] Commande recue : %s\n", buffer);
    return 1;
}

/* Supprime les mots inutiles et garde l essentiel   */
void filtrer_commande(const char *entree, char *sortie) {
    char tmp[TAILLE_CMD];
    strcpy(tmp, entree);

    sortie[0] = '\0';

    char *mot = strtok(tmp, " ");
    while (mot) {
        if (!mot_inutile(mot)) {
            strcat(sortie, mot);
            strcat(sortie, " ");
            
        }
        mot = strtok(NULL, " ");
    }
    
    printf("[TRACE] Commande filtree : %s\n", sortie);
}

/* Ecrit l action detectee dans le fichier action.txt */
void ecrire_action(const char *action) {
    FILE *f = fopen(FICHIER_ACTION, "w");
    if (!f) {
        printf("[ERREUR] Impossible d'ouvrir %s\n", FICHIER_ACTION);
        return;
    }

    fprintf(f, "%s\n", action);
    fclose(f);
}

/* Charge un fichier JSON complet dans une chaîne    */
int charger_json(const char *chemin, char *contenu) {
    FILE *f = fopen(chemin, "r");
    if (!f) return 0;

    int n = fread(contenu, 1, TAILLE_MAX - 1, f);
    contenu[n] = '\0';
    fclose(f);

    return 1;
}

/* Vérifie si au moins un mot de la commande est présent dans le fichier json        */
int commande_presente_dans_json(const char *json, const char *commande) {
    char tmp[TAILLE_CMD];
    strcpy(tmp, commande);

    char *mot = strtok(tmp, " ");
    while (mot) {
        if (strstr(json, mot)) {
            return 1;   /* mot trouvé dans le JSON */
        }
        mot = strtok(NULL, " ");
    }
    return 0;
}

void executer_action(const char *cmd) {

    if (strstr(cmd, "avance") || strstr(cmd, "forward") || strstr(cmd, "avanza")) {
        printf("[ACTION] Robot avance\n");
        ecrire_action("avance");
        lancer_simulation();
        return;
    }

    if (strstr(cmd, "gauche") || strstr(cmd, "left") || strstr(cmd, "izquierda")) {
        printf("[ACTION] Robot tourne a gauche\n");
        ecrire_action("gauche");
        lancer_simulation();
        return;
    }

    if (strstr(cmd, "droite") || strstr(cmd, "right") || strstr(cmd, "derecha")) {
        printf("[ACTION] Robot tourne a droite\n");
        ecrire_action("droite");
        lancer_simulation();
        return;
    }

    if (strstr(cmd, "stop") || strstr(cmd, "arrete") || strstr(cmd, "para")) {
        printf("[ACTION] Robot stop\n");
        ecrire_action("stop");
        lancer_simulation();
        return;
    }
}



/* Fonction principale appelée par le main           */
void traiter_commande(void) {
    char brute[TAILLE_CMD];
    char filtre[TAILLE_CMD];
    char json[TAILLE_MAX];
    lancer_python();
    int commande_présente= lire_commande(brute, TAILLE_CMD);
    if (!commande_présente)
        return;
    else
    effacer_commande();
    filtrer_commande(brute, filtre);


    if (charger_json(FR_JSON, json) &&
        commande_presente_dans_json(json, filtre)) {
        printf("[INFO] Langue detectee : francais\n");
        executer_action(filtre);
        return;
    }

    if (charger_json(EN_JSON, json) &&
        commande_presente_dans_json(json, filtre)) {
        printf("[INFO] Langue detectee : anglais\n");
        executer_action(filtre);
        return;
    }

    if (charger_json(ES_JSON, json) &&
        commande_presente_dans_json(json, filtre)) {
        printf("[INFO] Langue detectee : espagnol\n");
        executer_action(filtre);
        return;
    }

    printf("[ERREUR] Commande non reconnue dans les fichiers JSON\n");
}
