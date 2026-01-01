#include <Arduino.h>
#ifndef COM_H
#define COM_H

void com_setup();
void com_attachOnByte(void (*f)(byte));
void com_attachOnError(void (*f)(const char *));

void com_tick();
bool com_sendDataPending();
void com_sendByte(byte b);

void com_inhibit();
void com_release();

#endif
