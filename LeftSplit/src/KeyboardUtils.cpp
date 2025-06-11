#include <Arduino.h>
#include <BleKeyboard.h>
#include <EEPROM.h>
#include "../inc/Hardware.h"
#include "../inc/KeyboardUtils.h"

//Setup keyboard - moved from main file
BleKeyboard Kbd( "My Keyboard", "YourName", 100 );

// Code to "store" devices, so that the keyboard can switch connections on the fly
// Adapted from : https://github.com/Cemu0/ESP32_USBHOST_TO_BLE_KEYBOARD/blob/main/src/USBHIDBootBLEKbd.cpp
// Primarily stores the selected MAC address in EEPROM storage
const int maxdevice = 3;
uint8_t MACAddress[maxdevice][ 6 ] = 
{
  {0x35, 0xAF, 0xA4, 0x07, 0x0B, 0x66},
  {0x31, 0xAE, 0xAA, 0x47, 0x0D, 0x61},
  {0x31, 0xAE, 0xAC, 0x42, 0x0A, 0x31}
};

// Basically just change the selected ID and reset - MAC address can only be changed before the keyboard start, so write to store selection, until changed again
// Take in device number, and set the EEPROM to the selected - selects what address to shift to, instead of iterating to that address
void changeID( int DevNum ) {
    //Make sure the selection is valid
    if( DevNum < maxdevice )
    {
      //Write and commit to storage, reset ESP 32
      EEPROM.write(0,DevNum);
      EEPROM.commit();
      esp_sleep_enable_timer_wakeup( 1 );
      esp_deep_sleep_start(); 
    }
}

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
