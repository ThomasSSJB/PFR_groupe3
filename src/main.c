#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/config.h"
#include "../include/image.h"
#include "../include/objet.h"

int main(void){
    /* Charger la configuration */
    if (!charger_config("config/config.json")) {
        return 1;
    }

    /* Test intégration */
    Image image = lire_image();
    Objet balle_rouge = init_objet(0);
    int nb_objets = nombre_objets_image(image);

    for (int k=1 ; k<nb_objets+1 ; k++) {
        Objet objet_courant = init_objet(k);

        trouver_couleur_objet(objet_courant, image);
        trouver_nature_objet(objet_courant, image);

        if (get_couleur_objet(objet_courant) == ROUGE && !strcmp(get_nature_forme_objet(objet_courant), "Cercle")) {
            trouver_coordonnees_objet(objet_courant, image);
            balle_rouge = objet_courant;
            break;
        }
    }

    char* direction_objet = trouver_direction_objet(balle_rouge, image);
    int distance_objet = trouver_distance_objet(balle_rouge, image);
    int angle_objet = trouver_angle_objet(balle_rouge, image);

    commande_balle(direction_objet, angle_objet, distance_objet);


    /* Libération */    
    liberer_config();

    return 0;
}

