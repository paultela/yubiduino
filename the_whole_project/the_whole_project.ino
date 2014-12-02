// Arduino Due:
// ----------------------
// DS3231:  SDA pin   -> Arduino Digital 20 (SDA) or the dedicated SDA1 (Digital 70) pin
//          SCL pin   -> Arduino Digital 21 (SCL) or the dedicated SCL1 (Digital 71) pin

/*
 * Keyboard Password Printing
 *
 * When button is pressed, will type "this is your password".
 *
 * The circuit:
 * - button attached from pin 42 to +3.3V
 * - 10-kilohm resistor attached from pin 42 to ground
 *
 *
 * Based off of the example from:
 * http://www.arduino.cc/en/Tutorial/KeyboardMessage
 */

#include <DS3231.h>
#include <SPI.h>
#include <SD.h>

#define BUTTON_PIN 42
#define SD_PIN 4
#define KEY_FILE "key.txt"

DS3231 rtc(SDA, SCL);
String key = "";
int previousButtonState = HIGH;

// Gets a UNIX timestamp from the RTC
long getTimestamp() {
  return rtc.getUnixTime(rtc.getTime());
}

// Reads an integer from the Serial monitor, optionally with a prompt
int readInt(const char* prompt) {
  if (prompt) {
    Serial.println(prompt);
    while (!Serial.available());
  }

  return Serial.parseInt();
}

// Prompts to set the time on the RTC
void setTime() {
  int year = readInt("Year");
  int month = readInt("Month");
  int day = readInt("Day");
  rtc.setDate(day, month, year);

  int hour = readInt("Hour");
  int minute = readInt("Minute");
  int second = readInt("Second");
  rtc.setTime(hour, minute, second);
}

// This implements a minimal REPL
void command() {
  String command = "";
  while (Serial.available()) {
    char ch = Serial.read();
    command.concat(ch);
  }

  if (command == "time") {
    Serial.print(rtc.getDateStr());
    Serial.println(rtc.getTimeStr());
    Serial.println(rtc.getUnixTime(rtc.getTime()));
  } else if (command == "set time") {
    setTime();
  } else if (command == "key") {
    Serial.println(key);
  }
}

void setup() {
  // Set up the serial terminal
  Serial.begin(115200);

  // Set up the button
  pinMode(BUTTON_PIN, INPUT);

  // Set up the real time clock
  rtc.begin();

  // Set up the SD card
  pinMode(10, OUTPUT);
  if (!SD.begin(SD_PIN)) {
    Serial.println("SD card is not present.");
    return;
  }
  File keyFile = SD.open(KEY_FILE);
  if (!keyFile) {
    Serial.println("SD card has no key.txt");
    return;
  }
  while (keyFile.available()) {
    char ch = keyFile.read();
    if (ch == ' ' || ch == '\n') continue;
    key.concat(ch);
  }
  keyFile.close();

  // Set up the keyboard
  Keyboard.begin();

  // Good to go!
  Serial.println("Running...");
}

void loop() {

  int buttonState = digitalRead(BUTTON_PIN);

  if (Serial.available()) {
    command();
  } else if (buttonState == HIGH && previousButtonState == LOW) {
    Keyboard.print(key);
  } else {
    delay(100);
  }

  previousButtonState = buttonState;

}

