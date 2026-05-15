/* FICHIER: test_moteur4.ino
*  AUTEUR: DEVAUD Antoine
*  RÔLE: *Permet de tester les 4 moteurs du robot en pouvant les identifier et les contrôler individuellement*
**/

#include <AFMotor_R4.h>

// Test 4 moteurs avec le Shield L293D & la librairie "AFMotor.h
// -------------------------------------------------------------
  

AF_DCMotor motorAD(1);
AF_DCMotor motorAG(2);
AF_DCMotor motorDG(3);
AF_DCMotor motorDD(4);

void setup()
{
}

void loop()
{
// Vitesse des 4 moteurs (0 à 255)
//--------------------------------

motorAD.setSpeed(240);
motorAG.setSpeed(240);
motorDG.setSpeed(240);
motorDD.setSpeed(240);

// Marche avant des 4 moteurs pendant 2 secondes
// ---------------------------------------------

  motorAD.run(FORWARD);
  motorAG.run(FORWARD);
  motorDG.run(FORWARD);
  motorDD.run(FORWARD);
delay(2000);

  motorAD.run(BACKWARD);
  motorAG.run(BACKWARD);
  motorDG.run(BACKWARD);
  motorDD.run(BACKWARD);

delay(2000);
// Arrêt des 4 moteurs pendant 1 seconde
// -------------------------------------

motorAD.run(RELEASE);
motorAG.run(RELEASE);
motorDG.run(RELEASE);
motorDD.run(RELEASE);

}
