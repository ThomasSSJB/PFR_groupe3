/* FICHIER: vroum.ino
*  AUTEUR: DEVAUD Antoine
*  RÔLE: *à remplir*
**/

#include <AFMotor_R4.h>

char commande;

// moteurs
AF_DCMotor motor1(1); // gauche avant
AF_DCMotor motor2(2); // gauche arrière
AF_DCMotor motor3(3); // droite arrière
AF_DCMotor motor4(4); // droite avant

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  motor1.setSpeed(240);
  motor2.setSpeed(240);
  motor3.setSpeed(240);
  motor4.setSpeed(240);
}

void loop() {
  if (Serial1.available()) {
    commande = Serial1.read();
    Serial.println(commande); // debug

    if (commande == '1') {   // avancer
      motor1.run(FORWARD);
      motor2.run(FORWARD);
      motor3.run(FORWARD);
      motor4.run(FORWARD);
    }

    if (commande == '2') {   // reculer
      motor1.run(BACKWARD);
      motor2.run(BACKWARD);
      motor3.run(BACKWARD);
      motor4.run(BACKWARD);
    }

    if (commande == '3') {   // stop
      motor1.run(RELEASE);
      motor2.run(RELEASE);
      motor3.run(RELEASE);
      motor4.run(RELEASE);
    }

    if (commande == '4') {   // tourner à gauche
      motor1.run(BACKWARD); // gauche recule
      motor2.run(BACKWARD);
      motor3.run(FORWARD);  // droite avance
      motor4.run(FORWARD);
    }

    if (commande == '5') {   // tourner à droite
      motor1.run(FORWARD);  // gauche avance
      motor2.run(FORWARD);
      motor3.run(BACKWARD); // droite recule
      motor4.run(BACKWARD);
    }
  }
}
