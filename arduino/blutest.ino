void setup() {

  Serial.begin(9600);    // PC
  Serial1.begin(9600);   // HM10 (pins 18 et 19)

  Serial.println("Test HM10 pret");
}

void loop() {

  if (Serial1.available()) {

    char c = Serial1.read();   // lire caractère bluetooth
    Serial.print("Recu : ");
    Serial.println(c);         // afficher sur PC

    Serial1.print("Arduino a recu : ");
    Serial1.println(c);        // renvoyer au telephone
  }

}
