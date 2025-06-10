////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    Firmware for Custom Keyboard                                                               //
//    Modified from Sanctuary Keyboard Firmware                                                  //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

//Setup, once on boot
void setup() {
  Serial.begin(115200);
  delay(1000); // Give serial time to initialize
  
  Serial.println("Starting keyboard setup...");
  
  //Initialize row output pins and set default state
  for(int i = 0; i < NumRows; i++)
  {
    pinMode(Rows[i],OUTPUT);  
    digitalWrite(Rows[i],HIGH);  // Set HIGH for pullup configuration
    Serial.print("Row ");
    Serial.print(i);
    Serial.print(" on pin ");
    Serial.println(Rows[i]);
  }

  //Initialize column pins as input with pullup
  for(int i = 0; i < NumCols; i++)
  {
    pinMode(Cols[i],INPUT_PULLUP);
    Serial.print("Col ");
    Serial.print(i);
    Serial.print(" on pin ");
    Serial.println(Cols[i]);
  }
  
  // Print the key layout for debugging
  Serial.println("Key Layout (Row x Col):");
  for(int r = 0; r < NumRows; r++) {
    Serial.print("Row ");
    Serial.print(r);
    Serial.print(": ");
    for(int c = 0; c < NumCols; c++) {
      Serial.print(Layer1[0][r][c]);
      Serial.print("\t");
    }
    Serial.println();
  }
  
  //EEPROM and MAC address setup - keep existing functionality
  EEPROM.begin(4);
  int deviceChose = EEPROM.read(0);
  Serial.print("Device chosen: ");
  Serial.println(deviceChose);
  esp_base_mac_addr_set(&MACAddress[deviceChose][0]);

  //Begin bluetooth keyboard
  Serial.println("Starting BLE keyboard...");
  Kbd.begin();
  
  Serial.println("Keyboard setup complete!");
  Serial.println("Waiting for BLE connection...");
}

int RowCnt = 0;
int LayerCnt = 0;

//Main loop
void loop() {
  // Add debugging every few seconds to show matrix scanning is working
  static unsigned long lastDebug = 0;
  static bool debugMode = false;
  
  if (millis() - lastDebug > 10000) { // Every 10 seconds
    debugMode = !debugMode;
    Serial.print("Debug mode: ");
    Serial.println(debugMode ? "ON" : "OFF");
    lastDebug = millis();
  }
  
  //Check if the keyboard is connected, if so, scan the matrix
  if(Kbd.isConnected())
  {
    static bool connectionLogged = false;
    if (!connectionLogged) {
      Serial.println("BLE Keyboard connected!");
      connectionLogged = true;
    }
    
    //Set current row LOW to scan (since using pullup)
    digitalWrite(Rows[RowCnt],LOW);
    
    //Small delay for signal to stabilize
    delayMicroseconds(30);
    
    //Check columns
    int ColCnt = 0;

    //Repeat until all columns are scanned
    while(ColCnt <= (NumCols - 1))
    {
      // Show raw pin states if debug mode is on
      if(debugMode && RowCnt == 0 && ColCnt == 0) {
        // Serial.print("Scanning R");
        // Serial.print(RowCnt);
        // Serial.print(" (pin ");
        // Serial.print(Rows[RowCnt]);
        // Serial.print("=LOW) - Col states: ");
        // for(int i = 0; i < NumCols; i++) {
        //   Serial.print("C");
        //   Serial.print(i);
        //   Serial.print(":");
        //   Serial.print(digitalRead(Cols[i]));
        //   Serial.print(" ");
        // }
        // Serial.println();
      }
      
      //Check state of current position - LOW means pressed with pullup
      if(digitalRead(Cols[ColCnt]) == LOW && PressedCheck[LayerCnt][RowCnt][ColCnt] == OFF)
      {
        Serial.print("Key pressed at Row:");
        Serial.print(RowCnt);
        Serial.print(" Col:");
        Serial.print(ColCnt);
        Serial.print(" Key code:");
        Serial.println(Layer1[LayerCnt][RowCnt][ColCnt]);
        
        //Use existing switch structure for special keys
        switch(Layer1[LayerCnt][RowCnt][ColCnt])
        {
          //Keep existing special cases
          case 0:
          case 1:
          case 2:
            Serial.println("Changing device ID");
            changeID(Layer1[LayerCnt][RowCnt][ColCnt]);
            break;
          case 5:
            Serial.println("Restarting ESP");
            ESP.restart();
            break;
          case FUNCTION_SW:
            Serial.println("Function key pressed");
            Kbd.releaseAll();
            LayerCnt++;
            break;
          case NULL_CON:
            Serial.println("NULL key - no action");
            break;
          case NEXT:
            Serial.println("Next track");
            Kbd.press(KEY_MEDIA_NEXT_TRACK);
            Kbd.releaseAll();
            break;
          case PREV:
            Serial.println("Previous track");
            Kbd.press(KEY_MEDIA_PREVIOUS_TRACK);
            break;
          default:
            Serial.print("Pressing key: ");
            if (Layer1[LayerCnt][RowCnt][ColCnt] >= 32 && Layer1[LayerCnt][RowCnt][ColCnt] <= 126) {
              Serial.println((char)Layer1[LayerCnt][RowCnt][ColCnt]);
            } else {
              Serial.println(Layer1[LayerCnt][RowCnt][ColCnt]);
            }
            Kbd.press(Layer1[LayerCnt][RowCnt][ColCnt]);
        }
        PressedCheck[LayerCnt][RowCnt][ColCnt] = ON;   
      }

      //Check if key was released - HIGH with pullup means released
      else if(digitalRead(Cols[ColCnt]) == HIGH && PressedCheck[LayerCnt][RowCnt][ColCnt] == ON)
      {
        Serial.print("Key released at Row:");
        Serial.print(RowCnt);
        Serial.print(" Col:");
        Serial.println(ColCnt);
        
        //Use existing switch structure for key releases
        switch(Layer1[LayerCnt][RowCnt][ColCnt])
        {
          case 1:
          case 2:
          case 0:
            break;
          case FUNCTION_SW:
            Serial.println("Function key released");
            PressedCheck[LayerCnt][RowCnt][ColCnt] = OFF;
            if(LayerCnt > 0)
            {
              LayerCnt--;
            }
            Kbd.releaseAll();
            break;
          case NULL_CON:
            PressedCheck[LayerCnt][RowCnt][ColCnt] = OFF;
            if(LayerCnt > 0)
            {
              LayerCnt--;
            }
            Kbd.releaseAll();
            break;
          case NEXT:
            Kbd.release(KEY_MEDIA_NEXT_TRACK);
            break;
          case PREV:
            Kbd.release(KEY_MEDIA_PREVIOUS_TRACK);
            break;
          default:
            Serial.print("Releasing key: ");
            if (Layer1[LayerCnt][RowCnt][ColCnt] >= 32 && Layer1[LayerCnt][RowCnt][ColCnt] <= 126) {
              Serial.println((char)Layer1[LayerCnt][RowCnt][ColCnt]);
            } else {
              Serial.println(Layer1[LayerCnt][RowCnt][ColCnt]);
            }
            Kbd.release(Layer1[LayerCnt][RowCnt][ColCnt]);
        }
        PressedCheck[LayerCnt][RowCnt][ColCnt] = OFF;
      }
      ColCnt++;
    }
    
    //Reset row back to HIGH
    digitalWrite(Rows[RowCnt],HIGH);
    
    //Move to next row
    RowCnt++;
    if(RowCnt >= NumRows)
    {
      RowCnt = 0;
    }
  }
  else 
  {
    static unsigned long lastConnectionCheck = 0;
    if (millis() - lastConnectionCheck > 5000) { // Log every 5 seconds
      Serial.println("Waiting for BLE connection...");
      lastConnectionCheck = millis();
    }
  }
  
  //Delay so it's not too fast
  delay(10);
}
