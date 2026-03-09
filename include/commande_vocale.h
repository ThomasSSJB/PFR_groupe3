/* FICHIER: commande_vocale.h
*  AUTEURS: YAHYAOUI Nidal, BEN LTAIEF Yasmine
*  RÔLE: Déclaration des fonctions de traitement de la commande vocale
**/

#ifndef COMMANDE_VOCALE_H
#define COMMANDE_VOCALE_H

/* Fonction principale de traitement
 * - lit commande.txt
 * - filtre via les JSON
 * - génère les tokens
 * - exécute les actions
 */
void traiter_commande(void);

#endif