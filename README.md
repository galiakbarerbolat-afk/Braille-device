# ESP32 Braille Device

AI-powered Braille display device supporting English, Russian, and Kazakh languages.

## Features
- Single ESP32 microcontroller
- Real-time text-to-Braille conversion
- Multi-language support (EN/RU/KZ)
- 6 solenoid control
- Array-based fast translation

## Quick Start
1. Upload `ESP32_BrailleDevice_Complete.ino` to ESP32
2. Connect 6 solenoids via MOSFETs to GPIO pins 25,26,27,32,33,14
3. Open Serial Monitor (115200 baud)
4. Type text and press Enter

## Documentation
- [Hardware Guide](HARDWARE_DOCUMENTATION_SINGLE.md)
- [Block Diagrams](BLOCK_DIAGRAMS_SINGLE.md)
- [Complete README](README_SINGLE.md)

## Circuit
ESP32 → MOSFET → Solenoid
GPIO25-33 → 1kΩ → Gate → 5V Solenoid

## License
MIT License
