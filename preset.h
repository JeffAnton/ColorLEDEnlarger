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
    bool Set( unsigned int p, byte r, byte g, byte b, byte v );
    bool Get( unsigned int p, byte &r, byte &g, byte &b, byte &v );
    
  private:
    unsigned int cnt;
    struct c {
      byte r, g, b, v;
      c() : r( 0 ), g( 0 ), b( 0 ), v( 0 ) {}
    } *item;
};

#endif
