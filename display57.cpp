#include <arduino.h>
#include "font.h"
#include "task.h"
#include "display57.h"

/*
 * Multiplex display 5 x 7
 * 
 * Copyright 2020 Jeffrey Anton
 * See LICENSE file
 * 
 * Pin arrangement for Arduino Nano Every
 * Avoids PWM pins (except for 6)
 *
 * A0-A4 are rows
 * 2, 4, 6, 7, 11, 12, A5 are columns
 */

byte Display57::pinList[] = { 2, 4, 6, 7, 11, 12, A5 };

void
Display57::setbits(byte v)
{
  static byte last = 0;

  byte change = last ^ v;
  last = v;
  for (int p = A0; change && p < A5; ++p) {
    if (change & 1)
      digitalWrite(p, v & 1);
    v >>= 1;
    change >>= 1;
  }
}

void
Display57::init() {
  // clear all pins, make analog in pins digital out ones
  for (int i = 0; i <= 6; ++i)
    initpin(pinFromColumn(i));
  for (int i = A0; i <= A4; ++i)
    initpin(i);
  column = 0;
  disp = dmem;
  digitalWrite(LED_BUILTIN, HIGH);
}

void
Display57::initpin(int i) {
    digitalWrite(i, LOW);
    pinMode(i, OUTPUT);  
}

void
Display57::show(unsigned int v)
{
    char s[3];

    if (v < 10) {
      s[0] = v + '0';
      s[1] = '\0';
    }
    else if (v < 100) {
      s[0] = v / 10 + '0';
      s[1] = v % 10 + '0';
      s[2] = '\0';
    }
    show(s);
}

void
Display57::run() {
  digitalWrite(pinFromColumn(column), LOW);
  if (++column == 7)
    column = 0;
  setbits(disp[column]);
  digitalWrite(pinFromColumn(column), HIGH);
}

void
Display57::show(const char *s) {
  cleardisp();
  if (*s) {
    if (s[1]) {
      fillchar(*s, dmem);
      fillchar(s[1], dmem+4);
    }
    else {
      fillchar(*s, dmem+2);
    }
  }
}

void
Display57::fillchar(char c, byte *d) {
  const glyph5x3 *p = font5x3+c-0x20;
  *d++ = pgm_read_byte_near(*p);
  *d++ = pgm_read_byte_near(1+*p);
  *d = pgm_read_byte_near(2+*p);
}

void
Display57::cleardisp() {
  byte *d = dmem;
  *d++ = 0;
  *d++ = 0;
  *d++ = 0;
  *d++ = 0;
  *d++ = 0;
  *d++ = 0;
  *d = 0;
  disp = dmem;
}

void
Display57::setd(unsigned int d, byte b) {
  if (d < 7)
    dmem[d] = b;
}
