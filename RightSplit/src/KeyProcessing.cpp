#include <Arduino.h>
#include <BleKeyboard.h>
#include "../inc/Hardware.h"
#include "../inc/KeyboardUtils.h"
#include "../inc/KeyProcessing.h"

// Main matrix scanning function - scans both splits
void scanMatrix() {
  // Scan right split (direct GPIO)
  scanRightMatrix();
  
  // Scan left split (via PCF8575) if connected
  if ( leftSplitConnected ) {
    scanLeftMatrix();
  }
}

// Scan right split matrix (direct GPIO)
void scanRightMatrix() {
  // Set current row LOW to scan
  setRowState( RowCnt, LOW );
  
  delayMicroseconds( 5 );
  
  // Scan all columns in current row
  for ( int ColCnt = 0; ColCnt < NumCols; ColCnt++ ) {
    bool keyPressed = ( digitalRead( Cols[ ColCnt ] ) == LOW );
    bool wasPressed = ( RightPressedCheck[ LayerCnt ][ RowCnt ][ ColCnt ] == ON );
    
    if ( keyPressed && !wasPressed ) {
      handleRightKeyPress( RowCnt, ColCnt );
    } else if ( !keyPressed && wasPressed ) {
      handleRightKeyRelease( RowCnt, ColCnt );
    }
  }
  
  // Reset row back to HIGH
  setRowState( RowCnt, HIGH );
}

// Scan left split matrix (via PCF8575)
void scanLeftMatrix() {
  if ( !pcfInitialized ) {
    return;
  }
  
  // Set current row LOW to scan
  setLeftRowState( RowCnt, LOW );
  
  delayMicroseconds( 5 );
  
  // Read all pins from PCF8575
  uint16_t pinStates = readPCF8575();
  
  if ( pinStates == 0xFFFF ) {
    setLeftRowState( RowCnt, HIGH );
    return;
  }
  
  // Extract column states (bits 8-13)
  uint8_t colStates = ( pinStates >> 8 ) & 0x3F;
  
  // Scan all columns in current row
  for ( int ColCnt = 0; ColCnt < LeftNumCols; ColCnt++ ) {
    bool keyPressed = !( colStates & ( 1 << ColCnt ) ); // Inverted because of pullup
    bool wasPressed = ( LeftPressedCheck[ LayerCnt ][ RowCnt ][ ColCnt ] == ON );
    
    if ( keyPressed && !wasPressed ) {
      handleLeftKeyPress( RowCnt, ColCnt );
    } else if ( !keyPressed && wasPressed ) {
      handleLeftKeyRelease( RowCnt, ColCnt );
    }
  }
  
  // Reset row back to HIGH
  setLeftRowState( RowCnt, HIGH );
  
  // Move to next row (shared counter for both splits)
  RowCnt++;
  if ( RowCnt >= NumRows ) {
    RowCnt = 0;
  }
}

// Handle right split key press
void handleRightKeyPress( int row, int col ) {
  int keyCode = RightLayer1[ LayerCnt ][ row ][ col ];
  
  if ( keyCode <= 5 || keyCode >= FUNCTION_SW ) {
    processSpecialKey( keyCode, row, col, true );
  } else {
    processRegularKey( keyCode, true );
    logKeyAction( keyCode, true, "RIGHT" );
  }
  
  RightPressedCheck[ LayerCnt ][ row ][ col ] = ON;
}

// Handle right split key release
void handleRightKeyRelease( int row, int col ) {
  int keyCode = RightLayer1[ LayerCnt ][ row ][ col ];
  
  if ( keyCode <= 5 || keyCode >= FUNCTION_SW ) {
    processSpecialKey( keyCode, row, col, false );
  } else {
    processRegularKey( keyCode, false );
    logKeyAction( keyCode, false, "RIGHT" );
  }
  
  RightPressedCheck[ LayerCnt ][ row ][ col ] = OFF;
}

// Handle left split key press
void handleLeftKeyPress( int row, int col ) {
  int keyCode = LeftLayer1[ LayerCnt ][ row ][ col ];
  
  if ( keyCode <= 5 || keyCode >= FUNCTION_SW ) {
    processSpecialKey( keyCode, row, col, true );
  } else {
    processRegularKey( keyCode, true );
    logKeyAction( keyCode, true, "LEFT" );
  }
  
  LeftPressedCheck[ LayerCnt ][ row ][ col ] = ON;
}

// Handle left split key release
void handleLeftKeyRelease( int row, int col ) {
  int keyCode = LeftLayer1[ LayerCnt ][ row ][ col ];
  
  if ( keyCode <= 5 || keyCode >= FUNCTION_SW ) {
    processSpecialKey( keyCode, row, col, false );
  } else {
    processRegularKey( keyCode, false );
    logKeyAction( keyCode, false, "LEFT" );
  }
  
  LeftPressedCheck[ LayerCnt ][ row ][ col ] = OFF;
}

// Process regular keys
void processRegularKey( int keyCode, bool isPress ) {
  if ( getOutputMode() == OUTPUT_MODE_BLE ) {
    // Send via BLE
    if ( isPress ) {
      Kbd.press( keyCode );
    } else {
      Kbd.release( keyCode );
    }
  } else {
    // Send via Serial
    sendKeyViaSerial( keyCode, isPress );
  }
}

// Process special function keys
void processSpecialKey( int keyCode, int row, int col, bool isPress ) {
  if ( isPress ) {
    switch ( keyCode ) {
      case 0:
      case 1:
      case 2:
        changeID( keyCode );
        break;
      case 5:
        ESP.restart();
        break;
      case FUNCTION_SW:
        if ( getOutputMode() == OUTPUT_MODE_BLE ) {
          Kbd.releaseAll();
        }
        LayerCnt++;
        Serial.println( "Layer UP" );
        break;
      case NULL_CON:
        // Do nothing for null key
        break;
      case NEXT:
      case PREV:
        if ( getOutputMode() == OUTPUT_MODE_BLE ) {
          Kbd.press( keyCode == NEXT ? KEY_MEDIA_NEXT_TRACK : KEY_MEDIA_PREVIOUS_TRACK );
          Kbd.releaseAll();
        } else {
          sendSpecialKeyViaSerial( keyCode, isPress );
        }
        break;
    }
  } else {
    switch ( keyCode ) {
      case 0:
      case 1:
      case 2:
        break;
      case FUNCTION_SW:
        if ( LayerCnt > 0 ) {
          LayerCnt--;
        }
        if ( getOutputMode() == OUTPUT_MODE_BLE ) {
          Kbd.releaseAll();
        }
        Serial.println( "Layer DOWN" );
        break;
      case NULL_CON:
        break;
      case NEXT:
      case PREV:
        if ( getOutputMode() == OUTPUT_MODE_BLE ) {
          Kbd.release( keyCode == NEXT ? KEY_MEDIA_NEXT_TRACK : KEY_MEDIA_PREVIOUS_TRACK );
        } else {
          sendSpecialKeyViaSerial( keyCode, isPress );
        }
        break;
    }
  }
}

// Log key actions for debugging
void logKeyAction( int keyCode, bool isPress, const char* split ) {
  const char* action = isPress ? "PRESS" : "RELEASE";
  
  if ( keyCode >= 32 && keyCode <= 126 ) {
    Serial.print( split );
    Serial.print( " " );
    Serial.print( action );
    Serial.print( ": '" );
    Serial.print( (char)keyCode );
    Serial.println( "'" );
  } else {
    Serial.print( split );
    Serial.print( " " );
    Serial.print( action );
    Serial.print( ": " );
    Serial.println( keyCode );
  }
}

// Send regular key via Serial
void sendKeyViaSerial( int keyCode, bool isPress ) {
  Serial.print( "KEY:" );
  Serial.print( keyCode );
  Serial.print( ":" );
  Serial.println( isPress ? "PRESS" : "RELEASE" );
}

// Send special key via Serial
void sendSpecialKeyViaSerial( int keyCode, bool isPress ) {
  Serial.print( "SPECIAL:" );
  Serial.print( keyCode );
  Serial.print( ":" );
  Serial.println( isPress ? "PRESS" : "RELEASE" );
}
