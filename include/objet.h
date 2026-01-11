#ifndef OBJET_H
#define OBJET_H

#include "../include/image.h"
#include "../include/forme.h"

typedef struct Objet * Objet;

/* Initialise un objet (allocation mémoire) */
Objet init_objet(Couleur couleur, Forme forme);

/* à coder */
Objet remplir_objet(void);

#endif