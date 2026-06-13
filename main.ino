/**
 * This is a fork of Sanctuary Keyboard Firmware originally designed by Foster Phillips https://linktr.ee/Lego_Rocket
 * @b1ink0 modified the firmware for making it work with a custom split keyboard design.
 */

#include <BleKeyboard.h>
#include "inc/Hardware.h"
#include "inc/KeyboardUtils.h"
#include "inc/KeyProcessing.h"

void setup() {
  Serial.begin( SERIAL_BAUD_RATE );
  Serial.println( "Starting Keyboard setup..." );
  
  initializeHardware();    // Initialize right split GPIO
  initializeI2C();         // Initialize I²C for left split
  initializeBluetooth();   // Initialize BLE keyboard
  
  Serial.println( "Keyboard initialization complete" );
}

//Main loop
void loop() {  
  checkBluetoothConnection();
  checkModeToggle();
  
  if ( getOutputMode() == OUTPUT_MODE_BLE ) {
    // BLE mode - only scan if connected
    if ( Kbd.isConnected() ) {
      scanMatrix();
    }
    // Scan rate.
    delay( 5 );
  } else {
    // Serial mode - always scan regardless of BLE connection
    scanMatrix();
    // Scan rate.
    delay( 2 );
}