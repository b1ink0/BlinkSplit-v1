////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    Firmware for Left Split Part of Keyboard                                                    //
//    Modified from Sanctuary Keyboard Firmware                                                   //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

//Setup, once on boot
void setup() {
  Serial.begin( 115200 );
  Serial.println( "Starting keyboard setup..." );
  
  //Initialize row output pins and set default state
  for ( int i = 0; i < NumRows; i++ ) {
    pinMode( Rows[ i ], OUTPUT );  
    digitalWrite( Rows[ i ], HIGH );  // Set HIGH for pullup configuration
  }

  //Initialize column pins as input with pullup
  for ( int i = 0; i < NumCols; i++ ) {
    pinMode( Cols[ i ], INPUT_PULLUP );
  }
  
  //EEPROM and MAC address setup
  EEPROM.begin( 4 );
  int deviceChose = EEPROM.read( 0 );
  Serial.print( "Device chosen: " );
  Serial.println( deviceChose );
  esp_base_mac_addr_set( &MACAddress[ deviceChose ][ 0 ] );

  //Begin bluetooth keyboard
  Serial.println( "Starting BLE keyboard..." );
  Kbd.begin();
  Serial.println( "Waiting for BLE connection..." );
}

// Global variables for matrix scanning
int RowCnt = 0;
int LayerCnt = 0;

//Main loop
void loop() {  
  //Check if the keyboard is connected, if so, scan the matrix
  if ( Kbd.isConnected() ) {
    static bool connectionLogged = false;
    if ( ! connectionLogged ) {
      Serial.println( "BLE Keyboard connected!" );
      connectionLogged = true;
    }
    
    //Set current row LOW to scan (since using pullup)
    digitalWrite( Rows[ RowCnt ], LOW );
    
    //Small delay for signal to stabilize
    delayMicroseconds( 20 );
    
    //Check columns
    int ColCnt = 0;

    //Repeat until all columns are scanned
    while ( ColCnt <= ( NumCols - 1 )) {      
      //Check state of current position - LOW means pressed with pullup
      if ( digitalRead( Cols[ ColCnt ] ) == LOW && PressedCheck[ LayerCnt ][ RowCnt ][ ColCnt ] == OFF ) {
        switch ( Layer1[ LayerCnt ][ RowCnt ][ ColCnt ] ) {
          //Keep existing special cases
          case 0:
          case 1:
          case 2:
            changeID( Layer1[ LayerCnt ][ RowCnt ][ ColCnt ] );
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
          default:
            if ( Layer1[ LayerCnt ][ RowCnt ][ ColCnt ] >= 32 && Layer1[ LayerCnt ][ RowCnt ][ ColCnt ] <= 126 ) {
              Serial.println( (char) Layer1[ LayerCnt ][ RowCnt ][ ColCnt ] );
            } else {
              Serial.println( Layer1[ LayerCnt ][ RowCnt ][ ColCnt ] );
            }
            Kbd.press( Layer1[ LayerCnt ][ RowCnt ][ ColCnt ] );
        }
        PressedCheck[ LayerCnt ][ RowCnt ][ ColCnt ] = ON;   
      }

      //Check if key was released - HIGH with pullup means released
      else if ( digitalRead( Cols[ ColCnt ] ) == HIGH && PressedCheck[ LayerCnt ][ RowCnt ][ ColCnt ] == ON ) {
        switch ( Layer1[ LayerCnt ][ RowCnt ][ ColCnt ] ) {
          case 1:
          case 2:
          case 0:
            break;
          case FUNCTION_SW:
            PressedCheck[ LayerCnt ][ RowCnt ][ ColCnt ] = OFF;
            if ( LayerCnt > 0 ) {
              LayerCnt--;
            }
            Kbd.releaseAll();
            break;
          case NULL_CON:
            PressedCheck[ LayerCnt ][ RowCnt ][ ColCnt ] = OFF;
            if ( LayerCnt > 0 ){
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
          default:
            if ( Layer1[ LayerCnt ][ RowCnt ][ ColCnt ] >= 32 && Layer1[ LayerCnt ][ RowCnt ][ ColCnt ] <= 126 ) {
              Serial.println( (char) Layer1[ LayerCnt ][ RowCnt ][ ColCnt ] );
            } else {
              Serial.println( Layer1[ LayerCnt ][ RowCnt ][ ColCnt ] );
            }
            Kbd.release( Layer1[ LayerCnt ][ RowCnt ][ ColCnt ] );
        }
        PressedCheck[ LayerCnt ][ RowCnt ][ ColCnt ] = OFF;
      }
      ColCnt++;
    }
    
    //Reset row back to HIGH
    digitalWrite( Rows[ RowCnt ], HIGH );
    
    //Move to next row
    RowCnt++;
    if ( RowCnt >= NumRows ) {
      RowCnt = 0;
    }
  } else {
    static unsigned long lastConnectionCheck = 0;
    if ( millis() - lastConnectionCheck > 5000 ) { // Log every 5 seconds
      Serial.println( "Waiting for BLE connection..." );
      lastConnectionCheck = millis();
    }
  }
  
  //Delay so it's not too fast
  delay( 10 );
}
