#include <DS3231.h>

DS3231  rtc(SDA, SCL);

int get(const char* label) {
  Serial.println(label);
  while (!Serial.available());
  return Serial.parseInt();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  rtc.begin();
  
  int year = get("Year");
  int month = get("Month");
  int day = get("Day");
  rtc.setDate(day, month, year);
  
  int hour = get("Hour");
  int minute = get("Minute");
  int second = get("Second");
  rtc.setTime(hour, minute, second);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  Serial.println(rtc.getDateStr());
  Serial.println(rtc.getTimeStr());
  
  delay(1000);

}
