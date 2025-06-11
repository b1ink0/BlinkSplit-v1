#include <BleKeyboard.h>
#include "Hardware.h"
#include "KeyboardUtils.h"
#include "KeyProcessing.h"

// Main matrix scanning function
void scanMatrix() {
  // Set current row LOW to scan (since using pullup)
  setRowState( RowCnt, LOW );
  
  // Small delay for signal to stabilize
  delayMicroseconds( 20 );
  
  // Scan all columns in current row
  for ( int ColCnt = 0; ColCnt < NumCols; ColCnt++ ) {
    bool keyPressed = ( digitalRead( Cols[ ColCnt ] ) == LOW );
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
  
  // Process special keys first
  if ( keyCode <= 5 || keyCode >= FUNCTION_SW ) {
    processSpecialKey( keyCode, row, col, true );
  } else {
    // Regular key
    processRegularKey( keyCode, true );
    logKeyAction( keyCode, true );
  }
  
  PressedCheck[ LayerCnt ][ row ][ col ] = ON;
}

// Handle key release events
void handleKeyRelease( int row, int col ) {
  int keyCode = Layer1[ LayerCnt ][ row ][ col ];
  
  // Process special keys first
  if ( keyCode <= 5 || keyCode >= FUNCTION_SW ) {
    processSpecialKey( keyCode, row, col, false );
  } else {
    // Regular key
    processRegularKey( keyCode, false );
    logKeyAction( keyCode, false );
  }
  
  PressedCheck[ LayerCnt ][ row ][ col ] = OFF;
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
        Kbd.releaseAll();
        LayerCnt++;
        break;
      case NULL_CON:
        // Do nothing for null key
        break;
      case NEXT:
        Kbd.press( KEY_MEDIA_NEXT_TRACK );
        Kbd.releaseAll();
        break;
      case PREV:
        Kbd.press( KEY_MEDIA_PREVIOUS_TRACK );
        break;
    }
  } else {
    // Key release handling for special keys
    switch ( keyCode ) {
      case 0:
      case 1:
      case 2:
        // No action needed on release for device change keys
        break;
      case FUNCTION_SW:
        if ( LayerCnt > 0 ) {
          LayerCnt--;
        }
        Kbd.releaseAll();
        break;
      case NULL_CON:
        if ( LayerCnt > 0 ) {
          LayerCnt--;
        }
        Kbd.releaseAll();
        break;
      case NEXT:
        Kbd.release( KEY_MEDIA_NEXT_TRACK );
        break;
      case PREV:
        Kbd.release( KEY_MEDIA_PREVIOUS_TRACK );
        break;
    }
  }
}

// Process regular keys (letters, numbers, modifiers)
void processRegularKey( int keyCode, bool isPress ) {
  if ( isPress ) {
    Kbd.press( keyCode );
  } else {
    Kbd.release( keyCode );
  }
}

// Log key actions for debugging
void logKeyAction( int keyCode, bool isPress ) {
  const char* action = isPress ? "PRESS" : "RELEASE";
  
  if ( keyCode >= 32 && keyCode <= 126 ) {
    Serial.print( action );
    Serial.print( ": '" );
    Serial.print( (char)keyCode );
    Serial.println( "'" );
  } else {
    Serial.print( action );
    Serial.print( ": " );
    Serial.println( keyCode );
  }
}
