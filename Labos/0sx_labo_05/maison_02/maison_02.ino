#include <LiquidCrystal_I2C.h>
#include <OneButton.h>
#include <DHT.h>
#include <HCSR04.h>
#include <AccelStepper.h>
// Definition du moteur et des broches
#define MOTOR_INTERFACE_TYPE 4

#define IN_1 31
#define IN_2 33
#define IN_3 35
#define IN_4 37

#define LED_PIN 9
#define PHOTO_PIN A0
#define BTN_PIN 4
#define DHTPIN 7
#define DHTTYPE DHT11
#define TRIGGER_PIN 12
#define ECHO_PIN 11
#define LCD_ADDR 0x27

AccelStepper moteur(MOTOR_INTERFACE_TYPE, IN_1, IN_3, IN_2, IN_4);
// Traduction
// State = État
// current = actuel
// rate = taux
enum IrrigationState { FERME, OUVERTURE, OUVERT, FERMETURE, ARRET };
IrrigationState currentIrrState = OUVERT;
enum AppState { DEMARRAGE, LUM_DIST, DHT_STATE, CALIBRATION, ETAT_VANNE };
AppState currentState = DEMARRAGE;

// Définir les variables globales
unsigned long currentTime;
int luminosite = 0;
float temperature = 0.0;
float humidite = 0.0;
float distance = 0.0;
int val = 0;
int maximum;
int minimum;
long positionMoteur = 2038; // au départ ouvert
const int POS_FERME = 0;
const int POS_OUVERT = 2038;

LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
OneButton button(BTN_PIN);
DHT dht(DHTPIN, DHTTYPE);
HCSR04 hc(TRIGGER_PIN, ECHO_PIN);

void setup() {
  Serial.begin(115200);

  initPins();
  lcdInit();
  buttonInit();
  initPhoto();
  initMoteur();
  dht.begin();
}

// --------------------------------------------------
// Initialisation du materiel
// --------------------------------------------------

void initPins() {
  pinMode(LED_PIN, OUTPUT);
}

void lcdInit() {
  lcd.init();  
  lcd.backlight();
}

void buttonInit() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  button.attachClick(switchState);
  button.attachDoubleClick(doubleClic);
}

void initPhoto() {
  maximum = 0;
  minimum = 1023;
}

void initMoteur() {
  moteur.setMaxSpeed(200);
  moteur.setAcceleration(100);
  moteur.setCurrentPosition(2038);
}

// --------------------------------------------------
// Main Loop
// --------------------------------------------------

void loop() {
  currentTime = millis();
  static unsigned long lastTime = 0;
  const int interval = 3000;
  int vanPourcentage = map(moteur.currentPosition(), POS_FERME, POS_OUVERT, 0, 100);
  
  button.tick();
  gestionnaireEtatLCD(currentTime);
  gestionnaireEtatIrr(currentTime);
  // Attach double clic to doubleclic()
  // Attach clic to switchState()
  moteur.run();
  lireValeur(currentTime);


  if (currentTime - lastTime >= interval) {
    lastTime = currentTime;
    Serial.print("Lum:");
    Serial.print(luminosite);
    Serial.print(",Min:");
    Serial.print(minimum);
    Serial.print(",Max:");
    Serial.print(maximum);
    Serial.print(",Dist:");
    Serial.print(distance);
    Serial.print(",T:");
    Serial.print(temperature);
    Serial.print(",H:");
    Serial.print(humidite);
    Serial.print(",Van:");
    Serial.print(vanPourcentage);

    Serial.println();
  }
}

// --------------------------------------------------
// Gestionnaires des etats
// --------------------------------------------------

void gestionnaireEtatLCD(unsigned long ct) {
  switch (currentState) {
    case DEMARRAGE:
      bootState(currentTime);
      break;
    case LUM_DIST:
      lumDistState(currentTime);
      break;
    case DHT_STATE:
      dhtState(currentTime);
      break;
    case CALIBRATION:
      calibrationState(currentTime);
      break;
    case ETAT_VANNE:
      etatVanneState(currentTime);
      break;
  }
}

void gestionnaireEtatIrr(unsigned long ct) {
  switch (currentIrrState) {
    case FERME:
      fermeState();
      break;
    case OUVERTURE:
      ouvertureState(ct);  
      break;
    case OUVERT:
      ouvertState(); 
      break;
    case FERMETURE:
      fermetureState(ct);
      break;
    case ARRET:
      arretState();
      break;
  }
}

void switchState() {
  if (currentIrrState == FERMETURE) {
    currentIrrState = ARRET;
    return;
  }

  if (currentIrrState == ARRET) {
    currentIrrState = OUVERTURE;
    return;
  }
  
  if (currentState != DHT_STATE) {
    currentState = DHT_STATE;
  } else {
    currentState = LUM_DIST;
  }
}

void doubleClic() {
  currentState = CALIBRATION;
}

void lireValeur(unsigned long ct) {
  static unsigned long lastTimeDHT = 0;
  static unsigned long lastTimeDist = 0;
  const int rate = 5000;
  const int shortRate = 250;
  static bool ledState = false;

  if (ct - lastTimeDHT > rate) {
    lastTimeDHT = ct;
    lireDHT(ct);
  }
  if (ct - lastTimeDist > shortRate) {
    lastTimeDist = ct;
    lireDistance(ct);
  }

  lireLuminosite(ct);
}

void lireDHT(unsigned long ct) {
  const int intervale = 5000;
  static unsigned long lastTime = 0;

  if (ct - lastTime > intervale) {
    lastTime = ct;
    temperature = dht.readTemperature();
    humidite = dht.readHumidity();
  }

  if (isnan(humidite) || isnan(temperature)) {
    Serial.println(F("Echec de la lecture du DHT!"));
    return;
  }
}

void lireDistance(unsigned long ct) {
  const int intervale = 250;
  static unsigned long lastTime = 0;


  if (ct - lastTime > intervale) {
    lastTime = ct;
    distance = hc.dist();
  }

  if (isnan(distance)) {
    Serial.println(F("Echec de la lecture du HC-RS04!"));
    return;
  }
}

void lireLuminosite(unsigned long ct) {
  const int intervale = 500;
  static unsigned long lastTime = 0;
  const int petit = 0;
  const int grand = 100;
  const int allume = 30;
  const int eteint = 35;
  static bool ledState = false;

  if (maximum <= minimum) {
    ledState = false;  
  }

  if (ct - lastTime > intervale) {
    lastTime = ct;
    
    luminosite = analogRead(PHOTO_PIN);
    luminosite = map(luminosite, minimum, maximum, petit, grand);
    luminosite = constrain(luminosite, petit, grand);



    if (luminosite < allume) {
      ledState = true;
    } else if (luminosite > eteint) {
      ledState = false;
    }
  }

  if (currentState != ETAT_VANNE) {
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  }
}

void bootState(unsigned long ct) {
  static unsigned long startTime = 0;
  static bool first = true;
  const int exitTime = 3000;
  const int lcdRate = 250;

  if (startTime == 0) {
    startTime = ct;
  }

  if (first) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Etd 1: 2390972");
    first = false;
  }

  if (ct - startTime > exitTime) {
    currentState = DHT_STATE;
    return;
  }
}

void lumDistState(unsigned long ct) {
  static unsigned long lastTime = 0;
  const int lcdRate = 250;

  if (ct - lastTime > lcdRate) {
    lastTime = ct;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lum : ");
    lcd.print(luminosite);
    lcd.print("% ");
    lcd.setCursor(0, 1);
    lcd.print("Dist : ");
    lcd.print(distance);
    lcd.print(" cm ");
  }
}

void dhtState(unsigned long ct) {
  static unsigned long lastTime = 0;
  const int lcdRate = 250;

  if (ct - lastTime > lcdRate) {
    lastTime = ct;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp : ");
    lcd.print(temperature);
    lcd.print(" C ");
    lcd.setCursor(0, 1);
    lcd.print("Hum : ");
    lcd.print(humidite);
    lcd.print(" % ");
  }
}

void calibrationState(unsigned long ct) {
  static unsigned long lastTimeInt = 0;
  static unsigned long lastTimeLcd = 0;
  const int intervale = 50;
  const int lcdRate = 250;
  if (ct - lastTimeInt > intervale) {
    lastTimeInt = ct;
    val = analogRead(PHOTO_PIN);
  }

  if (val < minimum) {
    minimum = val;
  }
  if (val > maximum) {
    maximum = val;
  }

  if (currentTime - lastTimeLcd > lcdRate) {
    lastTimeLcd = ct;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lum min : ");
    lcd.print(minimum);
    lcd.setCursor(0, 1);
    lcd.print("Lum max : ");
    lcd.print(maximum);
  }
}

void etatVanneState(unsigned long ct) {
  static unsigned long lastTime = 0;
  const int lcdRate = 250;

  if (ct - lastTime > lcdRate) {
    lastTime = ct;
    String etat;
    int pourcentage = map(moteur.currentPosition(), POS_FERME, POS_OUVERT, 0, 100);
    
    switch (currentIrrState) {
      case FERME: 
        etat = " Ferme";
        break;
      case OUVERTURE:
        etat = " Ouverture";
        break;
      case OUVERT:
        etat = " Ouvert";
        break;
      case FERMETURE:
        etat = " Fermeture";
        break;
      case ARRET:
        etat = " Arret";
        break;  
    }

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Vanne: ");
    lcd.print(pourcentage);
    lcd.print(" %");

    lcd.setCursor(0,1);
    lcd.print("Etat:");
    lcd.print(etat);
  }
}

void clignoterLED(unsigned long ct) {
  static unsigned long lastTime = 0;
  static bool state = false;
  const int intervale = 100;


  if (ct - lastTime >= intervale) {
    lastTime = ct;
    state = !state;
    digitalWrite(LED_PIN, state);
  }
}

void ouvertState() {
  const int dist = 25;
  if (distance >= dist) {
    currentIrrState = FERMETURE;
  } 
}

void fermeState() {
  const int dist = 20;
  if (distance < dist) {
    currentIrrState = OUVERTURE;
  }
}

void ouvertureState(unsigned long ct) {
  const int stop = 0;
  moteur.moveTo(POS_OUVERT);
  moteur.run();


  currentState = ETAT_VANNE;
  clignoterLED(ct);

  if (moteur.distanceToGo() == stop) {
    currentIrrState = OUVERT;
    digitalWrite(LED_PIN, LOW);
  }
}

void fermetureState(unsigned long ct) {
  const int stop = 0;
  moteur.moveTo(POS_FERME);
  moteur.run();

  currentState = ETAT_VANNE;
  clignoterLED(ct);

  if (moteur.distanceToGo() == stop) {
    currentIrrState = FERME;
    digitalWrite(LED_PIN, LOW);
  }
}

void arretState() {
  moteur.stop();
  digitalWrite(LED_PIN, LOW);

  currentState = ETAT_VANNE;
}
