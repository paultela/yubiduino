// Arduino Due:
// ----------------------
// DS3231:  SDA pin   -> Arduino Digital 20 (SDA) or the dedicated SDA1 (Digital 70) pin
//          SCL pin   -> Arduino Digital 21 (SCL) or the dedicated SCL1 (Digital 71) pin

#include <DS3231.h>

DS3231  rtc(SDA, SCL);

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
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  rtc.begin();
  
  Serial.println("Running...");
}

void loop() {
  
  if (Serial.available()) {
    command();
  } else {
    delay(100);
  }

}
