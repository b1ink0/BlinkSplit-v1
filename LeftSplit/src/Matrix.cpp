#include <Arduino.h>
#include "../inc/Hardware.h"

// Keyboard layout - Left split layout
int Layer1[ NumLayers ][ NumRows ][ NumCols ] = 
{
  {
    { KEY_ESC,       KEY_F1,       KEY_F2,       KEY_F3,       KEY_F4,       KEY_F5       },  // Row 1
    { '`',           '1',          '2',          '3',          '4',          '5'          },  // Row 2  
    { KEY_TAB,       'q',          'w',          'e',          'r',          't'          },  // Row 3
    { KEY_CAPS_LOCK, 'a',          's',          'd',          'f',          'g'          },  // Row 4
    { KEY_LEFT_SHIFT,'z',          'x',          'c',          'v',          'b'          },  // Row 5
    { KEY_LEFT_CTRL, KEY_LEFT_GUI, KEY_LEFT_ALT, NULL_CON,     ' ',          '6'          }   // Row 6
  }
};

// Array to check if a key is currently pressed
short PressedCheck[ NumLayers ][ NumRows ][ NumCols ] = { OFF };
