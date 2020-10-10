/*

Author: Michael Julander ( @Sodium_Hydrogen, MikeJ.Tech )

SAINTCON Minibadge Developer Environment.
This tool emulates how the SAINTCON badge should communicate to a minibadge
both I2C and the clock pin through a nice command line interface. The cli
can be access by any serial monitor including the Arduino IDE's built in
serial monitor. If you do not have access to that then an application
like putty or minicom will work.



*/
#include "Wire.h"

void write_device(uint8_t, uint8_t, uint8_t, char*);
void read_device(uint8_t, bool);

void update_brightness(String);
void set_clock(String);
void info_from_device(String);
void read_from_device(String);
void scanner(String);
void update_score(String);
void print_help(String);


//--------------------------------------------------

char* command[] = {
  "bright",
  "clock",
  "info",
  "read",
  "scan",
  "score",
  "help"
};

void (* funct[])(String) = {
  &update_brightness,
  &set_clock,
  &info_from_device,
  &read_from_device,
  &scanner,
  &update_score,
  &print_help
};

const char _help0[] PROGMEM = " ADDR BRIGHT:\t Sends the brightness to device at ADDR. (BRIGHT 0-127 in decimal.)";
const char _help1[] PROGMEM = " PIN HZ:\t\t Sets a clock on PIN to pulse at a 50% duty cycle set at HZ (0 to turn it off. Max of 20 Hz.)";
const char _help2[] PROGMEM = " ADDR:\t\t Similar to read, but it also reports if write is supported.";
const char _help3[] PROGMEM = " ADDR:\t\t Checks the device at ADDR for a message and outputs all I2C transactions.";
const char _help4[] PROGMEM = ":\t\t\t Scan the I2C bus and print all address that are found.";
const char _help5[] PROGMEM = " ADDR SCORE:\t Sends the SCORE to device at ADDR. (SCORE is in decimal.)";
const char _help6[] PROGMEM = ":\t\t\t Print this message.";

const char* const help[] PROGMEM = {
  _help0,
  _help1,
  _help2,
  _help3,
  _help4,
  _help5,
  _help6
};
//--------------------------------------------------

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.print(F("\n\rSAINTCON Minibadge Debugger \n\n\r$ "));

}

String msg = "";
char history[10][30] = {};
uint8_t history_loc = 0;
uint8_t clk_pin;
uint8_t clk_hz = 0;
uint32_t time_since_last_pulse = 0;
uint8_t escape = 0;

void loop() {
  if(Serial.available()){ // Process the serial interface for cli like behavior
    char read = Serial.read();
    if(read == '\n' || read == '\r'){// Only parse once a newline character is present
      msg.trim();
      Serial.println();
      String com = msg.substring(0, msg.indexOf(" ")); //take just the first part of the command
      com.toLowerCase();

      bool reconCom = false;
      for(uint8_t i = 0; i < sizeof(command)/sizeof(char*); i++){
        if(strcmp(com.c_str(), command[i]) == 0){
          funct[i](msg);
          reconCom = true;
          break;
        }
      }
      if(!reconCom){ // Print error message if command is never run
        Serial.print(F("Unable to find command matching: \""));
        Serial.print(com);
        Serial.println(F("\"\n\rUse help for a list of all commands."));
      }
      if(msg.length() > 0 && history_loc == 0){ // shift all history locations one back
        for(uint8_t i = 9; i > 0; i--){
          strcpy(history[i], history[i-1]);
        }
      }
      strcpy(history[0], ""); // clear most recent history location
      history_loc = 0;
      Serial.print("$ ");
      msg = ""; // clear command from buffer to prep for next read
    }else{
      if(escape == 1 && read == '['){ // detect sequence for arrows
        read = 0;
        escape = 2;
      }else if(escape == 2 && (read == 'A' || read == 'B')){ // read up and down arrow keys and shift history
        Serial.print("\x1B[2K\r$ "); // clear current line and refil with default terminal entry
        if(read == 'A'){
          if(history_loc < 9){
            history_loc++;
          }
        }else{
          if(history_loc > 0){
            history_loc--;
          }
        }
        Serial.print(history[history_loc]);
        msg = history[history_loc]; // copy history into buffer
        read = 0;
        escape = 0;
      }else{ // if sequence does not match up and down arrows end sequence interpreting
        escape = 0;
      }
      if(read == 27){// catch escape character
        escape = 1;
      }else if(read >= 0x20 && read < 0x80){
        if(history_loc != 0){ // If you are editing the buffer make it the current command
          history_loc = 0;
        }
        if(read == 127){  // remove character from buffer if backspace is encountered.
          int8_t length = msg.length()-1;
          if(length < 0){
            length = 0;
          }else{
            Serial.print(read);
          }
          msg = msg.substring(0, length);
        }else if(msg.length() < 29){
          msg.concat(read); // If the new character is not a newline append character to back of buffer
          Serial.print(read);
        }
        strcpy(history[0], msg.c_str()); // keep history of command
      }
    }
  }

  // This will run when the clk_hz is set to anything but 0 and will run at 50% duty cycle
  if( clk_hz != 0 && millis() > ( time_since_last_pulse + (uint32_t)(500/clk_hz) )){
    time_since_last_pulse = millis();
    digitalWrite(clk_pin, !digitalRead(clk_pin));
  }
}

// This will parse a command to get the ADDR argument
// It will return 255 (out of range) if the address cannot be determined
uint8_t parse_addr(String com){
  if(com.indexOf(" ") == -1){ // Make sure ADDR has been provided
    Serial.println(F("Please provice an address to read from. "));
    return 255;
  }
  String addr = com.substring(com.indexOf(" ")+1); // get arguments
  addr.trim(); // remove extra whitespace
  addr.toLowerCase();
  if(addr.indexOf(" ") != -1){ // Trim any extra arguments off
    addr = addr.substring(0, addr.indexOf(" "));
  }
  uint8_t target = 0;
  if(addr.startsWith("0x")){
    target = (uint8_t)strtol(&(addr.c_str()[2]), nullptr, 16); // Parse hex
  }else if(addr.startsWith("0b")){
    target = (uint8_t)strtol(&(addr.c_str()[2]), nullptr, 2); // Parse binary
  }else{
    target = (uint8_t)strtol(addr.c_str(), nullptr, 10); // Parse decimal
  }
  if(target > 127){
    Serial.println(F("Please enter an address in range 0-127 (0x7F).")); // Error if address is out of range
    return 255;
  }
  return target;
}

// This reads from the provided device if it is connected
// getWriteState will output if the minibadge supports writing as well as make read less verbose
void read_device(uint8_t addr, bool getWriteState){
  uint8_t res = Wire.read();
  Wire.beginTransmission(addr);
  if(Wire.endTransmission(addr) == 0){ // Check if device is on bus before beginning read.
    if(getWriteState){
      Wire.beginTransmission(addr);
      Wire.write(0x00);
      Wire.write(0x00);
      Wire.endTransmission();

      Wire.requestFrom(addr, 1);
      res = Wire.read();
      Serial.print("Device at 0x");
      Serial.print( (addr< 16)?"0":"");
      Serial.print(addr, HEX);
      Serial.print( res?" supports":" does not support");
      Serial.println(" writing.");
    }else{
      Serial.print(F("Reading from 0x"));
      Serial.print( (addr< 16)?"0":"");
      Serial.println(addr, HEX);
      Serial.println(F("\tSending 0x00 0x01"));
    }

    Wire.beginTransmission(addr);
    Wire.write(0x00);
    Wire.write(0x01);
    Wire.endTransmission();

    Wire.requestFrom(addr, 1);
    res = Wire.read();
    if(!getWriteState){
      Serial.print(F("\tRecieved 0x"));
      Serial.print( (res< 16)?"0":"");
      Serial.println(res, HEX);
    }

    if(res == 0){
      Serial.println(F("Noting to do.\n\rEnding read."));
    }else if(res == 1){
      Serial.println(F("Read button press.\n\rEnding read."));
    }else if(res == 2){
      Wire.requestFrom(addr, 1);
      res = Wire.read();
      if(getWriteState){
        Serial.print(F("Read available message. Length: "));
        Serial.print(res);
        Serial.print(" Text: \"");
      }else{
        Serial.println(F("Read available message. Getting Length."));
        Serial.print(F("\tReceived 0x"));
        Serial.print( (res< 16)?"0":"");
        Serial.println(res, HEX);
        Serial.print(F("Getting message.\n\r\tReceived"));
      }

      char *text = (char *)malloc(res+1);
      uint8_t counter = 0;
      Wire.requestFrom(addr, res);
      while(Wire.available()){
        text[counter++] = (char)Wire.read();
        if(!getWriteState){
          Serial.print( (text[counter-1]< 16)?" 0x0":" 0x");
          Serial.print(text[counter-1], HEX);
        }
      }
      if(!getWriteState){
        Serial.print("\n\rText: \"");
      }
      text[res] = '\0';
      Serial.print(text);
      Serial.println("\"\n\rEnding read.");
    }

  }else{
    Serial.print(F("Unable to find device at 0x"));
    Serial.print( (addr< 16)?"0":"");
    Serial.println(addr, HEX);
  }
}


// This will send a command to the minibadge.
// Command should match the minibadge documentation and args is an array
// of one byte values to send. argc is the array length.
void write_device(uint8_t addr, uint8_t command, uint8_t argc, char* args){
  uint8_t res;
  Wire.beginTransmission(addr);
  if(Wire.endTransmission(addr) == 0){ // Check if device is on bus before beginning read.
    Serial.print(F("Writing to 0x"));
    Serial.print( (addr< 16)?"0":"");
    Serial.println(addr, HEX);
    Serial.println(F("\tSending 0x00 0x00"));

    Wire.beginTransmission(addr);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.requestFrom(addr, 1);
    res = Wire.read();
    Serial.print(F("\tMinibadge returned: "));
    Serial.print(res);
    Serial.print(F(" (Device does "));
    Serial.print(res?"":"not ");
    Serial.println(F("support writing.)"));

    if(res){
      Serial.print(F("\tSending 0x"));
      Serial.print( (command< 16)?"0":"");
      Serial.print(command, HEX);
      Wire.beginTransmission(addr);
      Wire.write(command);
      for(uint8_t i = 0; i < argc; i++){
        Serial.print(F(" 0x"));
        Serial.print( (args[i]< 16)?"0":"");
        Serial.print(args[i], HEX);
        Wire.write(args[i]);
      }
      Serial.println("");
      Wire.endTransmission();
    }

  }else{
    Serial.print(F("Unable to find device at 0x"));
    Serial.print( (addr< 16)?"0":"");
    Serial.println(addr, HEX);
  }
}

// This will send the update brightness minibadge command.
void update_brightness(String com){
  uint8_t target = parse_addr(com);
  if(target != 255){
    uint8_t brightness = 0;
    com = com.substring(com.indexOf(" ")+1);
    com.trim();
    if(com.indexOf(" ") == -1){
      Serial.println(F("Please provide a brightness."));
      return;
    }
    com = com.substring(com.indexOf(" ")+1);
    com.trim();
    com.toLowerCase();
    if(com.indexOf(" ") != -1){
      com = com.substring(0, com.indexOf(" "));
    }
    brightness = (uint8_t)strtol(com.c_str(), nullptr, 10);
    if(brightness > 127){
      Serial.println(F("Please enter a brightness in range (0-127)."));
      return;
    }
    char args[1] = {brightness};
    write_device(target, 0x02, 1, args);
  }
}

// Setup a pin to output a digital io clock pulse at 50% duty cycle and provided hz
void set_clock(String com){
    if(com.indexOf(" ") == -1){
      Serial.println(F("Please provide a clock pin."));
      return;
    }
    com = com.substring(com.indexOf(" ")+1);
    com.trim();
    com.toLowerCase();
    String pin = com;
    if(com.indexOf(" ") != -1){
      pin = com.substring(0, com.indexOf(" "));
    }
    clk_pin = (uint8_t)strtol(pin.c_str(), nullptr, 10);
    if(com.indexOf(" ") == -1){
      Serial.println(F("Please provide the clock speed in Hz."));
      return;
    }
    com = com.substring(com.indexOf(" ")+1);
    com.trim();
    com.toLowerCase();
    if(com.indexOf(" ") != -1){
      com = com.substring(0, com.indexOf(" "));
    }
    clk_hz = (uint8_t)strtol(com.c_str(), nullptr, 10);
    if(clk_hz > 20){
      Serial.println(F("Please enter a speed in range (0-20)."));
      clk_hz = 0;
      return;
    }
    Serial.print(F("Setting pin "));
    Serial.print(clk_pin);
    Serial.print(F(" to clock speed of "));
    Serial.print(clk_hz);
    Serial.println(F(" Hz."));
    pinMode(clk_pin, OUTPUT);
    digitalWrite(clk_pin, LOW);
}

// This is a less verbose read with the provided output of if the minibadge supports writing.
void info_from_device(String com){
  uint8_t target = parse_addr(com);
  if(target != 255){
    read_device(target, true);
  }
}

// A more verbose read from the minibadge showing all I2C transactions.
void read_from_device(String com){
  uint8_t target = parse_addr(com);
  if(target != 255){
    read_device(target, false);
  }
}

// This attempts to connect to every address on the I2C bus. If it responds it will print
// the addresses it finds. Or if there is an error at an address.
void scanner(String com){
  uint8_t cnt = 0;
  for(uint8_t i = 0; i < 128; i++){
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    // 0:success
    // 1:data too long to fit in transmit buffer
    // 2:received NACK on transmit of address
    // 3:received NACK on transmit of data
    // 4:other error
    Wire.beginTransmission(i);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      cnt++;
      Serial.print(F("Device at address 0x"));
      Serial.print( (i< 16)?"0":"");
      Serial.println(i,HEX);
    }else if(error == 4){
      Serial.print(F("Error at address 0x"));
      if (i<16)
        Serial.print("0");
      Serial.println(i,HEX);
    }
  }
  if(cnt == 0){
    Serial.println(F("Unable to find any connected devices."));
  }
}

// This will send the update score command.
void update_score(String com){
  uint8_t target = parse_addr(com);
  if(target != 255){
    uint16_t score = 0;
    com = com.substring(com.indexOf(" ")+1);
    com.trim();
    if(com.indexOf(" ") == -1){
      Serial.println(F("Please provide a score."));
      return;
    }
    com = com.substring(com.indexOf(" ")+1);
    com.trim();
    com.toLowerCase();
    if(com.indexOf(" ") != -1){
      com = com.substring(0, com.indexOf(" "));
    }
    score = (uint16_t)strtoul(com.c_str(), nullptr, 10);
    char args[2] = {(char)(score>>8), (char)(score&0xff)};
    write_device(target, 0x01, 2, args);
  }

}

// This prints the help message.
void print_help(String com){
  Serial.println(F("SAINTCON Minibadge Debugger \t(ADDR can be hex, binary, or decimal)"));
  for(uint8_t i = 0; i < (sizeof(command)/sizeof(char*)); i++){
    Serial.print("\t");
    Serial.print(command[i]);
    char msg[150];
    strcpy_P(msg, (char *)pgm_read_word(&(help[i])));
    Serial.println(msg);
  }
}
