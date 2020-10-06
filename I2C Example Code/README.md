# Reading #
## Do Nothing ##
```
[ 0bXXXXXXX1 0x00 0x01 ]
[ 0bXXXXXXX0 r(0x00) ]    // 0x00 for no communication.
// End communication
```

## Button Press ##
```
[ 0bXXXXXXX1 0x00 0x01 ]
[ 0bXXXXXXX0 r(0x01) ]    // 0x01 for button press.
// End communication
```

## Text Message ##
```
[ 0bXXXXXXX1 0x00 0x01 ]
[ 0bXXXXXXX0 r(0x02) ]    // 0x02 for text message.
[ 0bXXXXXXX0 r(0xXX) ]    // Where 0xXX is the message length.
[ 0bXXXXXXX0 r(0xXX):n ]  // r repeated n times where n is the message length received and 0xXX is the hex value of the ascii character.
// End communication
```

# Writing #
## No Write Support ##
```
[ 0bXXXXXXX1 0x00 0x00 ]
[ 0bXXXXXXX0 r(0x00) ] // 0x00 for no write support.
// End communication
```

## Update Score ##
```
[ 0bXXXXXXX1 0x00 0x00 ]
[ 0bXXXXXXX0 r(0x01) ]        // 0x01 for write support.
[ 0bXXXXXXX1 0x01 0xXX 0xXX ] // 0x01 for score update and where 0xXX 0xXX is the two byte score.
// End communication
```

## Update Brightness ##
```
[ 0bXXXXXXX1 0x00 0x00 ]
[ 0bXXXXXXX0 r(0x01) ]    // 0x01 for write support.
[ 0bXXXXXXX1 0x01 0xXX ]  // 0x02 for brightness update and where 0xXX is the brightness (max of 127).
// End communication
```
