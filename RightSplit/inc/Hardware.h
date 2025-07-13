//Define on and off values
#define OFF 0
#define ON 1

// Boot button for mode toggle (GPIO0 on most ESP32 boards)
#define BOOT_BUTTON_PIN 0

// Output mode definitions
#define OUTPUT_MODE_BLE 0
#define OUTPUT_MODE_SERIAL 1

// Serial communication settings
#define SERIAL_BAUD_RATE 230400  // Faster baud rate for better performance 

//Pin Definitions - Right split direct GPIO
#define Col7 15
#define Col8 16
#define Col9 17
#define Col10 18
#define Col11 19
#define Col12 21
#define Col13 22
#define Col14 23

#define Row1 32
#define Row2 4
#define Row3 5
#define Row4 12
#define Row5 13
#define Row6 14

// I²C pins for left split PCF8575 communication via USB-C
#define SDA_PIN 25  // GPIO25 - connects to D+ of USB-C
#define SCL_PIN 26  // GPIO26 - connects to D- of USB-C

//Matrix setup
#define NumRows 6
#define NumCols 8  // Right split has 8 columns
#define LeftNumCols 6  // Left split has 6 columns
#define NumLayers 1

// PCF8575 pin mapping for left split matrix
// Rows: P0_0 to P0_5 (bits 0-5 of lower byte)
#define LEFT_ROW_MASK 0x003F  // Bits 0-5 for rows
// Cols: P1_0 to P1_5 (bits 8-13 of 16-bit word)
#define LEFT_COL_MASK 0x3F00  // Bits 8-13 for columns

// Special key definitions
#define FUNCTION_SW   256
#define NULL_CON      257
#define NEXT          258
#define PREV          259
