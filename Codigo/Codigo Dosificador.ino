#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// Definición del LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definición del Keypad
const byte ROWS = 4; // Cuatro filas
const byte COLS = 4; // Cuatro columnas
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {12, 14, 27, 26}; // Pines de las filas
byte colPins[COLS] = {25, 33, 32, 35}; // Pines de las columnas
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Definición del motor paso a paso
#define STEP_PIN 16
#define DIR_PIN 17
#define ENABLE_PIN 5
#define LIMIT_SWITCH_PIN 18

// Parámetros del motor y flujo
const float stepsPerMilliliter = 1.0; // Ajustar según el cálculo
volatile bool limitSwitchActivated = false;

void setup() {
  // Inicialización del LCD
  lcd.init();
  lcd.backlight();
  lcd.print("Ingrese mL:");

  // Inicialización del Keypad
  Serial.begin(9600);

  // Inicialización del motor paso a paso
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW);

  // Inicialización del final de carrera
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(LIMIT_SWITCH_PIN), limitSwitchISR, FALLING);
}

void loop() {
  if (limitSwitchActivated) {
    resetMotor();
    limitSwitchActivated = false;
    lcd.clear();
    lcd.print("Motor reiniciado");
    delay(2000); // Esperar 2 segundos para mostrar el mensaje
    lcd.clear();
    lcd.print("Ingrese mL:");
  }

  static String input = "";
  char key = keypad.getKey();
  
  if (key) {
    if (key == '#') {
      // Procesar la entrada cuando se presiona #
      int ml = input.toInt();
      lcd.clear();
      lcd.print("Dispensando ");
      lcd.print(ml);
      lcd.print(" mL");
      dispense(ml);
      input = "";
      lcd.clear();
      lcd.print("Ingrese mL:");
    } else if (key == '*') {
      // Borrar la entrada cuando se presiona *
      input = "";
      lcd.clear();
      lcd.print("Ingrese mL:");
    } else {
      // Agregar el dígito a la entrada
      input += key;
      lcd.setCursor(0, 1);
      lcd.print(input);
    }
  }
}

void dispense(int milliliters) {
  int steps = milliliters * stepsPerMilliliter;
  digitalWrite(DIR_PIN, HIGH); // Ajustar la dirección según sea necesario
  for (int i = 0; i < steps; i++) {
    if (limitSwitchActivated) break; // Detener si se activa el final de carrera
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(500); // Ajustar según sea necesario para la velocidad
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(500);
  }
}

void resetMotor() {
  digitalWrite(DIR_PIN, LOW); // Ajustar la dirección de retroceso
  while (digitalRead(LIMIT_SWITCH_PIN) == HIGH) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(500); // Ajustar según sea necesario para la velocidad
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(500);
  }
  digitalWrite(ENABLE_PIN, LOW); // Desactivar el motor si es necesario
}

void limitSwitchISR() {
  limitSwitchActivated = true;
}
