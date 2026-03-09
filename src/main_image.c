/* FICHIER: main.c
*  AUTEUR: GRELET Thomas, YAHYAOUI Nidal
*  RÔLE: Point d'entrée du programme image, permet d'effectuer les test des différents traitements
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/image.h"
#include "../include/objet.h"
#include "../include/config.h"

int main() {
    /* Charger la configuration */
    if (!charger_config("config/config.json")) {
        return 1;
    }
    int seuil_quantif = lire_valeur_json("quantification_bits", config_json);
    printf("\n --> Nombre de bits utilisés pour la quantification : %d\n", seuil_quantif);
    
    /* Définition de l'image à traiter */
    Image image = lire_image();
    int nb_objets = nombre_objets_image(image);
    char* direction_objet;
    int distance_objet;
    int angle_objet;

    printf("\nNombre d'objets sur l'image : %d\n", nb_objets);

    for (int k=1 ; k<nb_objets+1 ; k++) {
        printf("\n===== Objet %d =====\n", k);
        Objet objet_courant = init_objet(k);

        trouver_coordonnees_objet(objet_courant, image);
        trouver_couleur_objet(objet_courant, image);
        trouver_nature_objet(objet_courant, image);

        afficher_coordonnees_objet(objet_courant);
        afficher_couleur_objet(objet_courant);
        afficher_nature_objet(objet_courant);

        direction_objet = trouver_direction_objet(objet_courant, image);
        distance_objet = trouver_distance_objet(objet_courant, image);
        angle_objet = trouver_angle_objet(objet_courant, image);


        printf("Direction : %s\n", direction_objet);
        printf("Distance : %d px\n", distance_objet);
        printf("Angle : %d°\n", angle_objet);
    }

    /* Libération */    
    liberer_config();

    return 0;
}