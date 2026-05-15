/* FICHIER: main_vfinal.ino
*  AUTEUR: DEVAUD Antoine
*  RÔLE: Firmware Arduino de contrôle d'un robot 4 roues,
      pilotable en Bluetooth ou en mode autonome avec évitement d'obstacles par ultrasons.
**/

#include <AFMotor_R4.h>

#define trigPin      30
#define echoPin      28
#define trigPinBack  31
#define echoPinBack  29
#define klaxonPin    50

#define INTERVALLE_SONAR 80

// ─────────────────────── MACHINE D'ÉTATS AUTO ────────────────────
enum EtatAuto { IDLE, AVANCE_AUTO, TOURNE_AUTO };
EtatAuto etatAuto = IDLE;

bool modeAuto             = false;
unsigned long debutVirage = 0;

const unsigned long DUREE_TOURNE  = 400;   // durée du virage (~30°), à ajuster
const int           VITESSE_VIRAGE = 215;  // vitesse boostée pendant le virage

// ─────────────────────── PARSING COMMANDE ────────────────────────
String commandeBuffer = "";
unsigned long dernierCaractere = 0;
const unsigned long TIMEOUT_CMD = 50;

char commande;
long distanceFront = 100;
long distanceBack  = 100;

bool avance       = false;
bool recule       = false;
bool radarAvant   = false;
bool radarArriere = false;

int vitesse = 200;

unsigned long derniereSonarAvant   = 0;
unsigned long derniereSonarArriere = 0;

unsigned long dernierBipAvant  = 0;
bool bipAvantEnCours            = false;
unsigned long debutBipAvant     = 0;

unsigned long dernierBip  = 0;
bool bipEnCours            = false;
unsigned long debutBip     = 0;

AF_DCMotor motorAD(1);
AF_DCMotor motorAG(2);
AF_DCMotor motorDG(3);
AF_DCMotor motorDD(4);

// ───────────────────────────── SETUP ─────────────────────────────
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  pinMode(trigPin,     OUTPUT);
  pinMode(echoPin,     INPUT);
  pinMode(trigPinBack, OUTPUT);
  pinMode(echoPinBack, INPUT);
  pinMode(klaxonPin,   OUTPUT);

  appliquerVitesse();
  Serial.println("Robot pret");
}

// ───────────────────────────── LOOP ──────────────────────────────
void loop() {
  unsigned long maintenant = millis();

  // -------- MODE AUTO --------
  if (modeAuto) {
    gererAuto(maintenant);
  }

  // -------- AVANCE + RADAR AVANT --------
  if (!modeAuto && (avance || radarAvant)) {
    if (maintenant - derniereSonarAvant >= INTERVALLE_SONAR) {
      mesurerDistanceAvant();
      derniereSonarAvant = maintenant;
    }
    if (avance && distanceFront <= 60) {
      stopRobot();
      avance = false; radarAvant = true; bipAvantEnCours = false;
      Serial.println("Obstacle devant - STOP radar actif");
    }
    radarBipAvant(distanceFront, maintenant);
    if (radarAvant && !avance && distanceFront > 65) {
      radarAvant = false; noTone(klaxonPin);
      Serial.println("Voie libre avant - radar off");
    }
  }

  // -------- RECUL + RADAR ARRIERE --------
  if (!modeAuto && (recule || radarArriere)) {
    if (maintenant - derniereSonarArriere >= INTERVALLE_SONAR) {
      mesurerDistanceArriere();
      derniereSonarArriere = maintenant;
    }
    if (recule && distanceBack <= 60) {
      stopRobot();
      recule = false; radarArriere = true; bipEnCours = false;
      Serial.println("Obstacle derriere - STOP radar actif");
    }
    radarBipArriere(distanceBack, maintenant);
    if (radarArriere && !recule && distanceBack > 65) {
      radarArriere = false; noTone(klaxonPin);
      Serial.println("Voie libre arriere - radar off");
    }
  }

  // -------- PARSING COMMANDE BLUETOOTH --------
    while (Serial.available()) {
    char c = Serial.read();
    commandeBuffer += c;
    dernierCaractere = maintenant;
}
  while (Serial1.available()) {
    char c = Serial1.read();
    commandeBuffer += c;
    dernierCaractere = maintenant;
  }

  if (commandeBuffer.length() > 0 && (maintenant - dernierCaractere >= TIMEOUT_CMD)) {
    int cmd = commandeBuffer.toInt();
    Serial.print("CMD : "); Serial.println(cmd);
    commandeBuffer = "";
    traiterCommande(cmd, maintenant);
  }
}

// ─────────────────────── MODE AUTO ───────────────────────────────
void gererAuto(unsigned long maintenant) {

  if (maintenant - derniereSonarAvant >= INTERVALLE_SONAR) {
    mesurerDistanceAvant();
    derniereSonarAvant = maintenant;
  }

  switch (etatAuto) {

    case AVANCE_AUTO:
      if (distanceFront <= 60) {
        // Mur détecté : stop puis virage boosté
        stopRobot();
        Serial.println("Mur detecte -> virage");

        // Boost vitesse pour le virage
        motorAD.setSpeed(VITESSE_VIRAGE);
        motorAG.setSpeed(VITESSE_VIRAGE);
        motorDG.setSpeed(VITESSE_VIRAGE);
        motorDD.setSpeed(VITESSE_VIRAGE);

        gauche();
        debutVirage = maintenant;
        etatAuto    = TOURNE_AUTO;
      }
      break;

    case TOURNE_AUTO:
      if (maintenant - debutVirage >= DUREE_TOURNE) {
        // Virage terminé : restaure vitesse normale et repart
        appliquerVitesse();
        avancer();
        etatAuto = AVANCE_AUTO;
        Serial.println("Virage fini -> avance");
      }
      break;

    case IDLE:
    default:
      break;
  }
}

void activerAuto(unsigned long maintenant) {
  modeAuto  = true;
  etatAuto  = AVANCE_AUTO;
  avance = false; recule = false; radarAvant = false; radarArriere = false;
  bipAvantEnCours = false; bipEnCours = false;
  noTone(klaxonPin);
  appliquerVitesse();
  avancer();
  Serial.println("Mode auto ON");
}

void stopAuto() {
  modeAuto = false;
  etatAuto = IDLE;
  appliquerVitesse();
  stopRobot();
  noTone(klaxonPin);
  Serial.println("Mode auto OFF");
}

// ─────────────────────── TRAITEMENT COMMANDES ────────────────────
void traiterCommande(int cmd, unsigned long maintenant) {

  if (cmd == 3 && modeAuto) { stopAuto(); return; }
  if (modeAuto) return;

  switch (cmd) {

    case 1:
      mesurerDistanceAvant(); derniereSonarAvant = maintenant;
      if (distanceFront > 50) {
        avancer(); avance = true; radarAvant = true;
        recule = false; radarArriere = false; bipEnCours = false; noTone(klaxonPin);
      } else Serial.println("Obstacle devant - avance impossible");
      break;

    case 2:
      mesurerDistanceArriere(); derniereSonarArriere = maintenant;
      if (distanceBack > 50) {
        reculer(); recule = true; radarArriere = true;
        avance = false; radarAvant = false; bipAvantEnCours = false; noTone(klaxonPin);
      } else Serial.println("Obstacle derriere - recul impossible");
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
      Serial.print("Vitesse : "); Serial.println(vitesse); break;

    case 7:
      vitesse = max(vitesse - 20, 0); appliquerVitesse();
      Serial.print("Vitesse : "); Serial.println(vitesse); break;

    case 8:
      tone(klaxonPin, 900); delay(100); noTone(klaxonPin); break;

    case 10:
      activerAuto(maintenant); break;
  }
}

// ─────────────────────── RADAR AVANT ─────────────────────────────
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

// ─────────────────────── RADAR ARRIERE ───────────────────────────
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

// ─────────────────────── SONAR ───────────────────────────────────
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

// ─────────────────────── MOTEURS ─────────────────────────────────
void appliquerVitesse() {
  motorAD.setSpeed(vitesse); motorAG.setSpeed(vitesse);
  motorDG.setSpeed(vitesse); motorDD.setSpeed(vitesse);
}
void avancer()  { motorAD.run(FORWARD);  motorAG.run(FORWARD);  motorDG.run(FORWARD);  motorDD.run(FORWARD);  }
void reculer()  { motorAD.run(BACKWARD); motorAG.run(BACKWARD); motorDG.run(BACKWARD); motorDD.run(BACKWARD); }
void gauche()   { motorAD.run(FORWARD);  motorAG.run(BACKWARD); motorDG.run(BACKWARD); motorDD.run(FORWARD);  }
void droite()   { motorAD.run(BACKWARD); motorAG.run(FORWARD);  motorDG.run(FORWARD);  motorDD.run(BACKWARD); }
void stopRobot(){ motorAD.run(RELEASE);  motorAG.run(RELEASE);  motorDG.run(RELEASE);  motorDD.run(RELEASE);  }
