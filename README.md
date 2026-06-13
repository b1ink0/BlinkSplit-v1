# BlinkSplit v1

BlinkSplit v1 is custom firmware for my hand-wired split keyboard build.

Reference keyboard used for this build: Redgear Shadow Blade (used as the donor keyboard for switches, keycaps, and shell parts).

<img style="border-radius: 20px" src="./assets/1.jpg">

## Firmware Architecture

The firmware runs on an ESP32 and scans both halves:

- Right half: direct GPIO matrix scan
- Left half: PCF8575 over I2C
- Output modes: BLE keyboard mode and optional serial output mode

## Flashing

1. Install Arduino IDE 2.x.
2. Install the ESP32 boards package in Arduino IDE.
3. Install the ESP32 BLE Keyboard library by T-vK.
4. Open `main.ino`.
5. Select your ESP32 board and COM port.
6. Upload.

### Optional Serial Bridge

If you want to use serial mode on the firmware side, the host bridge is in `serial/main.py`.

Required Python packages:

- Python 3
- pyserial
- pyautogui

## Parts Used

| Part | Notes |
| --- | --- |
| ESP32 dev board | Main controller |
| PCF8575 I2C expander | Left-half matrix scan |
| Mechanical switches | Reused from donor keyboard |
| Keycaps | Reused from donor keyboard |
| USB-C cable / split interconnect wiring | Half-to-half connection and routing |
| Reset / mode buttons | Mounted on the back |
| Hookup wire, solder, heatshrink | Hand wiring and insulation |
| Original keyboard shell parts | Reused and modified |

## Credits

- Original Sanctuary firmware by [Foster Phillips](https://linktr.ee/Lego_Rocket)

## License

GNU GPLv3. See [LICENSE.md](LICENSE.md).
