#ifndef KEYBOARD_UTILS_H
#define KEYBOARD_UTILS_H

#include "Hardware.h"

// Function prototypes for keyboard utilities
void initializeHardware();
void initializeBluetooth();
void checkBluetoothConnection();
void setRowState( int row, bool state );

// Global variables
extern int RowCnt;
extern int LayerCnt;

// External references to arrays from Matrix.ino
extern short Rows[ NumRows ];
extern short Cols[ NumCols ];
extern int Layer1[ NumLayers ][ NumRows ][ NumCols ];
extern short PressedCheck[ NumLayers ][ NumRows ][ NumCols ];

// External references from main file
extern BleKeyboard Kbd;
extern uint8_t MACAddress[][6];
extern void changeID( int DevNum );

#endif
