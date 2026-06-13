#!/bin/bash

echo "Installing required Python packages..."
pip3 install pyserial pyautogui

echo ""
echo "Starting High-Speed Serial to Keystroke Bridge..."
echo "The script will automatically find your ESP32 keyboard"
echo "Running at 230400 baud for maximum performance"
echo "Press Ctrl+C to stop"
echo ""

python3 main.py

echo ""
echo "Press Enter to exit..."
read
