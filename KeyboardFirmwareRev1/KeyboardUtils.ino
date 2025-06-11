#include <BleKeyboard.h>
#include <EEPROM.h>
#include "Hardware.h"
#include "KeyboardUtils.h"

// Global variables for matrix scanning
int RowCnt = 0;
int LayerCnt = 0;
static bool connectionLogged = false;

// Initialize hardware pins and EEPROM
void initializeHardware() {
  // Initialize row output pins and set default state
  for ( int i = 0; i < NumRows; i++ ) {
    pinMode( Rows[ i ], OUTPUT );  
    digitalWrite( Rows[ i ], HIGH );  // Set HIGH for pullup configuration
  }

  // Initialize column pins as input with pullup
  for ( int i = 0; i < NumCols; i++ ) {
    pinMode( Cols[ i ], INPUT_PULLUP );
  }
  
  // EEPROM and MAC address setup
  EEPROM.begin( 4 );
  int deviceChose = EEPROM.read( 0 );
  Serial.print( "Device chosen: " );
  Serial.println( deviceChose );
  esp_base_mac_addr_set( &MACAddress[ deviceChose ][ 0 ] );
}

// Initialize Bluetooth keyboard
void initializeBluetooth() {
  Serial.println( "Starting BLE keyboard..." );
  Kbd.begin();
  Serial.println( "Waiting for BLE connection..." );
}

// Set row pin state with bounds checking
void setRowState( int row, bool state ) {
  if ( row >= 0 && row < NumRows ) {
    digitalWrite( Rows[ row ], state ? HIGH : LOW );
  }
}

// Check Bluetooth connection status
void checkBluetoothConnection() {
  if ( Kbd.isConnected() ) {
    if ( !connectionLogged ) {
      Serial.println( "BLE Keyboard connected!" );
      connectionLogged = true;
    }
  } else {
    connectionLogged = false;
    static unsigned long lastConnectionCheck = 0;
    if ( millis() - lastConnectionCheck > 5000 ) { // Log every 5 seconds
      Serial.println( "Waiting for BLE connection..." );
      lastConnectionCheck = millis();
    }
  }
}
