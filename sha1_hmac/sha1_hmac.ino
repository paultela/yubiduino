
/**
HMAC-SHA-1 Testing

Make sure that the HMAC-SHA-1 Library works correctly.
*/

#include <sha1.h>

void setup() {
  Serial.begin(9600); 
  // put your setup code here, to run once:
  uint8_t *hash;
  uint8_t key[] = {0x0b, 0x0b};
  Sha1.initHmac(key,2); // key, and length of key in bytes
  Sha1.print("Hi There");
  hash = Sha1.resultHmac();
  for(int i = 0; i < 20; i++) {
     Serial.print(hash[i]); 
  }
  Serial.println("done");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Mom?");
  delay(2000);
}
