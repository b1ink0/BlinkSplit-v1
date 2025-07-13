#!/usr/bin/env python3
"""
Serial to Keystroke Bridge
Receives keystroke data from ESP32 keyboard via serial and sends them to the OS using pyautogui

Required packages:
pip install pyserial pyautogui

Usage:
python serial_to_keystroke.py [COM_PORT] [BAUD_RATE]

Example:
python serial_to_keystroke.py COM3 115200
"""

import serial
import serial.tools.list_ports
import time
import sys
import threading
from queue import Queue
import signal
import re

try:
    import pyautogui
except ImportError:
    print("pyautogui not installed. Install with: pip install pyautogui")
    sys.exit(1)

# Disable pyautogui failsafe for continuous operation
pyautogui.FAILSAFE = False

# Optimize pyautogui for speed
pyautogui.MINIMUM_DURATION = 0  # Remove artificial delays
pyautogui.MINIMUM_SLEEP = 0     # Remove sleep between actions
pyautogui.PAUSE = 0             # Remove pause between pyautogui calls

class SerialKeyboardBridge:
    def __init__(self, port=None, baud=230400, verbose=False):
        self.port = port
        self.baud = baud
        self.serial_conn = None
        self.running = False
        self.key_queue = Queue()
        self.verbose = verbose  # Control debug output
        
        # Key mappings from Arduino key codes to pyautogui key names
        self.key_mappings = {
            # Function keys (194-205)
            194: 'f1', 195: 'f2', 196: 'f3', 197: 'f4', 198: 'f5',
            199: 'f6', 200: 'f7', 201: 'f8', 202: 'f9', 203: 'f10',
            204: 'f11', 205: 'f12',
            
            # Arrow keys
            215: 'right',        # KEY_RIGHT_ARROW = 215
            216: 'left',         # KEY_LEFT_ARROW = 216
            217: 'down',         # KEY_DOWN_ARROW = 217  
            218: 'up',           # KEY_UP_ARROW = 218
            
            # Special keys - ASCII values
            8: 'backspace',      # ASCII backspace
            9: 'tab',            # ASCII tab
            13: 'enter',         # ASCII carriage return
            27: 'esc',           # ASCII escape
            32: 'space',         # ASCII space
            127: 'delete',       # ASCII delete
            
            # BLE keyboard constants (these are what ESP32 actually sends)
            176: 'enter',        # KEY_RETURN
            177: 'esc',          # KEY_ESC
            178: 'backspace',    # KEY_BACKSPACE
            179: 'tab',          # KEY_TAB
            180: 'space',        # KEY_SPACE (if used as constant)
            181: 'capslock',     # KEY_CAPS_LOCK
            193: 'capslock',     # Alternative caps lock
            212: 'delete',       # KEY_DELETE
            
            # Modifier keys (128-139 range)
            128: 'ctrleft',      # KEY_LEFT_CTRL
            129: 'shiftleft',    # KEY_LEFT_SHIFT  
            130: 'altleft',      # KEY_LEFT_ALT
            131: 'winleft',      # KEY_LEFT_GUI (Windows key)
            132: 'ctrlright',    # KEY_RIGHT_CTRL
            133: 'shiftright',   # KEY_RIGHT_SHIFT
            134: 'altright',     # KEY_RIGHT_ALT
            135: 'winright',     # KEY_RIGHT_GUI (Windows key)
        }
        
        # Special key mappings for media keys
        self.special_mappings = {
            258: 'nexttrack',  # NEXT
            259: 'prevtrack',  # PREV
        }
        
        # Track pressed keys for proper release
        self.pressed_keys = set()

    def discover_esp32_port(self):
        """Auto-discover ESP32 serial port"""
        print("Scanning for available serial ports...")
        
        available_ports = serial.tools.list_ports.comports()
        esp32_ports = []
        
        for port in available_ports:
            port_info = f"{port.device} - {port.description}"
            print(f"Found: {port_info}")
            
            # Check for ESP32 indicators in the port description
            description_lower = port.description.lower()
            if any(indicator in description_lower for indicator in [
                'esp32', 'cp210', 'silicon labs', 'ch340', 'ch341', 
                'ftdi', 'usb-serial', 'uart', 'usb2.0-serial'
            ]):
                esp32_ports.append(port.device)
                print(f"  -> Potential ESP32 port detected")
        
        if not esp32_ports:
            print("\nNo obvious ESP32 ports found. Available ports:")
            for port in available_ports:
                print(f"  {port.device} - {port.description}")
            
            if available_ports:
                print(f"\nTrying first available port: {available_ports[0].device}")
                return available_ports[0].device
            else:
                print("No serial ports found!")
                return None
        
        if len(esp32_ports) == 1:
            print(f"\nAuto-selected ESP32 port: {esp32_ports[0]}")
            return esp32_ports[0]
        else:
            print(f"\nMultiple potential ESP32 ports found:")
            for i, port in enumerate(esp32_ports):
                print(f"  {i+1}. {port}")
            
            try:
                choice = input(f"\nSelect port (1-{len(esp32_ports)}) or press Enter for first: ").strip()
                if not choice:
                    selected_port = esp32_ports[0]
                else:
                    index = int(choice) - 1
                    if 0 <= index < len(esp32_ports):
                        selected_port = esp32_ports[index]
                    else:
                        print("Invalid choice, using first port")
                        selected_port = esp32_ports[0]
                
                print(f"Selected: {selected_port}")
                return selected_port
                
            except (ValueError, KeyboardInterrupt):
                print("Using first detected port")
                return esp32_ports[0]

    def test_port_connection(self, port):
        """Test if a port can be opened and might be an ESP32"""
        try:
            test_conn = serial.Serial(port, self.baud, timeout=2)
            time.sleep(0.1)  # Brief pause
            
            # Try to read some data to see if it's active
            if test_conn.in_waiting > 0:
                data = test_conn.read(test_conn.in_waiting)
                print(f"Port {port} has data: {len(data)} bytes")
            
            test_conn.close()
            return True
            
        except Exception as e:
            print(f"Cannot connect to {port}: {e}")
            return False

    def connect(self):
        """Connect to the serial port"""
        # Auto-discover port if not specified
        if self.port is None:
            self.port = self.discover_esp32_port()
            if self.port is None:
                print("No suitable serial port found!")
                return False
        
        # Test the port first
        if not self.test_port_connection(self.port):
            return False
        
        try:
            self.serial_conn = serial.Serial(self.port, self.baud, timeout=1)
            print(f"Successfully connected to {self.port} at {self.baud} baud")
            print("Waiting for keyboard data...")
            print("Make sure your ESP32 keyboard is in SERIAL mode")
            print("(Press the boot button to toggle between BLE and SERIAL modes)")
            print()
            return True
        except serial.SerialException as e:
            print(f"Failed to connect to {self.port}: {e}")
            
            # Provide helpful error messages
            if "access is denied" in str(e).lower() or "permission denied" in str(e).lower():
                print("\nTroubleshooting:")
                print("- Close any other applications using this port (Arduino IDE, PuTTY, etc.)")
                print("- Try unplugging and reconnecting the ESP32")
                print("- On Linux: Add user to dialout group: sudo usermod -a -G dialout $USER")
            
            return False

    def disconnect(self):
        """Disconnect from serial port"""
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
            print("Disconnected from serial port")

    def parse_key_command(self, line):
        """Parse incoming key command from serial"""
        line = line.strip()
        if not line:
            return None
            
        # Expected format: "KEY:keycode:PRESS" or "KEY:keycode:RELEASE"
        # or "SPECIAL:keycode:PRESS" or "SPECIAL:keycode:RELEASE"
        parts = line.split(':')
        if len(parts) != 3:
            return None
            
        command_type, keycode_str, action = parts
        
        if command_type not in ['KEY', 'SPECIAL']:
            return None
            
        if action not in ['PRESS', 'RELEASE']:
            return None
            
        try:
            keycode = int(keycode_str)
        except ValueError:
            return None
            
        return {
            'type': command_type,
            'keycode': keycode,
            'action': action
        }

    def get_key_name(self, keycode, command_type):
        """Convert keycode to pyautogui key name"""
        if command_type == 'SPECIAL':
            return self.special_mappings.get(keycode)
        elif command_type == 'KEY':
            # Check if it's in our mapping table
            if keycode in self.key_mappings:
                key_name = self.key_mappings[keycode]
                
                # Handle pyautogui compatibility - some keys need fallbacks
                if key_name in ['winleft', 'winright']:
                    return 'win'  # pyautogui uses 'win' for Windows key
                elif key_name in ['ctrleft', 'ctrlright']:
                    return 'ctrl'  # pyautogui uses generic 'ctrl'
                elif key_name in ['shiftleft', 'shiftright']:
                    return 'shift'  # pyautogui uses generic 'shift'
                elif key_name in ['altleft', 'altright']:
                    return 'alt'  # pyautogui uses generic 'alt'
                else:
                    return key_name
                    
            # For printable ASCII characters
            elif 32 <= keycode <= 126:
                return chr(keycode)
            else:
                return None
        return None

    def process_keystroke(self, command):
        """Process a single keystroke command"""
        key_name = self.get_key_name(command['keycode'], command['type'])
        
        if key_name is None:
            # Always show unknown keys for debugging
            print(f"Unknown key: {command['keycode']} ({command['type']}) - Please report this!")
            return
            
        action = command['action']
        
        try:
            if action == 'PRESS':
                if command['type'] == 'SPECIAL':
                    # For special keys like media keys, just press and release
                    pyautogui.press(key_name)
                    if self.verbose:
                        print(f"Special key pressed: {key_name}")
                else:
                    # For regular keys, track the press
                    pyautogui.keyDown(key_name)
                    self.pressed_keys.add(key_name)
                    if self.verbose:
                        print(f"Key pressed: {key_name}")
                    
            elif action == 'RELEASE':
                if command['type'] != 'SPECIAL' and key_name in self.pressed_keys:
                    pyautogui.keyUp(key_name)
                    self.pressed_keys.discard(key_name)
                    if self.verbose:
                        print(f"Key released: {key_name}")
                    
        except Exception as e:
            print(f"Error processing keystroke {key_name}: {e}")

    def serial_reader_thread(self):
        """Thread to read from serial port"""
        first_data_received = False
        buffer = ""  # Buffer for incomplete lines
        
        while self.running:
            try:
                if self.serial_conn and self.serial_conn.in_waiting > 0:
                    # Read all available data at once for better performance
                    data = self.serial_conn.read(self.serial_conn.in_waiting)
                    text = data.decode('utf-8', errors='ignore')
                    buffer += text
                    
                    # Process complete lines
                    while '\n' in buffer:
                        line, buffer = buffer.split('\n', 1)
                        line = line.strip()
                        
                        # Provide feedback when first data is received (only once)
                        if not first_data_received and line:
                            if line.startswith('KEY:') or line.startswith('SPECIAL:'):
                                print("✓ ESP32 keyboard detected and working!")
                                if not self.verbose:
                                    print("Running in fast mode (no debug output)")
                                print("Ready to receive keystrokes...\n")
                                first_data_received = True
                            elif any(keyword in line.lower() for keyword in ['keyboard', 'split', 'ble', 'mode']):
                                print("✓ ESP32 detected - waiting for keystroke data...")
                                first_data_received = True
                        
                        # Parse and queue the command
                        command = self.parse_key_command(line)
                        if command:
                            self.key_queue.put(command)
                
                # Much smaller delay for better responsiveness
                time.sleep(0.0001)  # 0.1ms instead of 1ms
            except Exception as e:
                if self.verbose:
                    print(f"Serial read error: {e}")
                time.sleep(0.01)

    def keystroke_processor_thread(self):
        """Thread to process keystrokes"""
        while self.running:
            try:
                # Process multiple commands in batch for better performance
                commands_to_process = []
                
                # Get first command (blocking)
                try:
                    command = self.key_queue.get(timeout=0.01)  # Reduced timeout
                    commands_to_process.append(command)
                except:
                    continue
                
                # Get any additional commands that are ready (non-blocking)
                while not self.key_queue.empty() and len(commands_to_process) < 10:
                    try:
                        command = self.key_queue.get_nowait()
                        commands_to_process.append(command)
                    except:
                        break
                
                # Process all commands
                for command in commands_to_process:
                    self.process_keystroke(command)
                    
            except Exception as e:
                if self.verbose:
                    print(f"Keystroke processing error: {e}")

    def run(self):
        """Main run loop"""
        if not self.connect():
            return
            
        self.running = True
        
        # Set higher thread priority for better responsiveness
        reader_thread = threading.Thread(target=self.serial_reader_thread, daemon=True)
        processor_thread = threading.Thread(target=self.keystroke_processor_thread, daemon=True)
        
        reader_thread.start()
        processor_thread.start()
        
        print("Serial to Keystroke Bridge running...")
        if self.verbose:
            print("Verbose mode enabled - all keystrokes will be logged")
        else:
            print("Fast mode enabled - minimal output for better performance")
        print("Press Ctrl+C to stop")
        
        try:
            while self.running:
                time.sleep(0.1)
        except KeyboardInterrupt:
            print("\nShutting down...")
            self.stop()

    def stop(self):
        """Stop the bridge"""
        self.running = False
        
        # Release all pressed keys
        for key in self.pressed_keys.copy():
            try:
                pyautogui.keyUp(key)
            except:
                pass
        self.pressed_keys.clear()
        
        self.disconnect()

def signal_handler(sig, frame):
    """Handle Ctrl+C gracefully"""
    print("\nReceived interrupt signal")
    sys.exit(0)

def main():
    signal.signal(signal.SIGINT, signal_handler)
    
    print("=== Serial to Keystroke Bridge ===")
    print("Auto-discovering ESP32 keyboard...")
    print()
    
    # Parse command line arguments
    port = None
    baud = 230400  # faster baud rate
    verbose = False
    
    for arg in sys.argv[1:]:
        if arg.upper().startswith('COM') or arg.startswith('/dev/'):
            port = arg
            print(f"Using specified port: {port}")
        elif arg.lower() in ['-v', '--verbose', 'verbose']:
            verbose = True
            print("Verbose mode enabled")
        elif arg.lower() in ['-q', '--quiet', 'quiet']:
            verbose = False
            print("Quiet mode enabled")
        else:
            try:
                baud = int(arg)
                print(f"Using specified baud rate: {baud}")
            except ValueError:
                print(f"Invalid parameter: {arg}")
                print("Usage: python serial_to_keystroke.py [PORT] [BAUD_RATE] [verbose|quiet]")
                print("Example: python serial_to_keystroke.py COM3 230400 verbose")
                sys.exit(1)
    
    print(f"Baud rate: {baud}")
    print(f"Mode: {'Verbose' if verbose else 'Fast (quiet)'}")
    print()
    
    bridge = SerialKeyboardBridge(port, baud, verbose)
    bridge.run()

if __name__ == "__main__":
    main()
