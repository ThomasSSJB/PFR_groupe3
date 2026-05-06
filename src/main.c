/* FICHIER: main.c
 * AUTEURS: GRELET Thomas, YAHYAOUI Nidal
 * RÔLE: Point d'entrée du programme principal (intégration)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/commande_vocale.h"
#include "../include/config.h"
#include "../include/image.h"
#include "../include/objet.h"
#include "../include/utils.h"

#define PYTHON_CMD_VOC "python3 python/assistant_vocal.py"
#define PYTHON_SIMU "python3 python/simulation.py"
#define PYTHON_PILOTE "python3 python/pilote_robot.py"

/* ================= CARTOGRAPHIE LiDAR ================= */
#define PYTHON_LIDAR ". ~/code/PFR_groupe3/venv/bin/activate && python3 python/scan_lidar.py | python3 python/display_map.py"
/*"ssh pfr3@pfr3.local \"source ~/code/PFR_groupe3/venv/bin/activate && python ~/code/PFR_groupe3/python/scan_lidar.py\" | /opt/miniconda3/envs/pfr_lidar/bin/python python/display_map.py"
 */
int main(void)
{
    int choix = 0;
    int en_cours = 1;

    /* ================= RESET action.txt AU DÉMARRAGE ================= */
    FILE *f = fopen("data/action.txt", "w");

    if (f)
        fclose(f);

    /* ================================================================= */

    /* Charger la configuration */
    if (!charger_config("config/config.json"))
    {
        return 1;
    }

    printf("\n=========================================\n");
    printf("      SYSTEME DE COMMANDE ROBOT\n");
    printf("=========================================\n");

    while (en_cours)
    {
        printf("\n---------- MENU PRINCIPAL ----------\n");

        printf("1 - Ecrire une commande (clavier)\n");
        printf("2 - Parler (commande vocale)\n");
        printf("3 - Lancer la simulation (turtle)\n");
        printf("4 - Lancer le robot\n");
        printf("5 - Lancer la cartographie LiDAR\n");
        printf("6 - Arreter le programme\n");

        printf("------------------------------------\n");
        printf("-> Choix : ");

        if (scanf("%d", &choix) != 1)
        {
            while (getchar() != '\n');

            printf("Erreur de saisie.\n");
            continue;
        }

        getchar(); /* consomme le \n */

        switch (choix)
        {
            /* ========================================= */
            /* COMMANDE CLAVIER */
            /* ========================================= */
            case 1:

                printf("\n[MODE CLAVIER]\n");

                saisir_clavier();

                traiter_commande();

                break;

            /* ========================================= */
            /* COMMANDE VOCALE */
            /* ========================================= */
            case 2:

                printf("\n[MODE VOCAL]\n");

                system(PYTHON_CMD_VOC);

                traiter_commande();

                break;

            /* ========================================= */
            /* SIMULATION */
            /* ========================================= */
            case 3:

                printf("\n[LANCEMENT SIMULATION]\n");

                system(PYTHON_SIMU);

                break;

            /* ========================================= */
            /* PILOTAGE ROBOT */
            /* ========================================= */
            case 4:

                printf("\n[LANCEMENT ROBOT]\n");

                system(PYTHON_PILOTE);

                break;

            /* ========================================= */
            /* CARTOGRAPHIE LiDAR */
            /* ========================================= */
            case 5:

                printf("\n[LANCEMENT CARTOGRAPHIE LiDAR]\n");

                system(PYTHON_LIDAR);

                break;

            /* ========================================= */
            /* QUITTER */
            /* ========================================= */
            case 6:

                printf("\nArret du programme...\n");

                en_cours = 0;

                break;

            default:

                printf("Choix invalide.\n");

                break;
        }
    }

    printf("\nFin du programme.\n");

    return 0;
}