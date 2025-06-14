#include <Arduino.h>
#include "../inc/Hardware.h"
#include "../inc/KeyboardUtils.h"
#include "../inc/KeyProcessing.h"

// Main matrix scanning function using PCF8575
void scanMatrix() {
  // Skip scanning if PCF8575 is not initialized
  if ( !pcfInitialized ) {
    return;
  }
  
  // Set current row LOW to scan
  setRowState( RowCnt, LOW );
  
  // Small delay for signal to stabilize
  delayMicroseconds( 20 );
  
  // Read all pins from PCF8575
  uint16_t pinStates = readPCF8575();
  
  // Check if read was successful (not error value)
  if ( pinStates == 0xFFFF ) {
    // Read failed, skip this scan cycle
    setRowState( RowCnt, HIGH );
    RowCnt++;
    if ( RowCnt >= NumRows ) {
      RowCnt = 0;
    }
    return;
  }
  
  // Extract column states (bits 8-13)
  uint8_t colStates = ( pinStates >> 8 ) & 0x3F;
  
  // Scan all columns in current row
  for ( int ColCnt = 0; ColCnt < NumCols; ColCnt++ ) {
    bool keyPressed = !( colStates & ( 1 << ColCnt ) ); // Inverted because of pullup
    bool wasPressed = ( PressedCheck[ LayerCnt ][ RowCnt ][ ColCnt ] == ON );
    
    if ( keyPressed && !wasPressed ) {
      // Key press detected
      handleKeyPress( RowCnt, ColCnt );
    } else if ( !keyPressed && wasPressed ) {
      // Key release detected
      handleKeyRelease( RowCnt, ColCnt );
    }
  }
  
  // Reset row back to HIGH
  setRowState( RowCnt, HIGH );
  
  // Move to next row
  RowCnt++;
  if ( RowCnt >= NumRows ) {
    RowCnt = 0;
  }
}

// Handle key press events
void handleKeyPress( int row, int col ) {
  int keyCode = Layer1[ LayerCnt ][ row ][ col ];
  
  // Process special keys locally or send to right split
  if ( keyCode <= 5 || keyCode >= FUNCTION_SW ) {
    processSpecialKey( keyCode, row, col, true );
  } else {
    // Send regular key to right split
    sendKeyToRight( keyCode, true );
    logKeyAction( keyCode, true );
  }
  
  PressedCheck[ LayerCnt ][ row ][ col ] = ON;
}

// Handle key release events
void handleKeyRelease( int row, int col ) {
  int keyCode = Layer1[ LayerCnt ][ row ][ col ];
  
  // Process special keys locally or send to right split
  if ( keyCode <= 5 || keyCode >= FUNCTION_SW ) {
    processSpecialKey( keyCode, row, col, false );
  } else {
    // Send regular key to right split
    sendKeyToRight( keyCode, false );
    logKeyAction( keyCode, false );
  }
  
  PressedCheck[ LayerCnt ][ row ][ col ] = OFF;
}

// Process special function keys (layer switching, etc.)
void processSpecialKey( int keyCode, int row, int col, bool isPress ) {
  if ( isPress ) {
    switch ( keyCode ) {
      case FUNCTION_SW:
        LayerCnt++;
        Serial.println( "Layer UP" );
        break;
      case NULL_CON:
        // Do nothing for null key
        break;
      default:
        // Send other special keys to right split
        sendKeyToRight( keyCode, true );
        break;
    }
  } else {
    // Key release handling for special keys
    switch ( keyCode ) {
      case FUNCTION_SW:
        if ( LayerCnt > 0 ) {
          LayerCnt--;
        }
        Serial.println( "Layer DOWN" );
        break;
      case NULL_CON:
        // Do nothing for null key
        break;
      default:
        // Send other special keys to right split
        sendKeyToRight( keyCode, false );
        break;
    }
  }
}

// Log key actions for debugging - only for important keys
void logKeyAction( int keyCode, bool isPress ) {
  // Only log printable characters and important keys for debugging
  if ( keyCode >= 32 && keyCode <= 126 ) {
    Serial.print( isPress ? "+" : "-" );
    Serial.println( (char)keyCode );
  }
}
