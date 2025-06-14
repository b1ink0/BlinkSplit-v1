#include <Arduino.h>
#include <BleKeyboard.h>
#include "../inc/Hardware.h"

//Rows to scan, and Columns to read - Fixed order
short Rows[ NumRows ] = { Row1, Row2, Row3, Row4, Row5, Row6 };
short Cols[ NumCols ] = { Col7, Col8, Col9, Col10, Col11, Col12, Col13, Col14 };

//Keyboard layout - Using your original layout (non-transposed)
int Layer1[ NumLayers ][ NumRows ][ NumCols ] = 
{
  { //Col 7     Col 8     Col 9          Col 10    Col 11    Col 12           Col 13          Col 14    
    { KEY_F6,   KEY_F7,   KEY_F8,        KEY_F9,   KEY_F8,   KEY_F9,          KEY_F10,        KEY_F11          },  // Row 1
    { '7',      '8',      '9',           '0',      '-',      '=',             KEY_BACKSPACE,  KEY_DELETE       },  // Row 2  
    { 'y',      'u',      'i',           'o',      'p',      '[',             ']',            '\\'             },  // Row 3
    { 'h',      'j',      'k',           'l',      ';',      '\'',            KEY_RETURN,     KEY_RIGHT_ARROW  },  // Row 4
    { 'n',      'm',      ',',           '.',      '/',      KEY_RIGHT_SHIFT, NULL_CON,       KEY_UP_ARROW     },  // Row 5
    { ' ',      NULL_CON, KEY_RIGHT_ALT, NULL_CON, NULL_CON, KEY_RIGHT_CTRL,  KEY_LEFT_ARROW, KEY_DOWN_ARROW   }   // Row 6
  }
};

//Array to check if a key is currently pressed
short PressedCheck[ NumLayers ][ NumRows ][ NumCols ] = { OFF };
