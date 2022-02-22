#include <Arduino.h>
#include <EEPROM.h>
#include "preset.h"

/*
 * Preset Manager
 * Copyright 2022 Jeffrey Anton
 */

PresetMgr::PresetMgr( unsigned int n )
  : cnt(n)
{
  item = new c[n];
  for (int i = 0; i < n; i++)
    recover(i);
}

PresetMgr::~PresetMgr()
{
  delete [] item;
}

bool
PresetMgr::Set( unsigned int p, byte r, byte g, byte b, byte v )
{
   if (p >= cnt)
     return false;

   struct c *i = item + p;
   i->r = r;
   i->g = g;
   i->b = b;
   i->v = v;
   save(p);
   return true;
}

bool
PresetMgr::Get( unsigned int p, byte &r, byte &g, byte &b, byte &v )
{
  if (p >= cnt)
    return false;

  struct c *i = item + p;
  r = i->r;
  g = i->g;
  b = i->b;
  v = i->v;
  return true;
}

void
PresetMgr::recover(byte p)
{
  struct c *i = item + p;
  int a = p * sizeof *item;
  EEPROM.get(a, *i);
  // if location is uninitialized - then init to zero
  if (i->r == 255)
    i->r = i->g = i->b = i->v = 0;  
}

void
PresetMgr::save(byte p)
{
  int a = p * sizeof *item;
  EEPROM.put(a, item[p]);
}
