#include <LiquidCrystal_I2C.h>
#include <OneButton.h>
#include <DHT.h>
#include <HCSR04.h>

#define LED_PIN 9
#define PHOTO_PIN A0
#define BTN_PIN 4
#define DHTPIN 7
#define DHTTYPE DHT11
#define TRIGGER_PIN 12
#define ECHO_PIN 11
#define LCD_ADDR 0x27

// Traduction
// State = État
// current = actuel
// rate = taux

enum AppState { DEMARRAGE,
                LUM_DIST,
                DHT_STATE,
                CALIBRATION };
// Lum dist = page 1, DHT_state = page 2
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

LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
OneButton button(BTN_PIN);
DHT dht(DHTPIN, DHTTYPE);
HCSR04 hc(TRIGGER_PIN, ECHO_PIN);

void setup() {
  Serial.begin(9600);

  initPins();
  lcdInit();
  buttonInit();
  initPhoto();
  dht.begin();
}

void initPins() {
  pinMode(LED_PIN, OUTPUT);
}

void lcdInit() {
  lcd.init();  // initialize the lcd
  lcd.backlight();
}

void buttonInit() {
  // Faire le code pour l'initialisation du bouton
  pinMode(BTN_PIN, INPUT_PULLUP);
  button.attachClick(switchState);
  button.attachDoubleClick(doubleClic);
}

void initPhoto() {
  maximum = 0;
  minimum = 1023;
}

void loop() {
  currentTime = millis();
  static unsigned long lastTime = 0;
  const int interval = 3000;


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
  }
  // Attach double clic to doubleclic()
  // Attach clic to switchState()

  button.tick();
  lireValeur(currentTime);

  if (currentTime - lastTime >= interval) {
    lastTime = currentTime;
    Serial.print("Lum: ");
    Serial.print(luminosite);
    Serial.print(", Min: ");
    Serial.print(minimum);
    Serial.print(", Max: ");
    Serial.print(maximum);
    Serial.print(", Dist: ");
    Serial.print(distance);
    Serial.print(", T: ");
    Serial.print(temperature);
    Serial.print(", H: ");
    Serial.print(humidite);

    Serial.println();
  }
}

void switchState() {
  if (currentState != DHT_STATE) {
    currentState = DHT_STATE;
  } else {
    currentState = LUM_DIST;
  }
}

void doubleClic() {
  currentState = CALIBRATION;
}

// Représente une tâche quelconque qui doit être exéctué de façon continue
void lireValeur(unsigned long ct) {
  static unsigned long lastTimeDHT = 0;
  static unsigned long lastTimeDist = 0;
  const int rate = 5000;
  const int shortRate = 250;
  static bool ledState = false;

  if (ct - lastTimeDHT > rate) {
    lastTimeDHT = ct;
    lireDHT(currentTime);
  }
  if (ct - lastTimeDist > shortRate) {
    lastTimeDist = ct;
    lireDistance(currentTime);
  }

  ledState = lireLuminosite(currentTime);

  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
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

bool lireLuminosite(unsigned long ct) {
  const int intervale = 500;
  static unsigned long lastTime = 0;
  const int petit = 0;
  const int grand = 100;
  static bool ledState = false;

  if (maximum <= minimum) {
    return false;  
  }

  if (ct - lastTime > intervale) {
    lastTime = ct;
    
    luminosite = analogRead(PHOTO_PIN);
    luminosite = map(luminosite, minimum, maximum, petit, grand);
    luminosite = constrain(luminosite, 0, 100);

    if (luminosite < 30) {
      ledState = true;
    } else if (luminosite > 35) {
      ledState = false;
    }
  }

  return ledState;
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
