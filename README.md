# Communicate using the ps2 protocol on an Arduino

Test program to comunicate with ps/2 devices using an Arduino Uno.
com.cpp and com.h contain the device agnostic communication code.

## Building (no arduino ide) (linux)

If you are on Linux you can just install avrdude and run `make upload` to upload the code to the Arduino and connect the right pins (see pins.h).

## Building

If you have the arduino ide you should be able to just upload the project to the Arduino and connect the right pins (see pins.h).
