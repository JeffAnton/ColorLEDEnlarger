#include <Arduino.h>
#include <IrReceiverSampler.h>
#include <Nec1Decoder.h>
#include "task.h"
#include "display57.h"

/*
 * Color LED Enlarger
 * Copyright 2020 Jeffrey Anton
 * 
 * See LICENSE file
 */

static bool fixed;
static Display57 disp;

static byte
bits(int v)
{
  switch (v) {
    case 0:
      return 0;
    case 1:
    case 2:
      return 01;
    case 3:
    case 4:
      return 03;
    case 5:
    case 6:
      return 07;
    case 7:
    case 8:
      return 017;
    default:
      return 037;
  }
}

static void
setgraph(int a, int b, int c, int d)
{
  disp.cleardisp();
  disp.setd(0, bits(a));
  disp.setd(2, bits(b));
  disp.setd(4, bits(c));
  disp.setd(6, bits(d));
}

class LedPower : public TimedTask {
public:
  LedPower(int p, int d = 10, int c = 9) : TimedTask(d * 25), pin(p), val(0),
      column(c), down(false) {}
  void init(int);
  void set(int);
  void run();
  int get() const { return val; }

private:
  int pin;
  int val;
  bool down;
  int column;
};

void
LedPower::init(int v = 0)
{
    pinMode(pin, OUTPUT);
    set(v);
}

void
LedPower::set(int i) {
    val = i;
    if (i == 0) {
      digitalWrite(pin, LOW);
      down = false;
    } else if (i > 8) {
      digitalWrite(pin, HIGH);
      down = true;
    } else {
      analogWrite(pin, 1 << (i - 1));
    }
}

void
LedPower::run() {
    if (fixed)
      return;
    if (down) {
      set(val-1);
      if (val == 0)
        down = false;
    } else {
      set(val+1);
      if (val == 9)
        down = true;
    }
    disp.setd(column, bits(val));
}

#if defined(ARDUINO_AVR_NANO_EVERY) || defined(ARDUINO_ARDUINO_NANO33BLE)
LedPower blue(9, 10, 4);
LedPower red(10, 15, 0);
LedPower green(3, 25, 2);
LedPower violet(5, 35, 6);
#elif defined(ARDUINO_AVR_DUEMILANOVE)
LedPower blue(9, 10, 4);
LedPower red(10, 15, 0);
LedPower green(11, 25, 2);
LedPower violet(6, 35, 6);
#endif

int lastred, lastgreen, lastblue, lastviolet;
int expose, saveexp;

class Remote : public Task {
    IrReceiver *receiver;
    int selected;
    int contrast;
  public:
    Remote();
    bool runable(unsigned long);
    void run();
    void rotatemode();
    void modeup();
    void modedown();
    void dispmode();
} *irrem;

void
Remote::dispmode()
{
  char d[2];
  switch(selected) {
    case 0:
      d[0] = 'C';
      d[1] = '0'+contrast;
      break;
    case 1:
      d[0] = 'R';
      d[1] = '0'+lastred;
      break;
    case 2:
      d[0] = 'G';
      d[1] = '0'+lastgreen;
      break;
    case 3:
      d[0] = 'B';
      d[1] = '0'+lastblue;
      break;
    case 4:
      d[0] = 'V';
      d[1] = '0'+lastviolet;
      break;
  }
  disp.show(d);
}

void
Remote::rotatemode()
{
  if (++selected == 5)
    selected = 0;
  dispmode();
}

void
Remote::modeup()
{
  switch(selected) {
    case 0:
      if (++contrast == 10)
        contrast = 0;
      break;
    case 1:
      if (++lastred == 10)
        lastred = 0;
      break;
    case 2:
      if (++lastgreen == 10)
        lastgreen = 0;
      break;
    case 3:
      if (++lastblue == 10)
        lastblue = 0;
      break;
    case 4:
      if (++lastviolet == 10)
        lastviolet = 0;
      break;
  }
  dispmode();
}

void
Remote::modedown()
{
  switch(selected) {
    case 0:
      if (contrast-- == 0)
        contrast = 9;
      break;
    case 1:
      if (lastred-- == 0)
        lastred = 9;
      break;
    case 2:
      if (lastgreen-- == 0)
        lastgreen = 9;
      break;
    case 3:
      if (lastblue-- == 0)
        lastblue = 9;
      break;
    case 4:
      if (lastviolet-- == 0)
        lastviolet = 9;
      break;
  }
  dispmode();
}

void setup() {
  // put your setup code here, to run once:

  fixed = true;
  
  red.init();
  green.init();
  blue.init();
  violet.init();

  saveexp = 15;
  expose = 0;
  lastred = lastgreen = lastblue = lastviolet = 0;

  irrem = new Remote();
  disp.show("02");  // version 0.1
  disp.setd(3, 2);  // decimal point
  pinMode(LED_BUILTIN, OUTPUT);
}

class Exposure : public TimedTask
{
  int secs;
  public:
    Exposure(int t);
    void run();
} *exptim = NULL;

Exposure::Exposure(int t)
  : secs(t)
{
  disp.show(secs);
  digitalWrite(LED_BUILTIN, HIGH);
}
void
Exposure::run()
{
  if (--secs) {
    disp.show(secs);
    return;
  }
  red.set(0);
  green.set(0);
  blue.set(0);
  violet.set(0);
  digitalWrite(LED_BUILTIN, LOW);
  disp.show(saveexp);
  exptim = NULL;
  delete this;
}

Remote::Remote()
{
  receiver = IrReceiverSampler::newIrReceiverSampler(200, 8);
  selected = 0;
  contrast = 0;

  receiver->enable();
}

bool
Remote::runable(unsigned long)
{
  return receiver->isReady();
}

void
showexp()
{
  setgraph(lastred, lastgreen, lastblue, lastviolet);
}

void
Remote::run()
{
    receiver->disable();
    if (!receiver->isEmpty()) {
      Nec1Decoder decoder(*receiver);
      if (decoder.isValid() && !decoder.isDitto() && decoder.getD() == 53) {
        // got a key - do something
        switch (decoder.getF()) {
          case 1:
            red.set(lastred = 9);
            showexp();
            break;
          case 2:
            green.set(lastgreen = 9);
            showexp();
            break;
          case 3:
            blue.set(lastblue = 9);
            showexp();
            break;
          case 4:
            violet.set(lastviolet = 9);
            showexp();
            break;
          case 11:
            red.set(0);
            green.set(0);
            blue.set(0);
            violet.set(0);
            lastred = lastgreen = lastblue = lastviolet = 0;
            disp.show("##");
            break;
          case 19:
            fixed = !fixed;
            if (!fixed)
              disp.cleardisp();
            break;
          case 84:
            fixed = true;
            if (exptim)
              delete exptim;
            exptim = new Exposure(saveexp);
            red.set(lastred);
            green.set(lastgreen);
            blue.set(lastblue);
            violet.set(lastviolet);
            break;
          case 12:
            red.set(0);
            green.set(0);
            blue.set(0);
            violet.set(0);
            showexp();
            break;
          case 29:
            red.set(0);
            green.set(9);
            blue.set(9);
            violet.set(0);
            disp.setd(3, 1);
            break;
          case 14: // down
            saveexp /= 2;
            if (saveexp <= 0)
              saveexp = 1;
            disp.show(saveexp);
            break;
          case 72:
            if (saveexp > 1)
              --saveexp;
            disp.show(saveexp);
            break;
          case 73:
            if (saveexp < 99)
              ++saveexp;
            disp.show(saveexp);
            break;
          case 15: // up
            saveexp *= 2;
            if (saveexp > 99)
              saveexp = 99;
            // fall through
          case 0:
            disp.show(saveexp);
            break;
          case 20:
            rotatemode();
            break;
          case 30:
            modedown();
            break;
          case 31:
            modeup();
            break;
        }
      }
    }
    receiver->enable();
}

void loop() {
  unsigned long m = millis();

  disp.poll(m);

  if (exptim)
    exptim->poll(m);

  irrem->poll(m);
  red.poll(m);
  green.poll(m);
  blue.poll(m);
  violet.poll(m);
}
