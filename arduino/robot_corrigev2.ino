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
bool radarArriere = false;  // radar actif meme robot arrete

int vitesse = 200;

// millis() pour le bip radar sans bloquer
unsigned long dernierBip = 0;
bool bipEnCours = false;
unsigned long debutBip = 0;

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

  distanceFront = 100;
  distanceBack = 100;

  appliquerVitesse();

  Serial.println("Robot pret");
}

void loop() {

  unsigned long maintenant = millis();

  // -------- AVANCE --------
  if (avance) {
    mesurerDistanceAvant();
    alarmeAvant(distanceFront);

    if (distanceFront <= 50) {
      stopRobot();
      noTone(klaxonPin);
      avance = false;
      Serial.println("Obstacle devant - STOP");
    }
  }

  // -------- RECUL + RADAR --------
  if (recule || radarArriere) {

    mesurerDistanceArriere();

    if (recule && distanceBack <= 50) {
      stopRobot();
      recule = false;
      radarArriere = true;  // passe en mode radar sans bouger
      Serial.println("Obstacle derriere - STOP radar actif");
    }

    // bip radar non-bloquant
    radarBip(distanceBack, maintenant);

    // obstacle disparu -> radar off
    if (radarArriere && distanceBack > 75) {
      radarArriere = false;
      noTone(klaxonPin);
      Serial.println("Voie libre - radar off");
    }
  }

  // -------- COMMANDES BLUETOOTH --------
  while (Serial1.available()) {

    commande = Serial1.read();
    Serial.println(commande);

    if (commande == '1') {
      mesurerDistanceAvant();
      if (distanceFront > 50) {
        avancer();
        avance = true;
        recule = false;
        radarArriere = false;
        noTone(klaxonPin);
      } else {
        Serial.println("Obstacle devant - avance impossible");
      }
    }

    if (commande == '2') {
      mesurerDistanceArriere();
      if (distanceBack > 50) {
        reculer();
        recule = true;
        radarArriere = true;
        avance = false;
      } else {
        Serial.println("Obstacle derriere - recul impossible");
      }
    }

    if (commande == '3') {
      stopRobot();
      avance = false;
      recule = false;
      radarArriere = false;
      noTone(klaxonPin);
    }

    if (commande == '4') {
      gauche();
      avance = false;
      recule = false;
      radarArriere = false;
      noTone(klaxonPin);
    }

    if (commande == '5') {
      droite();
      avance = false;
      recule = false;
      radarArriere = false;
      noTone(klaxonPin);
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

    if (commande == '8') {
      tone(klaxonPin, 900);
      delay(100);
      noTone(klaxonPin);
    }
  }
}

// Bip radar non-bloquant avec millis()
// Plus l obstacle est proche, plus les bips sont rapides
void radarBip(long distance, unsigned long maintenant) {

  if (distance > 75) {
    noTone(klaxonPin);
    bipEnCours = false;
    return;
  }

  // Bip continu si trop pres
  if (distance <= 50) {
    tone(klaxonPin, 700);
    bipEnCours = false;
    return;
  }

  // Intervalle entre bips selon distance
  unsigned long intervalle;
  unsigned long dureeBip = 80;

  if (distance <= 50) {
    intervalle = 250;
  } else {
    intervalle = 600;
  }

  if (bipEnCours) {
    if (maintenant - debutBip >= dureeBip) {
      noTone(klaxonPin);
      bipEnCours = false;
      dernierBip = maintenant;
    }
  } else {
    if (maintenant - dernierBip >= intervalle) {
      tone(klaxonPin, 700);
      bipEnCours = true;
      debutBip = maintenant;
    }
  }
}

void appliquerVitesse() {
  motorAD.setSpeed(vitesse);
  motorAG.setSpeed(vitesse);
  motorDG.setSpeed(vitesse);
  motorDD.setSpeed(vitesse);
}

void mesurerDistanceAvant() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  durationFront = pulseIn(echoPin, HIGH, 30000); // timeout 30ms
  if (durationFront == 0) distanceFront = 999;
  distanceFront = (durationFront / 2) / 29.1;
  Serial.print("Avant : ");
  Serial.println(distanceFront);
}

void mesurerDistanceArriere() {
  digitalWrite(trigPinBack, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinBack, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinBack, LOW);
  durationBack = pulseIn(echoPinBack, HIGH);
  distanceBack = (durationBack / 2) / 29.1;
  Serial.print("Arriere : ");
  Serial.println(distanceBack);
}

void avancer() {
  motorAD.run(FORWARD);
  motorAG.run(FORWARD);
  motorDG.run(FORWARD);
  motorDD.run(FORWARD);
}

void reculer() {
  motorAD.run(BACKWARD);
  motorAG.run(BACKWARD);
  motorDG.run(BACKWARD);
  motorDD.run(BACKWARD);
}

void gauche() {
  motorAD.run(FORWARD);
  motorAG.run(BACKWARD);
  motorDG.run(BACKWARD);
  motorDD.run(FORWARD);
}

void droite() {
  motorAD.run(BACKWARD);
  motorAG.run(FORWARD);
  motorDG.run(FORWARD);
  motorDD.run(BACKWARD);
}

void stopRobot() {
  motorAD.run(RELEASE);
  motorAG.run(RELEASE);
  motorDG.run(RELEASE);
  motorDD.run(RELEASE);
}

void alarmeAvant(long distance) {
  if (distance <= 50) {
    tone(klaxonPin, 1000);
  } else if (distance <= 75) {
    tone(klaxonPin, 1000);
    delay(120);
    noTone(klaxonPin);
    delay(120);
  } else if (distance <= 100) {
    tone(klaxonPin, 1000);
    delay(300);
    noTone(klaxonPin);
    delay(500);
  } else {
    noTone(klaxonPin);
  }
}
