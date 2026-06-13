/**
 * BlinkSplit v1 firmware for a custom hand wired split keyboard build.
 */

#include <BleKeyboard.h>
#include "inc/Hardware.h"
#include "inc/KeyboardUtils.h"
#include "inc/KeyProcessing.h"

void setup() {
  Serial.begin( SERIAL_BAUD_RATE );
  Serial.println( "Starting BlinkSplit v1 setup..." );
  
  initializeHardware();    // Initialize right split GPIO
  initializeI2C();         // Initialize I²C for left split
  initializeBluetooth();   // Initialize BLE keyboard
  
  Serial.println( "BlinkSplit v1 initialization complete" );
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