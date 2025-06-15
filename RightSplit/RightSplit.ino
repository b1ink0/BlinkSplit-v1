//////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                  //
//    Firmware for the Sanctuary Keyboard                                                           //
//    Designed by: Foster Phillips, Lego_Rocket on many social media                                //
//    https://linktr.ee/Lego_Rocket                                                                 //
//    Primarily used to run the firmware behind the Sanctuary                                       //
//    Keyboard and kits to be sold at one point                                                     //
//    Firmware will be open source - hardware will be closed source                                 //
//    Main file - contains only setup() and loop() functions                                        //
//                                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////////////////////

//Include headers for all functionality
#include <BleKeyboard.h>
#include "inc/Hardware.h"
#include "inc/KeyboardUtils.h"
#include "inc/KeyProcessing.h"

//Setup, once on boot
void setup() {
  Serial.begin( 115200 );
  Serial.println( "Starting Keyboard setup..." );
  
  initializeHardware();    // Initialize right split GPIO
  initializeI2C();         // Initialize I²C for left split
  initializeBluetooth();   // Initialize BLE keyboard
  
  Serial.println( "Keyboard initialization complete" );
}

//Main loop
void loop() {  
  checkBluetoothConnection();
  
  if ( Kbd.isConnected() ) {
    scanMatrix(); // Scans both right and left splits
  }
  
  delay( 5 ); // Optimized scan rate
}