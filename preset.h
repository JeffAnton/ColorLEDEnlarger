#ifndef PRESET_H
#define PRESET_H

/*
 *  Enlarger Preset manager
 *  Copyright 2022 Jeffrey Anton
 */

 class PresetMgr {
  public:
    PresetMgr( unsigned int n = 12 );
    ~PresetMgr();
    int Count() const { return cnt; }
    bool Set(unsigned int p, byte r, byte g, byte b, byte v);
    bool Get(unsigned int p, byte &r, byte &g, byte &b, byte &v);
    
  private:
    void recover(byte);
    void save(byte);
    unsigned int cnt;
    struct c {
      byte r, g, b, v;
    } *item;
};

#endif
