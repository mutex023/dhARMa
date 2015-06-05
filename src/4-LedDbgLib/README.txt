4-LedDbgLib:
======================

This is a logical extension of the program in "2".
It provides different assembly procedures to use the BBB usr0-3 led's
for debugging purposes:
[1] Toggle a specific led
[2] Turn on/off a specific led
[3] Output a binary number on the the leds from 0000 - 1111
[4] Signal a panic/assert
[5] Blink an led pattern 0000 - 1111

NOTE - You have to save your registers before calling into the LED debug library procedures !!

To build: 
1) This cannot be built directly, you need to build this along with the other modules which will use the ledDbgLib.
3) See test-led.s & build-test.sh as to how to do it
