# Minibadge Debugger #
__Do not use an Arduino with 5v IO pins!! It may damage your minibadge!__

The debugger code can be interfaced with from the Arduino IDE or any terminal tool like
putty or minicom. The BAUD rate is 9600.




# Reading #
## Do Nothing ##
```
[ 0bXXXXXXX0 0x00 0x01 ]
[ 0bXXXXXXX1 r(0x00) ]    // 0x00 for no communication.
// End communication
```

## Button Press ##
```
[ 0bXXXXXXX0 0x00 0x01 ]
[ 0bXXXXXXX1 r(0x01) ]    // 0x01 for button press.
// End communication
```

## Text Message ##
```
[ 0bXXXXXXX0 0x00 0x01 ]
[ 0bXXXXXXX1 r(0x02) ]    // 0x02 for text message.
[ 0bXXXXXXX1 r(0xXX) ]    // Where 0xXX is the message length.
[ 0bXXXXXXX1 r(0xXX):n ]  // r repeated n times where n is the message length received and 0xXX is the hex value of the ascii character.
// End communication
```

# Writing #
## No Write Support ##
```
[ 0bXXXXXXX0 0x00 0x00 ]
[ 0bXXXXXXX1 r(0x00) ] // 0x00 for no write support.
// End communication
```

## Update Score ##
```
[ 0bXXXXXXX0 0x00 0x00 ]
[ 0bXXXXXXX1 r(0x01) ]        // 0x01 for write support.
[ 0bXXXXXXX0 0x01 0xXX 0xXX ] // 0x01 for score update and where 0xXX 0xXX is the two byte score.
// End communication
```

## Update Brightness ##
```
[ 0bXXXXXXX0 0x00 0x00 ]
[ 0bXXXXXXX1 r(0x01) ]    // 0x01 for write support.
[ 0bXXXXXXX0 0x02 0xXX ]  // 0x02 for brightness update and where 0xXX is the brightness (max of 127).
// End communication
```


# I2C ROM Support #
If you want to display custom messages, but not program the functionality you can use an I2C rom. This protocol should work
with ROMs you just need to set the first byte of the rom to 0x0 and the second byte to the code for your read message. (I suggest
not setting it to 0x01, the button press, because the badge won't unset the value. It will act like you are spamming the button.)
The first bytes of ROM should look something like this:
```
0x00 0x02 0x0B 'H' 'E' 'L' 'L' 'O' ' ' 'W' 'O' 'R' 'L' 'D'
// This will display "HELLO WORLD" and the length is 11 or 0x0B
```
