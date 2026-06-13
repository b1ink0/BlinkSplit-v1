#ifndef KEY_PROCESSING_H
#define KEY_PROCESSING_H

#include "Hardware.h"
#include "KeyboardUtils.h"

// Function prototypes for key processing
void scanMatrix();
void scanRightMatrix();
void scanLeftMatrix();
void handleRightKeyPress( int row, int col );
void handleRightKeyRelease( int row, int col );
void handleLeftKeyPress( int row, int col );
void handleLeftKeyRelease( int row, int col );
void processSpecialKey( int keyCode, int row, int col, bool isPress );
void processRegularKey( int keyCode, bool isPress );
void logKeyAction( int keyCode, bool isPress, const char* split );

// Serial output functions
void sendKeyViaSerial( int keyCode, bool isPress );
void sendSpecialKeyViaSerial( int keyCode, bool isPress );

#endif
