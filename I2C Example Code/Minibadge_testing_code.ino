//--------------------------------------------------
//    SAINTCON MINIBADGE SAMPLE CODE
//
// This code is a helpful guide for setting up the minibadge
//  to talk to the mainbadge following the minibadge 2.0 spec.
// You can use the Minibadge_debugger.ino code sample to
//  test your minibadge and make sure it is compliant.
//
//
// Author: Michael Julander ( @Sodium_Hydrogen, MikeJ.Tech )
//
//--------------------------------------------------


// This provides the i2c interface. Check with your board definitions to know what pins are used for SDA and SCL.
#include "Wire.h"



//--------------------------------------------------
// Config Values
//--------------------------------------------------

// This is the 7 bit address that uniquely identifies this chip to the badge.
// Make sure you are not using the same address as another minibadge or neither
// minibadges will work.
#define ADDR 0x42


//--------Variables---------------------------------
// You can change these from setup or loop to make the minibadge more interactive.

// If you would like to let the main badge write the minibadge this should be 1.
// Otherwise to disable write support set write_support to 0.
uint8_t write_support = 1;

// read_action is used for telling the read interupt what to respond with.
//  0 is do nothing.
//  1 is button press.
//  2 is text message. If you use this then you must also set message_length and message.
uint8_t read_action = 2;

// This is the length of the text message the badge will send if read_action is set to 2.
// The max length is 255. Anything larger and it cannot be sent in one byte and will
// also overflow the variable.
uint8_t message_length = 12;

// This is the message the minibadge will send when a text message read event happens.
char* message = "Hello World!";




//--------------------------------------------------
// Interupt Functions
//--------------------------------------------------

// Any variable being writen to inside an interupt Ie. request() or recieve() should be volatile.
// These are default values and will change as the badge talks to the minibadge.
volatile uint8_t reading_state = 0; // this should only be set to 0 or 1 in the recieve function.
volatile uint32_t score = 0;
volatile uint8_t brightness = 100;


//--------Read Event--------------------------------
void request(){

  // If the badge is writing then reading_state will be 0 from the recieve function.
  // This tells the badge wether this minibadge supports write events.
  if(reading_state == 0){
    Wire.write(write_support);

  // If reading_state is set to 1 then the badge is beginning the read request.
  }else if(reading_state == 1 ){

    // Send the read_action to the badge.
    Wire.write(read_action);

    // If it is 2 for a text message let this function know next read event will need to send the
    // message length by setting reading_state to 2.
    if(read_action == 2){
      reading_state = 2;

    // If read_action is anything else then set reading_state to 4 to indicate the minibadge
    // should not respond with anything else till the next init sequence from the badge.
    }else{
      reading_state = 4;
    }

    // It is a good idea to set the read_action to zero after any value is read.
    // This will prevent the minibadge from "spamming" its message if the main badge
    // is not checking for that.
    read_action = 0;

  // If reading_state is 2 then the minibadge should respond with the text message length and
  // advance reading_state to 3 to let the minibadge know next read should be the text message.
  }else if(reading_state == 2){
    Wire.write(message_length);
    reading_state = 3;

  // If reading_state is 3 then the minibadge should send the text message one byte at a time.
  // Once it is done it should set reading_state to 4 to let the minibadge know to do nothing
  // until the badge inits communication again.
  }else if(reading_state == 3){
    for(uint8_t i = 0; i < message_length; i++){
      Wire.write(message[i]);
    }
    reading_state = 4;
  }
}


//--------Write Event-------------------------------
// This is the write event. This function will be called whenever the main badge
// writes to this minibadge.
// byteCount will store the number of bytes sent in the write event.
void recieve(uint8_t byteCount){
  // This will read the first byte in the buffer into the byteOne variable.
  uint8_t byteOne = Wire.read();

  // If the first byte read is 0x00 then we know the minibadge initiation sequence was started.
  if(byteOne == 0x00){
    // This reads the next byte and will let us know if the badge wants to read
    // or write to the minibadge.
    reading_state = Wire.read();

  // If the first byte is anythong other than 0x00 than there is a write event
  // and the minibadge will only continue reading if write_support is enabled.
  }else if(write_support == 1){

    // 0x01 is a score update. The score is two bytes long and in big endian
    // ex. if the sent score is 0x02 0xFF then the value saved to score would be
    //  0x02FF or 767.
    if(byteOne == 0x01){
      score = (((uint32_t)Wire.read()) << 8) + (uint32_t)Wire.read();

    // 0x02 is for a brightness update. The brightness range should be 0-127.
    }else if(byteOne == 0x02){
      brightness = Wire.read();
    }
  }
}




//--------------------------------------------------
// Main Arduino Code
//--------------------------------------------------

void setup() {
  // These 3 lines setup the Wire library to work properly for read/write requests as well
  // as setting the address it will run on.
  Wire.onReceive(recieve);
  Wire.onRequest(request);
  Wire.begin(ADDR);

  // your setup here
}


void loop() {
  // Here is where you can make the badge interactive by changing the message
  // or responding to any write events from the main badge.

  // your code here
}
