#ifndef KEY_PROCESSING_H
#define KEY_PROCESSING_H

#include "Hardware.h"
#include "KeyboardUtils.h"

// Function prototypes for key processing
void scanMatrix();
void handleKeyPress( int row, int col );
void handleKeyRelease( int row, int col );
void processSpecialKey( int keyCode, int row, int col, bool isPress );
void processRegularKey( int keyCode, bool isPress );
void logKeyAction( int keyCode, bool isPress );

#endif
