// This file contains the code for the ps/2 protocol
#include "com.h"
#include "pins.h"
#include <Arduino.h>

volatile int bitCount = 0;
volatile byte curByte = 0;
volatile bool curByteEven = false;
volatile bool confused = false;
volatile bool transmit = false;

volatile byte sendBuffer[8];
volatile int sendBufferReadPtr = 0;
volatile int sendBufferWritePtr = 0;

void (*onByte)(byte);
void (*onError)(const char *);

void com_attachOnByte(void (*f)(byte)) { onByte = f; }
void com_attachOnError(void (*f)(const char *)) { onError = f; }

void reset() {
  curByte = 0;
  bitCount = 0;
  curByteEven = false;
}

void err(const char *msg) {
  if (onError) {
    onError(msg);
  }
}

void onClock() {
  if (transmit) {
    if (bitCount == 9) {
      pinMode(PIN_DATA, INPUT_PULLUP);
    } else if (bitCount == 10) {
      if (digitalRead(PIN_DATA)) {
        err("ack bit wrong");
      }
      reset();
      transmit = false;
      return;
    } else if (bitCount <= 8) {
      bool bit = false;
      if (bitCount == 8) {
        bit = !curByteEven;
      } else {
        if ((curByte >> bitCount) & 0x01) {
          curByteEven = !curByteEven;
          bit = true;
        }
      }
      digitalWrite(PIN_DATA, bit);
    }
    bitCount++;
    return;
  }

  bool bit = digitalRead(PIN_DATA);

  if (confused && bit == 0) {
    reset();
    confused = false;
  }

  // start bit
  if (bitCount == 0 && bit != 0) {
    err("Startbit failed");
    confused = true;
    return;
  }

  // stop bit
  if (bitCount == 10 && bit != 1) {
    err("Stopbit failed");
    confused = true;
    return;
  }

  // stop parity
  if (bitCount == 9 && bit != !curByteEven) {
    err("Parity failed");
    confused = true;
    return;
  }

  if (bitCount != 0 && bitCount != 9 && bitCount != 10) {
    if (bit) {
      curByte |= 0x01 << (bitCount - 1);
      curByteEven = !curByteEven;
    }
  }

  bitCount++;
  if (bitCount == 11) {
    if (onByte) {
      onByte(curByte);
    }
    reset();
  }
}

void com_inhibit() {
  detachInterrupt(digitalPinToInterrupt(PIN_CLOCK));
  pinMode(PIN_CLOCK, OUTPUT);
  digitalWrite(PIN_CLOCK, LOW);
  reset();
  delayMicroseconds(100);
}

void com_release() {
  pinMode(PIN_CLOCK, INPUT_PULLUP);
  delayMicroseconds(10);
  EIFR |= 0x2;
  attachInterrupt(digitalPinToInterrupt(PIN_CLOCK), onClock, FALLING);
}

void com_sendByte(byte b) {
  sendBuffer[sendBufferWritePtr] = b;
  sendBufferWritePtr++;
  if (sendBufferWritePtr == sizeof(sendBuffer)) {
    sendBufferWritePtr = 0;
  }
}
bool com_sendDataPending() { return sendBufferWritePtr != sendBufferReadPtr; }

void com_tick() {
  if (!transmit && com_sendDataPending()) {
    delay(10);
    byte b = sendBuffer[sendBufferReadPtr];
    sendBufferReadPtr++;
    if (sendBufferReadPtr == sizeof(sendBuffer)) {
      sendBufferReadPtr = 0;
    }

    com_inhibit();

    pinMode(PIN_DATA, OUTPUT);
    digitalWrite(PIN_DATA, LOW);
    delayMicroseconds(150);

    transmit = true;
    curByte = b;

    com_release();
  }
}

void com_setup() {
  pinMode(PIN_CLOCK, INPUT_PULLUP);
  pinMode(PIN_DATA, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_CLOCK), onClock, FALLING);
}
