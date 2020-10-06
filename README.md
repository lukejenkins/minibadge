# SAINTCON Minibadge

### Version 2.0  

<img src="minibadge-footprint.png" width=500px>

## A New Minibadge Standard ##

The Saincon Badge Team presents the new minibadge spec.

Changes:
- Added the CLK pin.
- Added dedicated pins for programming microcontrollers on the minibadge.
- Removed SPI data bus (It was never supported.)
- Change +5V to +VBATT to more accurately reflect usage. This pin can be anywhere from +5v to +3.3v volts.

## +VBATT ##
This pin can operate anywhere from +5v to +3.3v. You can use it if you want your LEDs to be a little brighter, __Make sure they can
handle the current__. In recent years it has been at lipo battery voltage ~4v. There have been discussions to make it 3.3v, but to
keep backwards compatibility with other badges we advise if you use it to allow it to run up to 5v.

__DO NOT CONNECT IT TO 3.3v__

## CLK ##
This pin is controlled via the badge and pulses around 1 to 10 Hz. This can be useful to sync minibadge updates and
allow minibadges to have more elaborate functionality without needing to create their own clock.

## I2C ##
There have occasionally been issues with how the badge communicates with minibadges. To prevent this there is now a standard
for how to use I2C communication.

__NOTE: I2C minibadges should act like a sequential read ROM. Communication will start and stop multiple times for each message so
microcontrollers will need to keep track of the current state and what to send outside the event functions.__

##### Reading #####
I2C communication should work like a sequential read ROM. The badge will initiate communication with the minibadges and tell
it to return to address by writing 0x00 0x00. This indicates the beginning of the read process. After this the badge will
send a two bit read request the first byte is a bool and indicates if the minibadge will accept write commands and the second
is as follows:

* 0x00: Do nothing.
* 0x01: Button Pressed. (The minibadge will need to clear this.)
* 0x02: Text. (The next byte is the length of the message Max value 0xFF followed by the ascii message.)

##### Write #####
When the badge wants to tell the minibadge something it will first initiate communication by writing 0x00 0x00 then reading one byte.
If the byte is 0x01 it will proceed with writing and if it 0x00 it will end communication. Then if the minibadge supports writing the
badge will send the instruction byte and any additional bytes as follows:

* 0x01 Score Update. (Then two bytes with the score.)
* 0x02 Brightness. (Then one more byte to set the brightness: 0x0 to 0x7F)


If you would like to use I2C for a minibadge there will be an official list of used addresses. If you would like to
reserve an address open a pull request with the 7-bit address so it can be added.

## PROG ##
These pins will never be connected to the badge and can be used how ever you want to use them. They are meant for
a useful drop in place programming pinout.

Here are some optional guides:

| Type        | Pin 3 | Pin 4 | Pin 5 | Pin 6 |
|-------------|-------|-------|-------|-------|
| AVR ISP     | MISO  | CLK   | MOSI  | RESET |
| ST-Link SWD | SWIM  | SWCLK | SWDIO | RESET |
| PIC ICSP    | PGM   | PGC   | PGD   | MCLR  |
| UART        | RX    | DTR   | TX    | RTS   |


## NC ##
These pins are reserved for future use. Do not connect anything including themselves to them as this may cause damage to either
minibadges, the host badge, or both.


This library is provided as is. Use at your own risk.  

**Licensing:** Creative Commons ShareAlike 4.0 International - https://creativecommons.org/licenses/by-sa/4.0/  
