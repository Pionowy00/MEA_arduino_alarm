#include <Keypad.h>

// Stany systemu
#define STATE_UNARMED  1
#define STATE_ARMING   2
#define STATE_ARMED    3
#define STATE_UNARMING 4
#define STATE_ALARM    5

// Piny (zgodnie z Twoim schematem)
#define PIN_ALARM_SOUND  4   // Buzzer
#define PIN_ALARM_LIGHT  2   // Jedyna dioda LED (sygnalizacyjna)
#define PIN_PIR_SENSOR   3   // Czujnik ruchu PIR

// Czasy (w milisekundach)
const unsigned long ARMING_TIME = 5000;
const unsigned long UNARMING_TIME = 5000;
const unsigned long BLINK_TIME = 500;

// Klawiatura membranowa 4x4
const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {12, 11, 10, 9};
byte colPins[COLS] = {8, 7, 6, 5};

Keypad customKeypad = Keypad(
  makeKeymap(hexaKeys),
  rowPins,
  colPins,
  ROWS,
  COLS
);

// Aktualny stan
int currentState = STATE_UNARMED;

// Timery
unsigned long stateStartTime = 0;
unsigned long lastBlinkTime = 0;

// Status migania diody
bool alarmLightState = false;

// Wprowadzane has³o
String enteredCode = "";
const String PASSWORD = "1234";

// Funkcja zmiany stanu
void changeState(int newState)
{
  currentState = newState;
  stateStartTime = millis();

  // Reset wyjœæ przy zmianie stanu
  digitalWrite(PIN_ALARM_SOUND, LOW);
  digitalWrite(PIN_ALARM_LIGHT, LOW);

  alarmLightState = false;
}

// Obs³uga klawiatury
void handleKeypad()
{
  char key = customKeypad.getKey();

  if (key)
  {
    if (key == '#')
    {
      Serial.println("#");
      Serial.println(enteredCode);
      if (enteredCode == PASSWORD)
      {
        if (currentState == STATE_UNARMED)
        {
          changeState(STATE_ARMING);
        }
        else if (currentState == STATE_ARMED ||
                 currentState == STATE_ALARM)
        {
          changeState(STATE_UNARMING);
        }
      }

      enteredCode = "";
    }
    else if (key >= '0' && key <= '9')
    {
      Serial.println("number");
      if (enteredCode.length() < 4)
      {
        enteredCode += key;
      }
    }
    else
    {
      enteredCode = "";
    }
  }
}

// Konfiguracja pocz¹tkowa
void setup()
{
  Serial.begin(9600);
  pinMode(PIN_ALARM_SOUND, OUTPUT);
  pinMode(PIN_ALARM_LIGHT, OUTPUT);
  pinMode(PIN_PIR_SENSOR, INPUT);

  changeState(STATE_UNARMED);
}

// G³ówna pêtla programu
void loop()
{
  handleKeypad();

  switch (currentState)
  {
    // -------------------------
    // ROZBROJONY
    // -------------------------
    case STATE_UNARMED:
      digitalWrite(PIN_ALARM_LIGHT, LOW);
      break;

    // -------------------------
    // UZBRAJANIE (odliczanie)
    // -------------------------
    case STATE_ARMING:
      // Mruganie diod¹ podczas uzbrajania
      if (millis() - lastBlinkTime >= BLINK_TIME)
      {
        lastBlinkTime = millis();
        alarmLightState = !alarmLightState;
        digitalWrite(PIN_ALARM_LIGHT, alarmLightState);
      }

      if (millis() - stateStartTime >= ARMING_TIME)
      {
        changeState(STATE_ARMED);
      }
      break;

    // -------------------------
    // UZBROJONY (czuwanie)
    // -------------------------
    case STATE_ARMED:
      // Dioda œwieci ci¹g³ym œwiat³em informuj¹c o uzbrojeniu
      digitalWrite(PIN_ALARM_LIGHT, HIGH);

      // Sprawdzenie czujnika ruchu PIR
      if (digitalRead(PIN_PIR_SENSOR) == HIGH)
      {
        changeState(STATE_ALARM);
      }
      break;

    // -------------------------
    // ROZBRAJANIE (odliczanie)
    // -------------------------
    case STATE_UNARMING:
      // Mruganie diod¹ podczas rozbrajania
      if (millis() - lastBlinkTime >= BLINK_TIME)
      {
        lastBlinkTime = millis();
        alarmLightState = !alarmLightState;
        digitalWrite(PIN_ALARM_LIGHT, alarmLightState);
      }

      if (millis() - stateStartTime >= UNARMING_TIME)
      {
        changeState(STATE_UNARMED);
      }
      break;

    // -------------------------
    // ALARM (wykryto ruch)
    // -------------------------
    case STATE_ALARM:
      // Aktywacja buzzera
      digitalWrite(PIN_ALARM_SOUND, HIGH);

      // Szybkie mruganie diod¹ alarmow¹
      if (millis() - lastBlinkTime >= BLINK_TIME)
      {
        lastBlinkTime = millis();
        alarmLightState = !alarmLightState;
        digitalWrite(PIN_ALARM_LIGHT, alarmLightState);
      }
      break;
  }
}
