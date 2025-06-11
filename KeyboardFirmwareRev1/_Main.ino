////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    Firmware for Left Split Part of Keyboard                                                    //
//    Modified from Sanctuary Keyboard Firmware                                                   //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "KeyboardUtils.h"
#include "KeyProcessing.h"

//Setup, once on boot
void setup() {
  Serial.begin( 115200 );
  Serial.println( "Starting keyboard setup..." );
  
  initializeHardware();
  initializeBluetooth();
}

//Main loop
void loop() {  
  checkBluetoothConnection();
  
  if ( Kbd.isConnected() ) {
    scanMatrix();
  }
  
  delay( 10 ); // Scan rate control
}
