#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/objet.h"
#include "../include/forme.h"
#include "../include/image.h"

struct Objet {
  Couleur couleur;
  Forme forme;
};


Objet init_objet(Couleur couleur, Forme forme) {
  Objet objet = malloc(sizeof(struct Objet));
  objet->couleur = couleur;
  objet->forme = forme;
  return objet;
}