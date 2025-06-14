//////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                  //
//    Firmware for the Sanctuary Keyboard - LEFT SPLIT                                             //
//    ESP8266 + PCF8575 I²C Expander Version                                                       //
//    Communicates with right split via serial over USB-C                                          //
//                                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////////////////////

//Include headers for all functionality
#include "inc/Hardware.h"
#include "inc/KeyboardUtils.h"
#include "inc/KeyProcessing.h"

//Setup, once on boot
void setup() {
  Serial.begin( 115200 );
  Serial.println( "Starting left split keyboard setup..." );
  
  initializeHardware();
  initializeSerial();
  
  Serial.println( "Left split initialization complete" );
}

//Main loop
void loop() {
  checkSerialConnection();
  scanMatrix();
  
  delay( 5 ); // Optimized delay for responsiveness
}