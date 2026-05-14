/* FICHIER: test_ultrason.ino
*  AUTEUR: DEVAUD Antoine
*  RÔLE: Permet de tester un capteur ultrason en renseignant les pins (trig, echo)
**/

///////////////////////////////////////////////
// Programme test pour capteur HC-SR05 //
// Go Tronic 2017 //
///////////////////////////////////////////////
#define trigPin 25  
#define echoPin 23

long duration, distance;

void setup() {
 Serial.begin (9600);
 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);
 Serial.println("== Debut du programme ==");
}

void loop() {
 // Envoie de l'onde
 digitalWrite(trigPin, LOW);
 delayMicroseconds(2);
 digitalWrite(trigPin, HIGH);
 delayMicroseconds(10);
 digitalWrite(trigPin, LOW);
 // Réception de l'écho
 duration = pulseIn(echoPin, HIGH);

 // Calcul de la distance
 distance = (duration/2) / 29.1;
 if (distance >= 400 || distance <= 0){
 Serial.println("Hors plage");
 }
 else {
 Serial.print("distance = ");
 Serial.print(distance);
 Serial.println(" cm");
 }
 delay(500); // délai entre deux mesures
}
