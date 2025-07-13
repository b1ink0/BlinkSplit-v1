#ifndef KEYBOARD_UTILS_H
#define KEYBOARD_UTILS_H

#include "Hardware.h"
#include <BleKeyboard.h>
#include <Wire.h>

// Function prototypes
void initializeHardware();
void initializeBluetooth();
void initializeI2C();
void checkBluetoothConnection();
void setRowState( int row, bool state );
void setLeftRowState( int row, bool state );
void changeID( int DevNum );

// Mode toggle functions
void initializeModeToggle();
void checkModeToggle();
void toggleOutputMode();
void setOutputMode( int mode );
int getOutputMode();

// PCF8575 functions for left split
uint16_t readPCF8575();
bool writePCF8575( uint16_t value );
byte getPCF8575Address();

// Global variables
extern int RowCnt;
extern int LayerCnt;
extern bool pcfInitialized;
extern bool leftSplitConnected;

// Mode toggle variables
extern int currentOutputMode;
extern bool lastBootButtonState;
extern unsigned long lastBootButtonPress;
extern bool modeToggleInitialized;

// External references to arrays from Matrix.cpp
extern short Rows[ NumRows ];
extern short Cols[ NumCols ];
extern int RightLayer1[ NumLayers ][ NumRows ][ NumCols ];
extern int LeftLayer1[ NumLayers ][ NumRows ][ LeftNumCols ];
extern short RightPressedCheck[ NumLayers ][ NumRows ][ NumCols ];
extern short LeftPressedCheck[ NumLayers ][ NumRows ][ LeftNumCols ];

// External references to BLE keyboard and MAC address storage
extern BleKeyboard Kbd;
extern uint8_t MACAddress[][6];
extern const int maxdevice;

#endif
