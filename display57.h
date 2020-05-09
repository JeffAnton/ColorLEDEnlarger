#ifndef DISPLAY57_H
#define DISPLAY57_H

/*
 * Multiple 5x7 display
 * 
 * Copyright 2020 Jeffrey Anton
 * See LICENSE file
 */

class Display57 : public TimedTask {
  public:
    Display57() : TimedTask(2) { init(); }
    void init();
    void show(const char *);
    void show(unsigned int v);
    void fillchar(char, byte *);
    void cleardisp();
    void setd(unsigned int i, byte b);
    void run();
  private:
    void setbits(byte);
    const byte *disp;
    byte dmem[7];
    int column;
    static byte pinList[];
    static int pinFromColumn(int i) { return pinList[i]; }
    static void initpin(int);
};

#endif
