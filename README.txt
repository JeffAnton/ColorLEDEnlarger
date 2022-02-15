Color LED Enlarger Head with 5x7 LED display and IR remote control

Copyright 2020, 2022 Jeffrey Anton
See LICENSE file

This is the code for my LED Enlarger Head I've built.
It's my most used enlarger. It's built into a Beseler Cold Light
head case. That unit is VERY rare. It may have been a custom built
item as people who used to manufacture cold light bulbs and heads
told me it didn't exist. I sent photos of it and the transformer it
used and they begged me not to plug it in as it would be unsafe.
Since I could not get a real cold light bulb for this, I decided to
build this LED head.

It's built for the Arduino Nano Every board with a 5x7 led matrix,
IR remote control, RGB led strips and discrete near UV leds.
The 5x7 led matrix display and IR sensor fits through the old head's
switch access hole.

The reason for the near UV leds is that the blue in most led strips is
not "blue" enough. The warelength is too close to green to allow for
full multigrade paper range. And the UV leds allow for extream high
contrast grades beyond what a 5 grade filter would give.

I plan to write some contrast calibration in the future.

It uses a 5x3 pixel font, see the JeffAnton/pixelfonts repo.
It also uses the "Infrared" IR library also known as Infrared4Arduino.

It should work with a traditional Arduino instead of the Nano Every.
I initially built pieces of this against an Arduino Duemillenove then
moved the parts to the Nano Every.  I'm not using any special features of
the Nano Every, yet. Read the source code for the pins mapping of the
different LED colors if you change to a differenet Arduino. The pins must
support PWM and the Nano Every is different from most other units.

The display is of my own construction.  The multiplexing is done by the
controller itself with just resistors and MOSFET drivers for the display.
I'll eventually post a schematic, but it's so trivial it's hardly worth it.
The columns are driven by 2N7000 MOSFETs and the rows just have 560 ohm
resistors.  Because this is ment to operate in a darkroom, the current limiting
is higher that would strictly be needed.  But I want a display which is not
bright in the dark, but readable with room lights.  Also, this limits the
current draw against the Arduino Nano Every which reports a 20 mA pin limit
and a 200 mA board limit.

The main LEDs in the head are driven by 4 power MOSFETs specifically the
INFZ44.

The IR remote control is hard coded for a very old Sanyo VCR control I have.

If you want to build this yourself you will want to change the code for a
different IR remote and probably a different LED dispay such as a dot matrix
driven by a MAX7219 (I think that's the code).  To use that via SIP you will
need to move the pin usages around.
