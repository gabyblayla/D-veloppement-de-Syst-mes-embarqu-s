// Labo 03 - Gabrielle Bourassa
// Declaration de librairies
#include <LCD_I2C.h> // Bibliotheque LCD I2C de blackhack
#include <OneButton.h> // Bibliothere OneButton de Matthias Hertel

// Ecran LCD
// Declaration des variables
byte customChar[] = {
  B11110,
  B00010,
  B00100,
  B01111,
  B10001,
  B00111,
  B00100,
  B00111
};

LCD_I2C lcd(0x27, 16, 2);
const int sda = 20, scl = 21; 
const int numEd = 2390972;
// LED
const int led = 8;
bool lightOn;
// Thermistance
const int thermistance = A0;

// Seuils temp
const int tempHaute = 25;
const int tempBasse = 24;

// Joystick
const int axeY = A1;
const int axeX = A2;
const int bouton = 2;
OneButton boutonJoystick(bouton, true);
const int minSpeed = -10;
const int maxSpeed = 10;
const int minimumDep = -100; // Minimum deplacement, -100
const int maximumDep = 100; // Maximum deplacement, 100
int posX = 0;
int posY = 0;
int depX = 0;
int depY = 0;
int page = 0;

// Temps
const int seconde = 1000;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(thermistance, INPUT);
  boutonJoystick.attachClick(changerPage);
  
  // LCD depart
  lcd.begin();
  lcd.backlight();
  lcd.createChar(0, customChar);
  lcd.setCursor(0, 0); 
  lcd.print("Bourassa");
  lcd.setCursor(0, 1); 
  lcd.write(0);        // affiche le caractère custom #0
  lcd.print(" 72");

  // On attend 3 secondes, puis on clear le LCD pour continuer dans la loop d'un ecran vide.
  delay(3000);
  lcd.clear();
}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned long anciennesMillis;
  unsigned long maintenant = millis();

  boutonJoystick.tick();

  if (maintenant - anciennesMillis >= 500) {
    int temp = capteur();
    clim(temp);
    joystick();
    affichageLCD(temp);
    anciennesMillis = maintenant;
  }
}


int capteur() {
  int Vo;
  float R1 = 10000;
  float logR2, R2, T, Tc, Tf;
  // Coefficients de A, B, et C
  float c1 = 1.129148e-03, c2 = 2.34125e-04, c3 = 8.76741e-08;
  
  Vo = analogRead(thermistance);
  if (Vo == 0) {
    Vo = 1;
  }
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  Tc = T - 273.15;

  return Tc;
}

void clim(int temp) {
  // Lumiere allume si a temperature est plus haute que 25 degres. Reste eteinte si 24 degres et moins.  
  if (temp >= tempHaute) {
    digitalWrite(led, HIGH);
    lightOn = true;
  } else if (temp <= tempBasse) {
    digitalWrite(led, LOW);
    lightOn = false;
  }
}

void joystick() {
  // Le joystick retourne les axes x et y pour determiner le deplacement de l'ecran LCD.
  static unsigned long anciennesMillis;
  unsigned long maintenant = millis();
  depX = map(analogRead(axeX), 0, 1023, minSpeed, maxSpeed);
  depY = map(analogRead(axeY), 0, 1023, minSpeed, maxSpeed);
  // J'ai decouvert du joystick drift mais uniquement sur l'axe des x.
  if (abs(depX) <= 1) depX = 0;
  if (abs(depY) <= 1) depY = 0;

  if (maintenant - anciennesMillis >= seconde) {
    if (depX < 0 && posX + depX > minimumDep) {
      posX += depX;
    } else if (depX < 0 && posX + depX <= minimumDep) {
      posX = minimumDep;
      depX = 0;
    } else if (depX > 0 && posX + depX < maximumDep) {
      posX += depX;
    } else if (depX > 0 && posX + depX >= maximumDep) {
      posX = maximumDep;
      depX = 0;
    }
    
    if (depY < 0 && posY + depY > minimumDep) {
      posY += depY;
    } else if (depY < 0 && posY + depY <= minimumDep) {
      posY = minimumDep;
      depY = 0;
    } else if (depY > 0 && posY + depY < maximumDep) {
      posY += depY;      
    } else if (depY > 0 && posY + depY >= maximumDep) {
      posY = maximumDep;
      depY = 0;
    }

    anciennesMillis = maintenant;
  }
}

void changerPage() {
  lcd.clear();
  page++;
  if (page > 1) {
    page = 0;
  }
}

void affichageLCD(int temp) {
  // Appeler joystick pour verifier l'etat de l'axe x et y.
  // Verifier le bouton du joystick directement a l'interieur de la fonction LCD. S'assurer que ca ne spam pas - si on clique le bouton, ca compte pour 1 clic, pas 10.
  String etatClim = "";

  if (lightOn == true) {
    lcd.clear();
    etatClim = "ON";
  } else {
    lcd.clear();
    etatClim = "OFF";
  }

  if (page == 0) {
    lcd.setCursor(0,0);
    lcd.print("Temp:");
    lcd.print(temp);

    lcd.setCursor(0,1);
    lcd.print("AC: ");
    lcd.print(etatClim);
  } 
  if (page == 1) {
    lcd.setCursor(0,0);
    lcd.print("X:");
    lcd.print(posX);
    lcd.print(" ");
    lcd.print(" V: ");
    lcd.print(depX);
    lcd.print(" ");

    lcd.setCursor(0,1);
    lcd.print("Y:");
    lcd.print(posY);
    lcd.print(" ");
    lcd.print(" V: ");
    lcd.print(depY);
    lcd.print(" ");
  }
}