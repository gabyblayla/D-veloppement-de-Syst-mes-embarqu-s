//Labo 1 - Gabrielle Bourassa

//Variables
const int ledPin = LED_BUILTIN;
int ledState = LOW;

unsigned long anciennesMillis = 0;

// Faire clignoter la DEL 1 a 5 fois selon l'avant dernier chiffre du numero etudiant (7 , donc 4).
//Utiliser une enum d'etats pour nommer l'etat (clignottement, variation, allumEteint)
enum EtatAppli { CLIGNOTEMENT,
                 VARIATION,
                 ALLUMEETEINT };

EtatAppli etatActuel = CLIGNOTEMENT;
void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void Clignoter() {
  // Il faudra clignoter 4 fois. Chaque clignottement est de 500 ms. (250 ms off 250ms on)
  // Faire un loop qui fonctionnera 4 fois (avec un compteur). A la fin de la loop, break le cycle et changer l'etat de l'enum.
  unsigned long maintenant = millis();
  const int intervaleClignotement = 250;
  static int compteur = 0;
  const int nbClignotements = 4;

  if (maintenant - anciennesMillis >= intervaleClignotement) {
    anciennesMillis = maintenant;

    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
      compteur++;
    }
    digitalWrite(ledPin, ledState);

    if (compteur == nbClignotements) {
      compteur = 0;
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      Serial.println("Variation - 2390972");
      anciennesMillis = millis();
      etatActuel = VARIATION;
    }
  }
}

void Varier() {
  // Variation d'intensite de la lumiere.
  // Le dernier chiffre le mon numero d'etudiant est pair. Donc, la variation sera de 0 a 100%.
  // La variation doit se faire graduellement pendant 2048ms.
  static int intensite = 0;
  const int dureeTotale = 2048;
  const int gradation = 5;
  unsigned long maintenant = millis();
  const int lumMax = 255;

  if (maintenant - anciennesMillis >= (dureeTotale / (lumMax / gradation))) {
    anciennesMillis = maintenant;

    analogWrite(ledPin, intensite);
    intensite += gradation;

    if (intensite > lumMax) {
      intensite = 0;
      digitalWrite(ledPin, intensite);
      Serial.println("Allume - 2390972");
      anciennesMillis = millis();
      etatActuel = ALLUMEETEINT;
    }
  }
}

void AllumerEteindre() {
  const int intervaleCourte = 250;
  const int intervaleLongue = 1000;
  static int compteur = 0;
  unsigned long maintenant = millis();

  if (compteur == 0 && maintenant - anciennesMillis >= intervaleCourte) {
    anciennesMillis = maintenant;
    ledState = HIGH;
    digitalWrite(ledPin, ledState);
    compteur++;
  } else if (compteur >= 1 && maintenant - anciennesMillis >= intervaleLongue) {
    anciennesMillis = maintenant;
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    digitalWrite(ledPin, ledState);
    compteur++;
  }

  if (compteur >= 3) {
    compteur = 0;
    Serial.println("Clignottement - 2390972");
    ledState = LOW;
    digitalWrite(ledPin, ledState); 
    etatActuel = CLIGNOTEMENT;
  }
}

void loop() {
  // Utilisation d'un switch case au lieu de ifs avec des numeros:
  switch (etatActuel) {
    case CLIGNOTEMENT:
      Clignoter();
      break;
    case VARIATION:
      Varier();
      break;
    case ALLUMEETEINT:
      AllumerEteindre();
      break;
  }
}
