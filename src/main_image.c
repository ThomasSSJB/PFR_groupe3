#include <stdio.h>
#include <stdlib.h>
#include "../include/image.h"
#include "../include/forme.h"

int main() {
    /* Initialisation des variables */
    Image image1 = init_image();
    int delta = 5;

    /* Charge une image */
    lire_image(&image1);

    /* Affichage de l'image binaire avec la boîte englobante */
    afficher_image_boite_englobante(image1, delta);

    printf("\n");

    return 0;
}
