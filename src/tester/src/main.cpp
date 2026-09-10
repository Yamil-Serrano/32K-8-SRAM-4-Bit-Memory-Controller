#include <Arduino.h>

// Color codes for serial output
#define COLOR_RED    "\033[31m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_RESET  "\033[0m"

// Address pins
#define A0 22
#define A1 23
#define A2 24
#define A3 25
#define A4 26
#define A5 27
#define A6 28
#define A7 29

// Write data pins
#define IN0 30
#define IN1 31
#define IN2 32
#define IN3 33

// Read data pins
#define OUT0 34
#define OUT1 35
#define OUT2 36
#define OUT3 37

// Control pins
#define PIN_WS 38
#define PIN_RS 39
#define PIN_CLK 40
#define PIN_SW 41

const int DELAY_US = 1;
const int RAM_SIZE = 256;

int addrPins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int inPins[4]   = {IN0, IN1, IN2, IN3};
int outPins[4]  = {OUT0, OUT1, OUT2, OUT3};

// Set the 8 bit address bus
void setAddress(byte address) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(addrPins[i], (address >> i) & 1);
  }
}

// Put a 4 bit value on the write pins
void setNibble(byte value) {
  value &= 0x0F;
  for (int i = 0; i < 4; i++) {
    digitalWrite(inPins[i], (value >> i) & 1);
  }
}

// Read a 4 bit value from the read pins
byte getNibble() {
  byte value = 0;
  for (int i = 0; i < 4; i++) {
    value |= digitalRead(outPins[i]) << i;
  }
  return value;
}

// Write one nibble (low or high) to a given address
void writeNibble(byte address, byte value, bool highNibble) {
  setAddress(address);
  digitalWrite(PIN_WS, highNibble ? LOW : HIGH);
  setNibble(value);
  delayMicroseconds(DELAY_US);
  digitalWrite(PIN_SW, HIGH);
  delayMicroseconds(DELAY_US);
  digitalWrite(PIN_CLK, HIGH);
  delayMicroseconds(DELAY_US);
  digitalWrite(PIN_CLK, LOW);
  delayMicroseconds(DELAY_US);
  digitalWrite(PIN_SW, LOW);
  delayMicroseconds(DELAY_US);
}

// Read one nibble (low or high) from a given address
byte readNibble(byte address, bool highNibble) {
  setAddress(address);
  digitalWrite(PIN_RS, highNibble ? HIGH : LOW);
  delayMicroseconds(DELAY_US);
  digitalWrite(PIN_SW, LOW);
  delayMicroseconds(DELAY_US);
  digitalWrite(PIN_CLK, HIGH);
  delayMicroseconds(DELAY_US);
  byte value = getNibble();
  digitalWrite(PIN_CLK, LOW);
  delayMicroseconds(DELAY_US);
  return value;
}

// Print a nibble as binary and hex
void printNibble(byte value) {
  for (int i = 3; i >= 0; i--) {
    Serial.print((value >> i) & 1);
  }
  Serial.print(" (0x");
  if (value < 0x10) Serial.print("0");
  Serial.print(value, HEX);
  Serial.print(")");
}

// Print one row of the results table
void printRow(int address, byte low, byte high, byte data, bool ok) {
  Serial.print("│ 0x");
  if (address < 0x10) Serial.print("0");
  Serial.print(address, HEX);
  Serial.print("  │  ");
  printNibble(low);
  Serial.print("  │  ");
  printNibble(high);
  Serial.print("  │  0x");
  if (data < 0x10) Serial.print("0");
  Serial.print(data, HEX);
  Serial.print("  │  ");
  if (ok) {
    Serial.print(COLOR_GREEN);
    Serial.print("PASS");
    Serial.print(COLOR_RESET);
  } else {
    Serial.print(COLOR_RED);
    Serial.print("FAIL");
    Serial.print(COLOR_RESET);
  }
  Serial.println("  │");
}

void printTableTop() {
  Serial.println("┌───────┬───────────────┬───────────────┬────────┬────────┐");
}

void printTableMid() {
  Serial.println("├───────┼───────────────┼───────────────┼────────┼────────┤");
}

void printTableBottom() {
  Serial.println("└───────┴───────────────┴───────────────┴────────┴────────┘");
}

void printTableHeader() {
  Serial.println();
  printTableTop();
  Serial.println("│ ADDR  │      LOW      │      HIGH     │  BYTE  │ STATUS │");
  printTableMid();
}

// Write the same value to every address, then read it back and check it
void fillRAM(byte value) {
  for (int address = 0; address < RAM_SIZE; address++) {
    writeNibble(address, value & 0x0F, false);
    writeNibble(address, value & 0x0F, true);
  }

  printTableHeader();

  int errors = 0;
  for (int address = 0; address < RAM_SIZE; address++) {
    byte low = readNibble(address, false);
    byte high = readNibble(address, true);
    byte data = (high << 4) | low;
    bool ok = (data == value);
    if (!ok) errors++;
    printRow(address, low, high, data, ok);
  }
  printTableBottom();

  Serial.print("Done. Errors: ");
  Serial.println(errors);
}

void testRAM() {
  fillRAM(0xFF);
}

void clearRAM() {
  fillRAM(0x00);
}

void printMenu() {
  Serial.println();
  Serial.println("┌──────────────────────────────────────────────────────┐");
  Serial.println("│              CHIBI-4 RAM MODULE TESTER               │");
  Serial.println("├──────────────────────────────────────────────────────┤");
  Serial.println("│  1. Test RAM  (fill with 0xFF and verify)            │");
  Serial.println("│  2. Clear RAM (fill with 0x00 and verify)            │");
  Serial.println("└──────────────────────────────────────────────────────┘");
  Serial.print("Enter option: ");
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 8; i++) pinMode(addrPins[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(inPins[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(outPins[i], INPUT);

  pinMode(PIN_WS, OUTPUT);
  pinMode(PIN_RS, OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_SW, OUTPUT);

  digitalWrite(PIN_WS, LOW);
  digitalWrite(PIN_RS, LOW);
  digitalWrite(PIN_CLK, LOW);
  digitalWrite(PIN_SW, LOW);

  printMenu();
}

void loop() {
  if (Serial.available() > 0) {
    char option = Serial.read();
    while (Serial.available() > 0) Serial.read();

    if (option == '1') {
      Serial.println(option);
      testRAM();
    } else if (option == '2') {
      Serial.println(option);
      clearRAM();
    } else {
      Serial.println("Invalid option. Please enter 1 or 2.");
    }

    printMenu();
  }
}
