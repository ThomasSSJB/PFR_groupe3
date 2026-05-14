#include <AFMotor_R4.h>

#define trigPin 30
#define echoPin 28

#define trigPinBack 31
#define echoPinBack 29

#define klaxonPin 50

char commande;

long durationFront;
long distanceFront;

long durationBack;
long distanceBack;

bool avance = false;
bool recule = false;

int vitesse = 200;

// moteurs
AF_DCMotor motorAD(1);
AF_DCMotor motorAG(2);
AF_DCMotor motorDG(3);
AF_DCMotor motorDD(4);

void setup() {

  Serial.begin(9600);
  Serial1.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(trigPinBack, OUTPUT);
  pinMode(echoPinBack, INPUT);

  pinMode(klaxonPin, OUTPUT);

  appliquerVitesse();

  Serial.println("Robot pret");
}

void loop() {
  

  // -------- AVANCE --------
  if(avance){

    mesurerDistanceAvant();
    alarmeAvant(distanceFront);

    if(distanceFront <= 35){
      stopRobot();
      avance = false;
      Serial.println("Obstacle devant - STOP");
    }

  }

  // -------- RECUL --------
  if(recule){

    mesurerDistanceArriere();
    alarmeArriere(distanceBack);

    if(distanceBack <= 35){
      stopRobot();
      recule = false;
      Serial.println("Obstacle derriere - STOP");
    }

  }

  if (Serial1.available()) {

    commande = Serial1.read();
    Serial.println(commande);

    if (commande == '1') {
      if (distanceFront > 35) {
        avancer();
        avance = true;
        recule = false;
      }
    }

    if (commande == '2') {
      if (distanceBack > 35) {
        reculer();
        recule = true;
        avance = false;
      }
    }

    if (commande == '3') {
      stopRobot();
      avance = false;
      recule = false;
    }

    if (commande == '4') {
      gauche();
      avance = false; 
      recule = false;
    }

    if (commande == '5') {
      droite();
      avance = false;
      recule = false;
    }

    if (commande == '6') {
      vitesse += 20;
      if (vitesse > 255) vitesse = 255;
      appliquerVitesse();
      Serial.print("Vitesse : ");
      Serial.println(vitesse);
    }

    if (commande == '7') {
      vitesse -= 20;
      if (vitesse < 0) vitesse = 0;
      appliquerVitesse();
      Serial.print("Vitesse : ");
      Serial.println(vitesse);
    }

    // KLAXON
    if (commande == '8') {
      tone(klaxonPin, 900);  // son
      delay(300);
      noTone(klaxonPin);
    }
  }
}

void appliquerVitesse(){
  motorAD.setSpeed(vitesse);
  motorAG.setSpeed(vitesse);
  motorDG.setSpeed(vitesse);
  motorDD.setSpeed(vitesse);
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
  Serial.println(" ");
}

// moteurs

void avancer(){
  motorAD.run(FORWARD);
  motorAG.run(FORWARD);
  motorDG.run(FORWARD);
  motorDD.run(FORWARD);
}

void reculer(){
  motorAD.run(BACKWARD);
  motorAG.run(BACKWARD);
  motorDG.run(BACKWARD);
  motorDD.run(BACKWARD);
}

void gauche(){
  motorAD.run(FORWARD);
  motorAG.run(BACKWARD);
  motorDG.run(BACKWARD);
  motorDD.run(FORWARD);
}

void droite(){
  motorAD.run(BACKWARD);
  motorAG.run(FORWARD);
  motorDG.run(FORWARD);
  motorDD.run(BACKWARD);
}

void stopRobot(){
  motorAD.run(RELEASE);
  motorAG.run(RELEASE);
  motorDG.run(RELEASE);
  motorDD.run(RELEASE);
}
void alarmeAvant(long distance){

  if(distance <= 35){
    tone(klaxonPin, 1000); // bip continu
  }

  else if(distance <= 50){
    tone(klaxonPin, 1000);
    delay(120);
    noTone(klaxonPin);
    delay(120);
  }

  else if(distance <= 75){
    tone(klaxonPin, 1000);
    delay(300);
    noTone(klaxonPin);
    delay(500);
  }

  else{
    noTone(klaxonPin);
  }

}
void alarmeArriere(long distance){

  if(distance <= 35){
    tone(klaxonPin, 700);
  }

  else if(distance <= 50){
    tone(klaxonPin, 700);
    delay(120);
    noTone(klaxonPin);
    delay(120);
  }

  else if(distance <= 75){
    tone(klaxonPin, 700);
    delay(300);
    noTone(klaxonPin);
    delay(500);
  }

  else{
    noTone(klaxonPin);
  }

}
