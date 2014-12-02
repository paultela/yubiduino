#include <sha1.h>

#define INTERVAL 30
#define TOKEN_LENGTH 6


#include <DS3231.h>


DS3231 rtc(SDA, SCL);

void printHash(uint8_t* hash, size_t len) {
  int i;
  for (i=0; i<len; i++) {
    Serial.print("0123456789abcdef"[hash[i]>>4]);
    Serial.print("0123456789abcdef"[hash[i]&0xf]);
  }
  Serial.println();
}

// Gets a UNIX timestamp from the RTC
long getTimestamp() {
  return rtc.getUnixTime(rtc.getTime());
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

void totp(const uint8_t* key, const size_t len, const long time, char token[TOKEN_LENGTH]) {
  Serial.println("Starting totp");
  long tc = time / INTERVAL;
  Sha1.initHmac(key, len);
  Serial.println("inited Hmac");
  Sha1.write((uint8_t) (tc >> 24));
  Sha1.write((uint8_t) (tc >> 16));
  Sha1.write((uint8_t) (tc >> 8));
  Sha1.write((uint8_t) tc);
 


  
  Serial.println("wrote");
  uint8_t *hash = Sha1.resultHmac();
  
  Serial.println("hashed");
  
  uint8_t offset = hash[19] & 0xf;
  
  unsigned long thing = ((hash[offset] & 0x7f) << 24)
                      | ((hash[offset + 1] & 0xff) << 16)
                      | ((hash[offset + 2] & 0xff) << 8)
                      | ((hash[offset + 3] & 0xff));
  Serial.println("thinged");
  Serial.println(thing);
  
  for (int i = TOKEN_LENGTH - 1; i >= 0; i--) {
    token[i] = (thing % 10) + '0';
    thing /= 10;
  }
  
  Serial.println("\ndone");
} 

void setup() {
  // put your setup code here, to run once
  Serial.begin(115200);
  rtc.begin();
  
  uint8_t keyBase32[32];
  String("salvdaxi3ipetshmxf54muq7q4k3cnrj").toCharArray((char *)keyBase32, 32);
  
  uint8_t key[20];
  base32_decode(keyBase32, key, 20);
  
  uint8_t correctKey[20] = { 0x90, 0x17, 0x51, 0x82, 0xe8, 0xda, 0x1e, 0x49, 0xc8, 0xec, 0xb9, 0x7b, 0xc6, 0x52, 0x1f, 0x87, 0x15, 0xb1, 0x36, 0x29 };
  
  printHash(correctKey, 20);
  
  
  char token[TOKEN_LENGTH];
  
  Serial.println("Trying to do work");
  
  totp(key, 20, getTimestamp(), token);
  
  Serial.println(token);
  Serial.println("All done with work.");

}

void loop() {
  // put your main code here, to run repeatedly:
  
}
