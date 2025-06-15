#include <Arduino.h>
#include <BleKeyboard.h>
#include <EEPROM.h>
#include <Wire.h>
#include "../inc/Hardware.h"
#include "../inc/KeyboardUtils.h"

//Setup keyboard - moved from main file
BleKeyboard Kbd( "Sanctuary Keyboard", "Foster Phillips", 100 );

// Code to "store" devices, so that the keyboard can switch connections on the fly
const int maxdevice = 3;
uint8_t MACAddress[maxdevice][ 6 ] = 
{
  {0x35, 0xAF, 0xA4, 0x07, 0x0B, 0x66},
  {0x31, 0xAE, 0xAA, 0x47, 0x0D, 0x61},
  {0x31, 0xAE, 0xAC, 0x42, 0x0A, 0x31}
};

void changeID( int DevNum ) {
    if( DevNum < maxdevice )
    {
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

// PCF8575 variables for left split
bool pcfInitialized = false;
bool leftSplitConnected = false;
static uint16_t pcfState = 0xFFFF;
static byte detectedPCFAddress = 0;
static unsigned long lastErrorTime = 0;
static int errorCount = 0;

// Initialize hardware pins, I²C, and EEPROM
void initializeHardware() {
  Serial.println( "Initializing right split hardware..." );
  
  // Initialize right split - row output pins
  for ( int i = 0; i < NumRows; i++ ) {
    pinMode( Rows[ i ], OUTPUT );  
    digitalWrite( Rows[ i ], HIGH );  // Set HIGH for pullup configuration
  }

  // Initialize right split - column pins as input with pullup
  for ( int i = 0; i < NumCols; i++ ) {
    pinMode( Cols[ i ], INPUT_PULLUP );
  }
  
  // EEPROM and MAC address setup
  EEPROM.begin( 4 );
  int deviceChose = EEPROM.read( 0 );
  Serial.print( "Device chosen: " );
  Serial.println( deviceChose );
  esp_base_mac_addr_set( &MACAddress[ deviceChose ][ 0 ] );
  
  Serial.println( "Right split GPIO initialized" );
}

// Initialize I²C for left split communication
void initializeI2C() {
  Serial.println( "Initializing I²C for left split..." );
  
  // Initialize I²C communication for left split
  Wire.begin( SDA_PIN, SCL_PIN );
  Wire.setClock( 100000 ); // 100kHz I²C speed
  
  Serial.println( "I²C pins initialized:" );
  Serial.print( "SDA: GPIO" );
  Serial.println( SDA_PIN );
  Serial.print( "SCL: GPIO" );
  Serial.println( SCL_PIN );
  
  // Auto-detect PCF8575 address
  Serial.println( "Auto-detecting left split PCF8575..." );
  
  byte pcfAddresses[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 };
  bool pcfFound = false;
  
  for ( int i = 0; i < 8; i++ ) {
    byte testAddress = pcfAddresses[i];
    
    Serial.print( "Testing address 0x" );
    Serial.print( testAddress, HEX );
    Serial.print( "... " );
    
    Wire.beginTransmission( testAddress );
    Wire.write( 0xFF );
    Wire.write( 0xFF );
    byte error = Wire.endTransmission();
    
    if ( error == 0 ) {
      Serial.println( "FOUND!" );
      detectedPCFAddress = testAddress;
      pcfFound = true;
      break;
    } else {
      Serial.println( "not found" );
    }
    
    delay( 10 );
  }
  
  if ( !pcfFound ) {
    Serial.println( "Left split PCF8575 not found - will operate right split only" );
    pcfInitialized = false;
    leftSplitConnected = false;
    return;
  }
  
  Serial.print( "Left split PCF8575 detected at address 0x" );
  Serial.println( detectedPCFAddress, HEX );
  
  pcfState = 0xFFFF;
  pcfInitialized = true;
  leftSplitConnected = true;
  Serial.println( "Left split initialized successfully" );
}

// Initialize Bluetooth keyboard
void initializeBluetooth() {
  Serial.println( "Starting BLE keyboard..." );
  Kbd.begin();
  Serial.println( "Waiting for BLE connection..." );
}

// Set row pin state for right split (direct GPIO)
void setRowState( int row, bool state ) {
  if ( row >= 0 && row < NumRows ) {
    digitalWrite( Rows[ row ], state ? HIGH : LOW );
  }
}

// Set row pin state for left split (via PCF8575)
void setLeftRowState( int row, bool state ) {
  if ( !pcfInitialized || row < 0 || row >= NumRows ) {
    return;
  }
  
  if ( state ) {
    // Set row pin high
    pcfState |= ( 1 << row );
  } else {
    // Set row pin low
    pcfState &= ~( 1 << row );
  }
  
  writePCF8575( pcfState );
}

// Write 16-bit value to PCF8575
bool writePCF8575( uint16_t value ) {
  if ( !pcfInitialized ) {
    return false;
  }
  
  Wire.beginTransmission( detectedPCFAddress );
  Wire.write( value & 0xFF );
  Wire.write( ( value >> 8 ) & 0xFF );
  byte error = Wire.endTransmission();
  
  if ( error != 0 ) {
    errorCount++;
    if ( millis() - lastErrorTime > 5000 ) {
      Serial.print( "PCF8575 write error (count: " );
      Serial.print( errorCount );
      Serial.println( ")" );
      lastErrorTime = millis();
    }
    return false;
  }
  
  return true;
}

// Read 16-bit value from PCF8575
uint16_t readPCF8575() {
  if ( !pcfInitialized ) {
    return 0xFFFF;
  }
  
  Wire.requestFrom( detectedPCFAddress, 2 );
  
  int timeout = 0;
  while ( Wire.available() < 2 && timeout < 100 ) {
    delayMicroseconds( 10 );
    timeout++;
  }
  
  if ( Wire.available() >= 2 ) {
    uint8_t lowByte = Wire.read();
    uint8_t highByte = Wire.read();
    return ( highByte << 8 ) | lowByte;
  }
  
  errorCount++;
  if ( millis() - lastErrorTime > 5000 ) {
    Serial.print( "PCF8575 read error (count: " );
    Serial.print( errorCount );
    Serial.println( ")" );
    lastErrorTime = millis();
  }
  return 0xFFFF;
}

// Check Bluetooth connection status
void checkBluetoothConnection() {
  if ( Kbd.isConnected() ) {
    if ( !connectionLogged ) {
      Serial.println( "BLE Keyboard connected!" );
      if ( leftSplitConnected ) {
        Serial.println( "Both splits active" );
      } else {
        Serial.println( "Right split only - left split not detected" );
      }
      connectionLogged = true;
    }
  } else {
    connectionLogged = false;
    static unsigned long lastConnectionCheck = 0;
    if ( millis() - lastConnectionCheck > 5000 ) {
      Serial.println( "Waiting for BLE connection..." );
      lastConnectionCheck = millis();
    }
  }
}

// Get the detected PCF8575 address
byte getPCF8575Address() {
  return detectedPCFAddress;
}
