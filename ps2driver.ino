#include "com.h"
#include "keys.h"
#include "scancodes2ascii.h"

#define CMD_RESET 0xFF
#define CMD_ECHO 0xEE
#define CMD_LEDS 0xED

// was the previous byte the start of a break code
volatile bool breakCode = false;

char keyToAscii(byte key) {
  if (key >= 128) {
    return '\0';
  }
  return ScanCodeSet2ToAsciiTable[key];
}

void onKey(byte key, bool press) {
  if (press) {
    if (key == KEY_F1) {
      com_sendByte(CMD_RESET);
    } else if (key == KEY_F2) {
      com_sendByte(CMD_LEDS);
      com_sendByte(0b00000000);
    } else if (key == KEY_F3) {
      com_sendByte(CMD_LEDS);
      com_sendByte(0b00000111);
    }

    char kchar = keyToAscii(key);
    if (kchar) {
      Serial.print(kchar);
    } else {
      Serial.println(key);
    }
  }
}

void onTransmitted() {}

void onByte(byte b) {
  if (b == 0xF0) {
    breakCode = true;
    return;
  }
  if (b == 0xEE) {
    Serial.println("Got echo");
    return;
  }

  if (b == 0xAA) {
    Serial.println("Keyboard self test (BAT) was successful");
    return;
  }
  if (b == 0xFC) {
    Serial.println("Keyboard self test (BAT) failed");
    return;
  }
  if (b == 0xFE) {
    Serial.println("Got resend command from keyboard");
  }

  onKey(b, !breakCode);
  breakCode = false;
}

void onError(const char *msg) { Serial.println(msg); }

void setup() {
  Serial.begin(9600);
  Serial.println("\nStart");

  com_setup();
  com_attachOnByte(onByte);
  com_attachOnError(onError);
}

void loop() { com_tick(); }
