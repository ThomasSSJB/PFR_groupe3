/* FICHIER: vroumv3.ino
*  AUTEUR: DEVAUD Antoine
*  RÔLE: *à remplir*
**/

#include <AFMotor_R4.h>

#define trigPin 25
#define echoPin 23

#define trigPinBack 31
#define echoPinBack 29

char commande;

long durationFront;
long distanceFront;

long durationBack;
long distanceBack;

bool avance = false;
bool recule = false;

// moteurs
AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

void setup() {

  Serial.begin(9600);
  Serial1.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(trigPinBack, OUTPUT);
  pinMode(echoPinBack, INPUT);

  motor1.setSpeed(240);
  motor2.setSpeed(240);
  motor3.setSpeed(240);
  motor4.setSpeed(240);

  Serial.println("Robot pret");
}

void loop() {

  mesurerDistanceAvant();
  mesurerDistanceArriere();

  // sécurité avant
  if (avance && distanceFront < 50) {
    stopRobot();
    avance = false;
    Serial.println("Obstacle devant !");
  }

  // sécurité arrière
  if (recule && distanceBack < 50) {
    stopRobot();
    recule = false;
    Serial.println("Obstacle derriere !");
  }

  // lecture bluetooth
  if (Serial1.available()) {

    commande = Serial1.read();
    Serial.println(commande);

    if (commande == 'F') { // avancer
      if (distanceFront > 50) {
        avancer();
        avance = true;
        recule = false;
      }
    }

    if (commande == 'B') { // reculer
      if (distanceBack > 50) {
        reculer();
        recule = true;
        avance = false;
      }
    }

    if (commande == 'S      ') { // stop
      stopRobot();
      avance = false;
      recule = false;
    }

    if (commande == 'L') { // gauche
      gauche();
      avance = false;
      recule = false;
    }

    if (commande == 'R') { // droite
      droite();
      avance = false;
      recule = false;
    }
  }
}

void mesurerDistanceAvant(){

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  durationFront = pulseIn(echoPin, HIGH);
  distanceFront = (durationFront / 2) / 29.1;

  Serial.print("Avant : ");
  Serial.println(distanceFront);
}

void mesurerDistanceArriere(){

  digitalWrite(trigPinBack, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPinBack, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPinBack, LOW);

  durationBack = pulseIn(echoPinBack, HIGH);
  distanceBack = (durationBack / 2) / 29.1;

  Serial.print("Arriere : ");
  Serial.println(distanceBack);
  Serial.println("  ");
}

// fonctions moteurs

void avancer(){
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void reculer(){
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
}

void gauche(){
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
}

void droite(){
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void stopRobot(){
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}
