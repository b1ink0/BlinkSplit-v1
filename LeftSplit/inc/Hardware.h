//Define on and off values
#define OFF 0
#define ON 1

// ESP8266 Pin Definitions for I²C
#define SDA_PIN 4  // D2 on ESP8266
#define SCL_PIN 5  // D1 on ESP8266
#define INT_PIN 0  // D3 on ESP8266 (optional interrupt)

// Serial communication pins for split communication
#define SERIAL_TX 1  // GPIO1 (TX0) - connects to D- of USB-C
#define SERIAL_RX 3  // GPIO3 (RX0) - connects to D+ of USB-C

// Matrix setup - 6x6 matrix
#define NumRows 6
#define NumCols 6
#define NumLayers 1

// PCF8575 pin mapping for matrix
// Rows: P0_0 to P0_5 (bits 0-5 of lower byte)
#define ROW_MASK 0x003F  // Bits 0-5 for rows
// Cols: P1_0 to P1_5 (bits 8-13 of 16-bit word)
#define COL_MASK 0x3F00  // Bits 8-13 for columns

// Special key definitions
#define FUNCTION_SW   256
#define NULL_CON      257
#define NEXT          258
#define PREV          259

#define KEY_LEFT_CTRL 0x80
#define KEY_LEFT_SHIFT 0x81
#define KEY_LEFT_ALT 0x82
#define KEY_LEFT_GUI 0x83
#define KEY_RIGHT_CTRL 0x84
#define KEY_RIGHT_SHIFT 0x85
#define KEY_RIGHT_ALT 0x86
#define KEY_RIGHT_GUI 0x87

#define KEY_UP_ARROW 0xDA
#define KEY_DOWN_ARROW 0xD9
#define KEY_LEFT_ARROW 0xD8
#define KEY_RIGHT_ARROW 0xD7
#define KEY_BACKSPACE 0xB2
#define KEY_TAB 0xB3
#define KEY_RETURN 0xB0
#define KEY_ESC 0xB1
#define KEY_INSERT 0xD1
#define KEY_PRTSC 0xCE
#define KEY_DELETE 0xD4
#define KEY_PAGE_UP 0xD3
#define KEY_PAGE_DOWN 0xD6
#define KEY_HOME 0xD2
#define KEY_END 0xD5
#define KEY_CAPS_LOCK 0xC1
#define KEY_F1 0xC2
#define KEY_F2 0xC3
#define KEY_F3 0xC4
#define KEY_F4 0xC5
#define KEY_F5 0xC6
