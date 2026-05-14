c#include <AFMotor_R4.h>

#define trigPin      30
#define echoPin      28
#define trigPinBack  31
#define echoPinBack  29
#define klaxonPin    50

#define INTERVALLE_SONAR 80

char commande;

long distanceFront = 100;
long distanceBack  = 100;

bool avance       = false;
bool recule       = false;
bool radarAvant   = false;  // miroir de radarArriere
bool radarArriere = false;

int vitesse = 200;

// Timing sonar
unsigned long derniereSonarAvant   = 0;
unsigned long derniereSonarArriere = 0;

// Radar avant
unsigned long dernierBipAvant  = 0;
bool bipAvantEnCours            = false;
unsigned long debutBipAvant     = 0;

// Radar arrière
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

  // -------- AVANCE + RADAR AVANT --------
  if (avance || radarAvant) {
    if (maintenant - derniereSonarAvant >= INTERVALLE_SONAR) {
      mesurerDistanceAvant();
      derniereSonarAvant = maintenant;
    }

    if (avance && distanceFront <= 50) {
      stopRobot();
      avance = false;
      radarAvant = true;   // reste actif comme radarArriere
      bipAvantEnCours = false;
      Serial.println("Obstacle devant - STOP radar actif");
    }

    radarBipAvant(distanceFront, maintenant);

    // obstacle disparu -> radar off
    if (radarAvant && !avance && distanceFront > 75) {
      radarAvant = false;
      noTone(klaxonPin);
      Serial.println("Voie libre avant - radar off");
    }
  }

  // -------- RECUL + RADAR ARRIERE --------
  if (recule || radarArriere) {
    if (maintenant - derniereSonarArriere >= INTERVALLE_SONAR) {
      mesurerDistanceArriere();
      derniereSonarArriere = maintenant;
    }

    if (recule && distanceBack <= 50) {
      stopRobot();
      recule = false;
      radarArriere = true;
      bipEnCours = false;
      Serial.println("Obstacle derriere - STOP radar actif");
    }

    radarBipArriere(distanceBack, maintenant);

    if (radarArriere && !recule && distanceBack > 75) {
      radarArriere = false;
      noTone(klaxonPin);
      Serial.println("Voie libre arriere - radar off");
    }
  }

  // -------- COMMANDES BLUETOOTH --------
  if (Serial1.available() || Serial.available()) {
   if (Serial.available()) commande = Serial.read();
    else if (Serial1.available()) commande = Serial1.read();
    
    Serial.print("Commande recue : "); Serial.println(commande);

    switch (commande) {

      case '1':
        mesurerDistanceAvant();
        derniereSonarAvant = maintenant;
        if (distanceFront > 50) {
          avancer();
          avance = true; radarAvant = true;
          recule = false; radarArriere = false;
          bipEnCours = false;
          noTone(klaxonPin);
        } else {
          Serial.println("Obstacle devant - avance impossible");
        }
        break;

      case '2':
        mesurerDistanceArriere();
        derniereSonarArriere = maintenant;
        if (distanceBack > 50) {
          reculer();
          recule = true; radarArriere = true;
          avance = false; radarAvant = false;
          bipAvantEnCours = false;
          noTone(klaxonPin);
        } else {
          Serial.println("Obstacle derriere - recul impossible");
        }
        break;

      case '3':
        stopRobot();
        avance = false; recule = false;
        radarAvant = false; radarArriere = false;
        bipAvantEnCours = false; bipEnCours = false;
        noTone(klaxonPin);
        break;

      case '4':
        gauche();
        avance = false; recule = false;
        radarAvant = false; radarArriere = false;
        bipAvantEnCours = false; bipEnCours = false;
        noTone(klaxonPin);
        break;

      case '5':
        droite();
        avance = false; recule = false;
        radarAvant = false; radarArriere = false;
        bipAvantEnCours = false; bipEnCours = false;
        noTone(klaxonPin);
        break;

      case '6':
        vitesse = min(vitesse + 20, 255);
        appliquerVitesse();
        Serial.print("Vitesse : "); Serial.println(vitesse);
        break;

      case '7':
        vitesse = max(vitesse - 20, 0);
        appliquerVitesse();
        Serial.print("Vitesse : "); Serial.println(vitesse);
        break;

      case '8':
        tone(klaxonPin, 900);
        delay(100);
        noTone(klaxonPin);
        break;
    }
  }
}

// ─────────────────────── RADAR AVANT ─────────────────────────────
void radarBipAvant(long distance, unsigned long maintenant) {
  if (distance > 75) {
    noTone(klaxonPin); bipAvantEnCours = false; return;
  }
  if (distance <= 50) {
    tone(klaxonPin, 1000); bipAvantEnCours = false; return;
  }
  // 50 < distance <= 75
  const unsigned long dureeBip   = 80;
  const unsigned long intervalle = 600;

  if (bipAvantEnCours) {
    if (maintenant - debutBipAvant >= dureeBip) {
      noTone(klaxonPin); bipAvantEnCours = false; dernierBipAvant = maintenant;
    }
  } else {
    if (maintenant - dernierBipAvant >= intervalle) {
      tone(klaxonPin, 1000); bipAvantEnCours = true; debutBipAvant = maintenant;
    }
  }
}

// ─────────────────────── RADAR ARRIERE ───────────────────────────
void radarBipArriere(long distance, unsigned long maintenant) {
  if (distance > 75) {
    noTone(klaxonPin); bipEnCours = false; return;
  }
  if (distance <= 50) {
    tone(klaxonPin, 700); bipEnCours = false; return;
  }
  // 50 < distance <= 75
  const unsigned long dureeBip   = 80;
  const unsigned long intervalle = 600;

  if (bipEnCours) {
    if (maintenant - debutBip >= dureeBip) {
      noTone(klaxonPin); bipEnCours = false; dernierBip = maintenant;
    }
  } else {
    if (maintenant - dernierBip >= intervalle) {
      tone(klaxonPin, 700); bipEnCours = true; debutBip = maintenant;
    }
  }
}

// ───────────────────── SONAR ─────────────────────────────────────
long mesurerDistance(int trigP, int echoP) {
  digitalWrite(trigP, LOW);
  delayMicroseconds(2);
  digitalWrite(trigP, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigP, LOW);
  long dur = pulseIn(echoP, HIGH, 25000);
  if (dur == 0) return 999;
  return (dur / 2) / 29.1;
}

void mesurerDistanceAvant() {
  distanceFront = mesurerDistance(trigPin, echoPin);
  Serial.print("Avant : "); Serial.println(distanceFront);
}

void mesurerDistanceArriere() {
  distanceBack = mesurerDistance(trigPinBack, echoPinBack);
  Serial.print("Arriere : "); Serial.println(distanceBack);
}

// ─────────────────────── MOTEURS ─────────────────────────────────
void appliquerVitesse() {
  motorAD.setSpeed(vitesse); motorAG.setSpeed(vitesse);
  motorDG.setSpeed(vitesse); motorDD.setSpeed(vitesse);
}
void avancer()  { motorAD.run(FORWARD);   motorAG.run(FORWARD);   motorDG.run(FORWARD);   motorDD.run(FORWARD);   }
void reculer()  { motorAD.run(BACKWARD);  motorAG.run(BACKWARD);  motorDG.run(BACKWARD);  motorDD.run(BACKWARD);  }
void gauche()   { motorAD.run(FORWARD);   motorAG.run(BACKWARD);  motorDG.run(BACKWARD);  motorDD.run(FORWARD);   }
void droite()   { motorAD.run(BACKWARD);  motorAG.run(FORWARD);   motorDG.run(FORWARD);   motorDD.run(BACKWARD);  }
void stopRobot(){ motorAD.run(RELEASE);   motorAG.run(RELEASE);   motorDG.run(RELEASE);   motorDD.run(RELEASE);   }
