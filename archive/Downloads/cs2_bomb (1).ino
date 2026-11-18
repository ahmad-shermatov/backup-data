// CS2 bomb prop: Arduino + I2C 1602 LCD + 4x4 keypad + buzzer (low-level trigger)
// Libraries: "Keypad" (Mark Stanley) and "LiquidCrystal I2C" (Frank de Brabander)

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// ---------- Settings ----------
const char ARM_CODE[]    = "7355608";   // code to plant the bomb (as in CS2)
const char DEFUSE_CODE[] = "7355608";   // code to defuse
const unsigned long BOMB_TIME = 40000UL; // 40 seconds, as in CS2

const byte BUZZER_PIN = 10;
const byte BUZZ_ON  = LOW;   // low-level trigger: LOW = sound
const byte BUZZ_OFF = HIGH;

LiquidCrystal_I2C lcd(0x27, 16, 2);      // if screen is empty, try 0x3F

// ---------- Keypad ----------
const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------- State ----------
enum State { IDLE, ARMED, DEFUSED, EXPLODED };
State state = IDLE;

char input[17];
byte inputLen = 0;

unsigned long armedAt = 0;
unsigned long lastBeep = 0;
unsigned long beepStart = 0;
unsigned long lastLcd = 0;
bool beeping = false;

// ---------- Helpers ----------
void buzz(bool on) {
  digitalWrite(BUZZER_PIN, on ? BUZZ_ON : BUZZ_OFF);
}

void beep(unsigned int ms) {
  buzz(true);
  delay(ms);
  buzz(false);
}

void clearInput() {
  inputLen = 0;
  input[0] = '\0';
}

void addDigit(char key) {
  if (inputLen < 16) {
    input[inputLen++] = key;
    input[inputLen] = '\0';
  }
}

void printRow(byte row, const char *text) {
  lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(0, row);
  lcd.print(text);
}

void showIdle() {
  printRow(0, "Enter code:");
  printRow(1, input);
}

// ---------- Actions ----------
void armBomb() {
  state = ARMED;
  armedAt = millis();
  lastBeep = 0;
  clearInput();
  lcd.clear();
  printRow(0, "BOMB PLANTED");
  beep(300);
  delay(700);
  lcd.clear();
}

void defuseBomb() {
  state = DEFUSED;
  buzz(false);
  lcd.clear();
  printRow(0, "BOMB DEFUSED");
  printRow(1, "CT WIN  (#=new)");
  beep(80); delay(80); beep(80);
}

void explode() {
  state = EXPLODED;
  lcd.clear();
  printRow(0, "*** BOOM ***");
  printRow(1, "T WIN   (#=new)");
  // long sound + blinking backlight
  unsigned long start = millis();
  buzz(true);
  while (millis() - start < 2500) {
    lcd.noBacklight(); delay(150);
    lcd.backlight();   delay(150);
  }
  buzz(false);
}

void resetBomb() {
  state = IDLE;
  clearInput();
  buzz(false);
  lcd.clear();
  showIdle();
}

// ---------- State handlers ----------
void handleIdle(char key) {
  if (!key) return;

  if (key >= '0' && key <= '9') {
    addDigit(key);
    beep(20);
    showIdle();
  } else if (key == '*') {        // clear
    clearInput();
    showIdle();
  }

  if (inputLen == strlen(ARM_CODE)) {
    if (strcmp(input, ARM_CODE) == 0) {
      armBomb();
    } else {
      printRow(1, "WRONG CODE");
      beep(400);
      delay(600);
      clearInput();
      showIdle();
    }
  }
}

void handleArmed(char key) {
  unsigned long now = millis();
  unsigned long elapsed = now - armedAt;

  if (elapsed >= BOMB_TIME) {
    explode();
    return;
  }
  unsigned long remaining = BOMB_TIME - elapsed;

  // Beeps get faster as time runs out (1000 ms -> 100 ms)
  unsigned long interval = 100 + (remaining * 900UL) / BOMB_TIME;
  if (!beeping && now - lastBeep >= interval) {
    buzz(true);
    beeping = true;
    beepStart = now;
    lastBeep = now;
  }
  if (beeping && now - beepStart >= 40) {
    buzz(false);
    beeping = false;
  }

  // Defuse input
  if (key >= '0' && key <= '9') {
    addDigit(key);
  } else if (key == '*') {
    clearInput();
  }
  if (inputLen == strlen(DEFUSE_CODE)) {
    if (strcmp(input, DEFUSE_CODE) == 0) {
      defuseBomb();
      return;
    }
    clearInput();                  // wrong code: just clear
  }

  // Update screen every 100 ms
  if (now - lastLcd >= 100) {
    lastLcd = now;
    char line[17];
    snprintf(line, sizeof(line), "Time: %2lu.%lu s",
             remaining / 1000, (remaining % 1000) / 100);
    printRow(0, line);
    snprintf(line, sizeof(line), "Def: %s", input);
    printRow(1, line);
  }
}

// ---------- Main ----------
void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  buzz(false);
  lcd.init();
  lcd.backlight();
  resetBomb();
}

void loop() {
  char key = keypad.getKey();

  switch (state) {
    case IDLE:  handleIdle(key);  break;
    case ARMED: handleArmed(key); break;
    default:
      if (key == '#') resetBomb();
      break;
  }
}
