#ifndef KEYBOARD_UTILS_H
#define KEYBOARD_UTILS_H

#include "Hardware.h"
#include <BleKeyboard.h>

// Function prototypes
void initializeHardware();
void initializeBluetooth();
void checkBluetoothConnection();
void setRowState( int row, bool state );
void changeID( int DevNum );  // Added this function

// Global variables
extern int RowCnt;
extern int LayerCnt;

// External references to arrays from Matrix.cpp
extern short Rows[ NumRows ];
extern short Cols[ NumCols ];
extern int Layer1[ NumLayers ][ NumRows ][ NumCols ];
extern short PressedCheck[ NumLayers ][ NumRows ][ NumCols ];

// External references to BLE keyboard and MAC address storage
extern BleKeyboard Kbd;
extern uint8_t MACAddress[][6];
extern const int maxdevice;

#endif
