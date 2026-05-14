/*
 * FICHIER: robot_final.ino
 * BASE:    robot_corrige_v31.ino (version la plus récente du projet)
 * MODIF:   Lecture des commandes sur Serial (USB/RPi) ET Serial1 (Bluetooth)
 *          La RPi envoie via câble USB → Serial
 *          Le téléphone/BT envoie via module HC-05 → Serial1
 */

#include <AFMotor_R4.h>

#define trigPin      30
#define echoPin      28
#define trigPinBack  31
#define echoPinBack  29
#define klaxonPin    50

#define INTERVALLE_SONAR 80

// ─────────────────────── MODE ASPIRATEUR ──────────────────────────
enum EtatAuto { IDLE, AVANCE_AUTO, TOURNE_AUTO };
EtatAuto etatAuto    = IDLE;
bool modeAspirateur  = false;
bool tourneGauche    = true;
unsigned long debutVirage = 0;
unsigned long dureeVirage = 0;

// ─────────────────────── PARSING COMMANDE ─────────────────────────
// Buffer partagé USB + BT
String commandeBuffer    = "";
unsigned long dernierCar = 0;
const unsigned long TIMEOUT_CMD = 50;

long distanceFront = 100;
long distanceBack  = 100;

bool avance       = false;
bool recule       = false;
bool radarAvant   = false;
bool radarArriere = false;

int vitesse = 200;

unsigned long derniereSonarAvant   = 0;
unsigned long derniereSonarArriere = 0;

// Radar avant
unsigned long dernierBipAvant = 0;
bool bipAvantEnCours           = false;
unsigned long debutBipAvant    = 0;

// Radar arrière
unsigned long dernierBip = 0;
bool bipEnCours           = false;
unsigned long debutBip    = 0;

AF_DCMotor motorAD(1);
AF_DCMotor motorAG(2);
AF_DCMotor motorDG(3);
AF_DCMotor motorDD(4);

// ───────────────────────────── SETUP ──────────────────────────────
void setup() {
  Serial.begin(9600);   // USB → Raspberry Pi (via câble)
  Serial1.begin(9600);  // TX1/RX1 → Module Bluetooth HC-05

  pinMode(trigPin,     OUTPUT);
  pinMode(echoPin,     INPUT);
  pinMode(trigPinBack, OUTPUT);
  pinMode(echoPinBack, INPUT);
  pinMode(klaxonPin,   OUTPUT);

  randomSeed(analogRead(A0));
  appliquerVitesse();
  Serial.println("Robot pret - USB + BT actifs");
}

// ───────────────────────────── LOOP ───────────────────────────────
void loop() {
  unsigned long maintenant = millis();

  // -------- MODE ASPIRATEUR --------
  if (modeAspirateur) {
    gererAspirateur(maintenant);
  }

  // -------- AVANCE + RADAR AVANT --------
  if (!modeAspirateur && (avance || radarAvant)) {
    if (maintenant - derniereSonarAvant >= INTERVALLE_SONAR) {
      mesurerDistanceAvant();
      derniereSonarAvant = maintenant;
    }
    if (avance && distanceFront <= 50) {
      stopRobot();
      avance = false; radarAvant = true; bipAvantEnCours = false;
      Serial.println("Obstacle devant - STOP");
    }
    radarBipAvant(distanceFront, maintenant);
    if (radarAvant && !avance && distanceFront > 75) {
      radarAvant = false; noTone(klaxonPin);
    }
  }

  // -------- RECUL + RADAR ARRIERE --------
  if (!modeAspirateur && (recule || radarArriere)) {
    if (maintenant - derniereSonarArriere >= INTERVALLE_SONAR) {
      mesurerDistanceArriere();
      derniereSonarArriere = maintenant;
    }
    if (recule && distanceBack <= 50) {
      stopRobot();
      recule = false; radarArriere = true; bipEnCours = false;
      Serial.println("Obstacle derriere - STOP");
    }
    radarBipArriere(distanceBack, maintenant);
    if (radarArriere && !recule && distanceBack > 75) {
      radarArriere = false; noTone(klaxonPin);
    }
  }

  // ================================================================
  // LECTURE COMMANDES : Serial (USB/RPi) ET Serial1 (Bluetooth)
  // Les deux alimentent le même commandeBuffer
  // ================================================================
  while (Serial.available()) {
    char c = Serial.read();
    commandeBuffer += c;
    dernierCar = maintenant;
  }
  while (Serial1.available()) {
    char c = Serial1.read();
    commandeBuffer += c;
    dernierCar = maintenant;
  }

  // Commande complète = silence depuis TIMEOUT_CMD ms
  if (commandeBuffer.length() > 0 && (maintenant - dernierCar >= TIMEOUT_CMD)) {
    commandeBuffer.trim();
    int cmd = commandeBuffer.toInt();
    Serial.print("CMD recu : "); Serial.println(cmd);
    commandeBuffer = "";
    traiterCommande(cmd, maintenant);
  }
}

// ─────────────────────── MODE ASPIRATEUR ──────────────────────────
void gererAspirateur(unsigned long maintenant) {
  if (maintenant - derniereSonarAvant >= INTERVALLE_SONAR) {
    mesurerDistanceAvant();
    derniereSonarAvant = maintenant;
  }
  switch (etatAuto) {
    case AVANCE_AUTO:
      if (distanceFront <= 50) {
        stopRobot();
        tone(klaxonPin, 800); delay(60); noTone(klaxonPin);
        dureeVirage = random(400, 900);
        debutVirage = maintenant;
        if (tourneGauche) gauche(); else droite();
        tourneGauche = !tourneGauche;
        etatAuto = TOURNE_AUTO;
      }
      break;
    case TOURNE_AUTO:
      if (maintenant - debutVirage >= dureeVirage) {
        avancer();
        etatAuto = AVANCE_AUTO;
      }
      break;
    default: break;
  }
}

void activerAspirateur() {
  modeAspirateur = true; etatAuto = AVANCE_AUTO;
  avance = false; recule = false; radarAvant = false; radarArriere = false;
  bipAvantEnCours = false; bipEnCours = false;
  noTone(klaxonPin);
  avancer();
  Serial.println("Aspirateur ON");
}

void stopAspirateur() {
  modeAspirateur = false; etatAuto = IDLE;
  stopRobot(); noTone(klaxonPin);
  Serial.println("Aspirateur OFF");
}

// ─────────────────────── TRAITEMENT COMMANDES ─────────────────────
void traiterCommande(int cmd, unsigned long maintenant) {

  if (cmd == 3 && modeAspirateur) { stopAspirateur(); return; }
  if (modeAspirateur) return;

  switch (cmd) {
    case 1:
      mesurerDistanceAvant(); derniereSonarAvant = maintenant;
      if (distanceFront > 50) {
        avancer(); avance = true; radarAvant = true;
        recule = false; radarArriere = false; bipEnCours = false; noTone(klaxonPin);
      } else Serial.println("Obstacle devant");
      break;

    case 2:
      mesurerDistanceArriere(); derniereSonarArriere = maintenant;
      if (distanceBack > 50) {
        reculer(); recule = true; radarArriere = true;
        avance = false; radarAvant = false; bipAvantEnCours = false; noTone(klaxonPin);
      } else Serial.println("Obstacle derriere");
      break;

    case 3:
      stopRobot();
      avance = false; recule = false; radarAvant = false; radarArriere = false;
      bipAvantEnCours = false; bipEnCours = false; noTone(klaxonPin);
      break;

    case 4:
      gauche();
      avance = false; recule = false; radarAvant = false; radarArriere = false;
      bipAvantEnCours = false; bipEnCours = false; noTone(klaxonPin);
      break;

    case 5:
      droite();
      avance = false; recule = false; radarAvant = false; radarArriere = false;
      bipAvantEnCours = false; bipEnCours = false; noTone(klaxonPin);
      break;

    case 6:
      vitesse = min(vitesse + 20, 255); appliquerVitesse();
      Serial.print("Vitesse : "); Serial.println(vitesse);
      break;

    case 7:
      vitesse = max(vitesse - 20, 0); appliquerVitesse();
      Serial.print("Vitesse : "); Serial.println(vitesse);
      break;

    case 8:
      tone(klaxonPin, 900); delay(100); noTone(klaxonPin);
      break;

    case 10:
      activerAspirateur();
      break;
  }
}

// ─────────────────────── RADAR AVANT ──────────────────────────────
void radarBipAvant(long distance, unsigned long maintenant) {
  if (distance > 75)  { noTone(klaxonPin); bipAvantEnCours = false; return; }
  if (distance <= 50) { tone(klaxonPin, 1000); bipAvantEnCours = false; return; }
  const unsigned long dureeBip = 80, intervalle = 600;
  if (bipAvantEnCours) {
    if (maintenant - debutBipAvant >= dureeBip)
      { noTone(klaxonPin); bipAvantEnCours = false; dernierBipAvant = maintenant; }
  } else {
    if (maintenant - dernierBipAvant >= intervalle)
      { tone(klaxonPin, 1000); bipAvantEnCours = true; debutBipAvant = maintenant; }
  }
}

// ─────────────────────── RADAR ARRIERE ────────────────────────────
void radarBipArriere(long distance, unsigned long maintenant) {
  if (distance > 75)  { noTone(klaxonPin); bipEnCours = false; return; }
  if (distance <= 50) { tone(klaxonPin, 700); bipEnCours = false; return; }
  const unsigned long dureeBip = 80, intervalle = 600;
  if (bipEnCours) {
    if (maintenant - debutBip >= dureeBip)
      { noTone(klaxonPin); bipEnCours = false; dernierBip = maintenant; }
  } else {
    if (maintenant - dernierBip >= intervalle)
      { tone(klaxonPin, 700); bipEnCours = true; debutBip = maintenant; }
  }
}

// ─────────────────────── SONAR ────────────────────────────────────
long mesurerDistance(int trigP, int echoP) {
  digitalWrite(trigP, LOW); delayMicroseconds(2);
  digitalWrite(trigP, HIGH); delayMicroseconds(10);
  digitalWrite(trigP, LOW);
  long dur = pulseIn(echoP, HIGH, 25000);
  if (dur == 0) return 999;
  return (dur / 2) / 29.1;
}

void mesurerDistanceAvant()   { distanceFront = mesurerDistance(trigPin, echoPin);         Serial.print("Avant : ");   Serial.println(distanceFront); }
void mesurerDistanceArriere() { distanceBack  = mesurerDistance(trigPinBack, echoPinBack); Serial.print("Arriere : "); Serial.println(distanceBack);  }

// ─────────────────────── MOTEURS ──────────────────────────────────
void appliquerVitesse() {
  motorAD.setSpeed(vitesse); motorAG.setSpeed(vitesse);
  motorDG.setSpeed(vitesse); motorDD.setSpeed(vitesse);
}
void avancer()  { motorAD.run(FORWARD);  motorAG.run(FORWARD);  motorDG.run(FORWARD);  motorDD.run(FORWARD);  }
void reculer()  { motorAD.run(BACKWARD); motorAG.run(BACKWARD); motorDG.run(BACKWARD); motorDD.run(BACKWARD); }
void gauche()   { motorAD.run(FORWARD);  motorAG.run(BACKWARD); motorDG.run(BACKWARD); motorDD.run(FORWARD);  }
void droite()   { motorAD.run(BACKWARD); motorAG.run(FORWARD);  motorDG.run(FORWARD);  motorDD.run(BACKWARD); }
void stopRobot(){ motorAD.run(RELEASE);  motorAG.run(RELEASE);  motorDG.run(RELEASE);  motorDD.run(RELEASE);  }
