/**
 * Yubiduino, TOTP on an Arduino
 *
 * Jessica Fleck, Brandon Mills, and Paul Tela
 *
 * Load base 32 encoded secret key onto the SD card in a file called key.txt.
 * When the button is pressed, a one time password is generated and typed.
 *
 * A "shell" is available which allows for setting and retrieval of the time
 * on the RTC.  It also allows for retrieval of the secret key.
 *
 * Port layout:
 * DS3231:
 *  SDA pin -> Arduino Digital 20 (SDA) or the dedicated SDA1 (Digital 70) pin
 *  SCL pin -> Arduino Digital 21 (SCL) or the dedicated SCL1 (Digital 71) pin
 *
 * Button pin: 42
 *
 * Serial baud: 115200
 *
 * Uses the DS3231 Library for Arduino, Copyright (C)2014 Henning Karlsen.
 * Used under the terms of the CC BY-NC-SA 3.0 license.
 *
 * Also makes use of portions of the Cryptosuite Library for Arduino.
 */

#include <DS3231.h>
#include <SPI.h>
#include <SD.h>
#include <sha1.h>

#define BUTTON_PIN 42
#define SD_PIN 4
#define KEY_FILE "key.txt"
#define INTERVAL 30
#define TOKEN_LENGTH 6

DS3231 rtc(SDA, SCL);
uint8_t key[20];
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

// Prints out a hash as a hex string
void printHash(uint8_t* hash, size_t len) {
  int i;
  for (i=0; i<len; i++) {
    Serial.print("0123456789abcdef"[hash[i]>>4]);
    Serial.print("0123456789abcdef"[hash[i]&0xf]);
  }
  Serial.println();
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
    printHash(key, 20);
    Serial.println();
  }
}

// From https://code.google.com/p/google-authenticator/source/browse/libpam/base32.c
int base32_decode(const uint8_t *encoded, uint8_t *result, int bufSize) {
  int buffer = 0;
  int bitsLeft = 0;
  int count = 0;
  for (const uint8_t *ptr = encoded; count < bufSize && *ptr; ++ptr) {
    uint8_t ch = *ptr;
    if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '-') {
      continue;
    }
    buffer <<= 5;

    // Deal with commonly mistyped characters
    if (ch == '0') {
      ch = 'O';
    } else if (ch == '1') {
      ch = 'L';
    } else if (ch == '8') {
      ch = 'B';
    }

    // Look up one base32 digit
    if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
      ch = (ch & 0x1F) - 1;
    } else if (ch >= '2' && ch <= '7') {
      ch -= '2' - 26;
    } else {
      return -1;
    }

    buffer |= ch;
    bitsLeft += 5;
    if (bitsLeft >= 8) {
      result[count++] = buffer >> (bitsLeft - 8);
      bitsLeft -= 8;
    }
  }
  if (count < bufSize) {
    result[count] = '\000';
  }
  return count;
}

// Generates a TOKEN_LENGTH long token given a secret and timestamp.
void totp(uint8_t* secret, const size_t len, const long time, char token[TOKEN_LENGTH]) {
  long timestamp = time / INTERVAL;
  uint8_t input[8] = { 0x00, 0x00, 0x00, 0x00 };
  input[4] = (timestamp >> 24) & 0xff;
  input[5] = (timestamp >> 16) & 0xff;
  input[6] = (timestamp >> 8) & 0xff;
  input[7] = timestamp & 0xff;

  Sha1.initHmac(secret, len);
  Sha1.write(input, 8);
  uint8_t* hash = Sha1.resultHmac();

  uint8_t offset = hash[19] & 0xf;

  unsigned long thing = ((hash[offset] & 0x7f) << 24)
                      | ((hash[offset + 1] & 0xff) << 16)
                      | ((hash[offset + 2] & 0xff) << 8)
                      | ((hash[offset + 3] & 0xff));

  for (int i = TOKEN_LENGTH - 1; i >= 0; i--) {
    token[i] = (thing % 10) + '0';
    thing /= 10;
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
  String keyString = "";
  while (keyFile.available()) {
    char ch = keyFile.read();
    if (ch == ' ' || ch == '\n') continue;
    keyString.concat(ch);
  }
  keyFile.close();
  uint8_t keyText[33];
  keyString.toCharArray((char*)keyText, 33);
  base32_decode(keyText, key, 20);

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
    char token[TOKEN_LENGTH];
    totp(key, 20, getTimestamp(), token);
    Keyboard.print(token);
  } else {
    delay(100);
  }

  previousButtonState = buttonState;

}

