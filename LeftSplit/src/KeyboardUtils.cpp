#include <Arduino.h>
#include <Wire.h>
#include "../inc/Hardware.h"
#include "../inc/KeyboardUtils.h"

// Global variables for matrix scanning
int RowCnt = 0;
int LayerCnt = 0;
static bool serialConnected = false;
static uint16_t pcfState = 0xFFFF; // All pins high initially
bool pcfInitialized = false;
static unsigned long lastErrorTime = 0;
static int errorCount = 0;
static byte detectedPCFAddress = 0; // Store the detected address

// Initialize hardware pins and I²C
void initializeHardware() {
  Serial.println( "Initializing left split hardware..." );
  
  // Initialize I²C communication
  Wire.begin( SDA_PIN, SCL_PIN );
  Wire.setClock( 100000 ); // 100kHz I²C speed
  
  Serial.println( "I²C pins initialized:" );
  Serial.print( "SDA: GPIO" );
  Serial.println( SDA_PIN );
  Serial.print( "SCL: GPIO" );
  Serial.println( SCL_PIN );
  
  // Auto-detect PCF8575 address
  Serial.println( "Auto-detecting PCF8575..." );
  
  // PCF8575 can be at addresses 0x20-0x27
  byte pcfAddresses[] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27 };
  bool pcfFound = false;
  
  for ( int i = 0; i < 8; i++ ) {
    byte testAddress = pcfAddresses[i];
    
    Serial.print( "Testing address 0x" );
    Serial.print( testAddress, HEX );
    Serial.print( "... " );
    
    // Test by trying to write to the device
    Wire.beginTransmission( testAddress );
    Wire.write( 0xFF ); // Low byte
    Wire.write( 0xFF ); // High byte
    byte error = Wire.endTransmission();
    
    if ( error == 0 ) {
      Serial.println( "FOUND!" );
      detectedPCFAddress = testAddress;
      pcfFound = true;
      break;
    } else {
      Serial.println( "not found" );
    }
    
    delay( 10 ); // Small delay between tests
  }
  
  if ( !pcfFound ) {
    Serial.println( "No PCF8575 found at any standard address!" );
    Serial.println( "Check wiring:" );
    Serial.println( "- VCC to 3.3V" );
    Serial.println( "- GND to GND" );
    Serial.println( "- SDA to D2 (GPIO4)" );
    Serial.println( "- SCL to D1 (GPIO5)" );
    pcfInitialized = false;
    return;
  }
  
  Serial.print( "PCF8575 auto-detected at address 0x" );
  Serial.println( detectedPCFAddress, HEX );
  
  // Set initial state
  pcfState = 0xFFFF;
  pcfInitialized = true;
  Serial.println( "PCF8575 initialized successfully" );
}

// Initialize serial communication with right split
void initializeSerial() {
  Serial.println( "Left split ready for communication" );
}

// Write 16-bit value to PCF8575 - return success/failure
bool writePCF8575( uint16_t value ) {
  if ( !pcfInitialized ) {
    return false;
  }
  
  Wire.beginTransmission( detectedPCFAddress ); // Use detected address
  Wire.write( value & 0xFF );        // Low byte
  Wire.write( ( value >> 8 ) & 0xFF ); // High byte
  byte error = Wire.endTransmission();
  
  if ( error != 0 ) {
    errorCount++;
    // Only log errors occasionally to avoid spam
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
  
  Wire.requestFrom( detectedPCFAddress, 2 ); // Use detected address
  
  // Wait a bit for data
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
  // Only log read errors occasionally
  if ( millis() - lastErrorTime > 5000 ) {
    Serial.print( "PCF8575 read error (count: " );
    Serial.print( errorCount );
    Serial.println( ")" );
    lastErrorTime = millis();
  }
  return 0xFFFF; // Return all high on error
}

// Set row pin state with PCF8575
void setRowState( int row, bool state ) {
  if ( !pcfInitialized ) {
    return; // Skip if not initialized
  }
  
  if ( row >= 0 && row < NumRows ) {
    if ( state ) {
      // Set row pin high
      pcfState |= ( 1 << row );
    } else {
      // Set row pin low
      pcfState &= ~( 1 << row );
    }
    
    writePCF8575( pcfState );
  }
}

// Send key press/release to right split via serial
void sendKeyToRight( int keyCode, bool isPress ) {
  Serial.print( isPress ? "P:" : "R:" );
  Serial.println( keyCode );
}

// Check serial connection status
void checkSerialConnection() {
  static unsigned long lastCheck = 0;
  if ( millis() - lastCheck > 30000 ) { // Only log every 30 seconds
    if ( pcfInitialized ) {
      Serial.print( "Left split active - PCF8575 at 0x" );
      Serial.println( detectedPCFAddress, HEX );
    } else {
      Serial.println( "Left split - PCF8575 not initialized" );
    }
    lastCheck = millis();
  }
}

// Get the detected PCF8575 address (for debugging)
byte getPCF8575Address() {
  return detectedPCFAddress;
}
