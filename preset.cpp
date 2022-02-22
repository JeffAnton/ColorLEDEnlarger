#include <Arduino.h>
#include "preset.h"

/*
 * Preset Manager
 * Copyright 2022 Jeffrey Anton
 */

PresetMgr::PresetMgr( unsigned int n )
  : cnt( n )
{
  item = new c[ n ];
}

PresetMgr::~PresetMgr()
{
  delete [] item;
}

bool
PresetMgr::Set( unsigned int p, byte r, byte g, byte b, byte v )
{
   if ( p >= cnt )
     return false;

   struct c *i = item + p;
   i->r = r;
   i->g = g;
   i->b = b;
   i->v = v;
   return true;
}

bool
PresetMgr::Get( unsigned int p, byte &r, byte &g, byte &b, byte &v )
{
   if ( p >= cnt )
     return false;

   struct c *i = item + p;
   r = i->r;
   g = i->g;
   b = i->b;
   v = i->v;
   return true;  
}
 
