#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/image.h"
#include "../include/forme.h"
#include "../include/objet.h"
#include "../include/config.h"

int main() {
    /* Charger la configuration */
    if (!charger_config("config/config.json")) {
        return 1;
    }
    int seuil_quantif = lire_valeur_json("quantification_bits", config_json);
    printf("\n --> Nombre de bits utilisés pour la quantification : %d\n", seuil_quantif);
    
    /* Test caractéristiques des objets d'une image (localisation et couleur) */
    Image image1 = lire_image();
    Image sous_image;
    Couleur couleur_objet;
    int* tab_coord;
    int nb_objets = nombre_objets_image(image1);
    char* nature_forme;
    // afficher_image_boites_englobantes(image1, 5);

    for (int i=1 ; i<nb_objets+1 ; i++) {
        printf("\n===== Objet %d =====\n", i);

        tab_coord = trouver_coordonnees_forme(image1, i);
        printf("Coordonnées de l'objet : de (%d, %d) à (%d, %d)\n", tab_coord[0], tab_coord[1], tab_coord[2], tab_coord[3]);

        sous_image = sous_image_objet(image1, i, 3);
        couleur_objet = trouver_couleur_objet(sous_image);

        if (couleur_objet == ROUGE) printf("Couleur : ROUGE\n");
        else if (couleur_objet == VERT) printf("Couleur : VERT\n");
        else if (couleur_objet == BLEU) printf("Couleur : BLEU\n");
        else if (couleur_objet == JAUNE) printf("Couleur : JAUNE\n");
        else if (couleur_objet == GRIS) printf("Couleur indéterminée\n");
        
        nature_forme = trouver_nature_forme(sous_image);

        if (strcmp(nature_forme, "Cercle") == 0) printf("Forme : Cercle\n");
        else printf("Forme : indéterminée\n");
    }

    if (nb_objets == 0) printf("Aucun objet identifié sur l'image.\n");

    afficher_image_binaire(binariser_image(image1), get_largeur(image1), get_hauteur(image1));
    
    /* Libération */    
    liberer_config();

    return 0;
}