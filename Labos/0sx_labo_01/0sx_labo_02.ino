// Labo 2 - Gabrielle Bourassa

//Variables
unsigned long anciennesMillis = 0;
int niveau = 0;
// Lumieres
const int ledPins[] = { 8, 9, 10, 11 };  // Tableau des numeros de broches

// Bouton
const int bouton = 2;

// Setup
void setup() {
  for (int i = 0; i < 4; i++) {
    // Initialisation des DEL en sortie
    pinMode(ledPins[i], OUTPUT);
  }

  pinMode(bouton, INPUT_PULLUP);

  Serial.begin(9600);
}

void verifierEtat() {
  // Verifie le pentiometre a chaque 20 ms et la convertit en une echelle de 0 a 20.
  const int intervale = 20;
  unsigned long maintenant = millis();


  if (maintenant - anciennesMillis >= intervale) {
    niveau = analogRead(A1);
    niveau = map(niveau, 0, 1023, 0, 20);  // A ajuster si ne marche pas
    anciennesMillis = maintenant;
  }

  // Lorsque le bouton est appuyé, le programme envoie via le port série la valeur de l'échelle (barre de progression)
  if (digitalRead(bouton) == LOW) {
    // Barre de progression : 25 % [>>>>>>............]  avant-dernier chiffre de num etudiant impair
    Serial.print(niveau * 5);
    Serial.print("% ");
    Serial.print("[");
    for (int i = 0; i < niveau; i++) {
      Serial.print(">");
    }
    for (int i = 0; i < 20 - niveau; i++) {
      Serial.print(".");
    }
    Serial.println("]");
  }

  // Dernier num etudiant pair, donc une del allumee a la fois
  if (niveau == 0) {
    digitalWrite(ledPins[3], LOW);
    digitalWrite(ledPins[2], LOW);
    digitalWrite(ledPins[1], LOW);
    digitalWrite(ledPins[0], LOW);
  } else if (niveau >= 1 && niveau <= 5) {
    digitalWrite(ledPins[3], HIGH);
    digitalWrite(ledPins[2], LOW);
    digitalWrite(ledPins[1], LOW);
    digitalWrite(ledPins[0], LOW);
  } else if (niveau >= 6 && niveau <= 10) {
    digitalWrite(ledPins[3], LOW);
    digitalWrite(ledPins[2], HIGH);
    digitalWrite(ledPins[1], LOW);
    digitalWrite(ledPins[0], LOW);
  } else if (niveau >= 11 && niveau <= 15) {
    digitalWrite(ledPins[3], LOW);
    digitalWrite(ledPins[2], LOW);
    digitalWrite(ledPins[1], HIGH);
    digitalWrite(ledPins[0], LOW);
  } else if (niveau >= 16 && niveau <= 20) {
    digitalWrite(ledPins[3], LOW);
    digitalWrite(ledPins[2], LOW);
    digitalWrite(ledPins[1], LOW);
    digitalWrite(ledPins[0], HIGH);
  }
}


void loop() {
  // put your main code here, to run repeatedly:
  verifierEtat();
}
