#ifndef KEYBOARD_UTILS_H
#define KEYBOARD_UTILS_H

#include "Hardware.h"
#include <Wire.h>

// Function prototypes
void initializeHardware();
void initializeSerial();
void checkSerialConnection();
void setRowState( int row, bool state );
void sendKeyToRight( int keyCode, bool isPress );
uint16_t readPCF8575();
bool writePCF8575( uint16_t value );
byte getPCF8575Address(); 

// Global variables
extern int RowCnt;
extern int LayerCnt;
extern bool pcfInitialized;

// External references to arrays from Matrix.cpp
extern int Layer1[ NumLayers ][ NumRows ][ NumCols ];
extern short PressedCheck[ NumLayers ][ NumRows ][ NumCols ];

#endif
