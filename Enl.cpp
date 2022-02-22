#include <Arduino.h>
#include <IrReceiverSampler.h>
#include <Nec1Decoder.h>
#include "task.h"
#include "display57.h"
#include "preset.h"

/*
 * Color LED Enlarger
 * Copyright 2022 Jeffrey Anton
 * 
 * See LICENSE file
 */

static bool fixed;
static Display57 disp;

static byte cross[] = { 0, 021, 012, 04, 012, 021, 0 };
static byte upptr[] = { 0, 04, 010, 037, 010, 04, 0 };
static byte downptr[] = { 0, 04, 02, 037, 02, 04, 0 };

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
  disp.clear();
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

private:
  int pin;
  int val;
  bool down;
  int column;
  static int scale[];
};

// one stop is a three, i.e. 1 is a third of a stop
int LedPower::scale[] = { 0, 0x2a, 0x35, 0x40, 0x55, 0x6a, 0x80, 0xaa, 0xb5 };

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
      analogWrite(pin, scale[i]);
    }
}

void
LedPower::run() {
    if (fixed)
      return;
    if (down) {
      set(val-1);
    } else {
      set(val+1);
    }
    disp.setd(column, bits(val));
}

#if defined(ARDUINO_AVR_NANO_EVERY) || defined(ARDUINO_ARDUINO_NANO33BLE)
LedPower blue(9, 10, 4);
LedPower red(10, 15, 0);
LedPower green(3, 25, 2);
LedPower violet(5, 35, 6);
#else // classic 328P, 128 AVR processors
LedPower blue(9, 10, 4);
LedPower red(10, 15, 0);
LedPower green(11, 25, 2);
LedPower violet(6, 35, 6);
#endif

byte lastred, lastgreen, lastblue, lastviolet;
int expose, saveexp;

class Remote : public Task {
    IrReceiver *receiver;
    int selected;
    int contrast;
    int lastbutton;
    int whitefocus;
    PresetMgr preset;
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
    selected = 1;
  dispmode();
}

void
Remote::modeup()
{
  switch(selected) {
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
  fixed = true;
  
  red.init();
  green.init();
  blue.init();
  violet.init();

  saveexp = 15;
  expose = 0;
  lastred = lastgreen = lastblue = lastviolet = 0;

  irrem = new Remote();
  disp.show("04");  // version 0.4
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
  selected = 1;
  contrast = 0;
  lastbutton = -1;
  whitefocus = 0;

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
    int button;

    receiver->disable();
    if (!receiver->isEmpty()) {
      Nec1Decoder decoder(*receiver);
      if (decoder.isValid() && !decoder.isDitto() && decoder.getD() == 53) {
        // got a key - do something
        switch (button = decoder.getF()) {
          case 1:
            // all RED
            red.set(lastred = 9);
            showexp();
            break;
          case 2:
            // all GREEN
            green.set(lastgreen = 9);
            showexp();
            break;
          case 3:
            // all BLUE
            blue.set(lastblue = 9);
            showexp();
            break;
          case 4:
            // all VIOLET
            violet.set(lastviolet = 9);
            showexp();
            break;
          case 11:
            // all OFF
            red.set(0);
            green.set(0);
            blue.set(0);
            violet.set(0);
            lastred = lastgreen = lastblue = lastviolet = 0;
            disp.setbits(cross);
            break;
          case 19:
            // DEMO?
            fixed = !fixed;
            if (!fixed)
              disp.clear();
            break;
          case 84:
            // do EXPOSURE
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
            // lights OFF
            red.set(0);
            green.set(0);
            blue.set(0);
            violet.set(0);
            showexp();
            break;
          case 29:
            // FOCUS light
            if (lastbutton == 29)
              whitefocus ^= 1;
            red.set(whitefocus ? 9 : 0);
            green.set(9);
            blue.set(9);
            violet.set(0);
            disp.setd(3, 1);
            break;
          case 14: // time down by HALF
            saveexp /= 2;
            if (saveexp <= 0)
              saveexp = 1;
            disp.show(saveexp);
            break;
          case 72:
            // down 1 SECOND
            if (saveexp > 1)
              --saveexp;
            disp.show(saveexp);
            break;
          case 73:
            // up 1 SECOND
            if (saveexp < 99)
              ++saveexp;
            disp.show(saveexp);
            break;
          case 15: // time up by DOUBLE
            saveexp *= 2;
            if (saveexp > 99)
              saveexp = 99;
            // fall through
          case 0: // show current TIME
            disp.show(saveexp);
            break;
          case 20: // change fine color mode
            rotatemode();
            break;
          case 30: // color value down
            modedown();
            break;
          case 31: // color value up
            modeup();
            break;
          case 0x45:  // set to contrast
            disp.show("C");
            preset.Get( contrast, lastred, lastgreen, lastblue, lastviolet );
            break;
          case 0x41:  // contrast down
            disp.setbits(downptr);
            if (contrast-- == 0)
              contrast = preset.Count() - 1;
            preset.Get( contrast, lastred, lastgreen, lastblue, lastviolet );
            break;
          case 0x40:  // contrast up
            disp.setbits(upptr);
            if (++contrast >= preset.Count())
              contrast = 0;
            preset.Get( contrast, lastred, lastgreen, lastblue, lastviolet );
            break;
          case 0x58:  // store contrast setting
            disp.inverse();
            preset.Set( contrast, lastred, lastgreen, lastblue, lastviolet );
            break;
          case 0x15:  // font demo?
          default:
            if (button == lastbutton)
              disp.showhex(button);
        }
        lastbutton = button;
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
