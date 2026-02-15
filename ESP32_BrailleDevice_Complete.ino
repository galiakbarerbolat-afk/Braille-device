/*
 * ESP32 Complete Braille Device
 * 
 * Purpose: All-in-one Braille display device
 *          - Receives text from OCR (via Serial/WiFi)
 *          - Translates to Braille patterns
 *          - Controls 6 solenoids directly
 * 
 * Supports: English, Kazakh, Russian
 * Hardware: ESP32 + 6 solenoids + 6 MOSFETs
 * 
 * Author: Braille Device Project
 * Date: 2025
 */

#include <Arduino.h>

// ============================================
// HARDWARE CONFIGURATION
// ============================================

// Solenoid control pins (connected to MOSFETs)
#define SOLENOID_1 GPIO_NUM_25  // Dot 1 (top left)
#define SOLENOID_2 GPIO_NUM_26  // Dot 2 (middle left)
#define SOLENOID_3 GPIO_NUM_27  // Dot 3 (bottom left)
#define SOLENOID_4 GPIO_NUM_32  // Dot 4 (top right)
#define SOLENOID_5 GPIO_NUM_33  // Dot 5 (middle right)
#define SOLENOID_6 GPIO_NUM_14  // Dot 6 (bottom right)

// OCR Input Configuration
#define OCR_INPUT_SERIAL Serial  // Use Serial for OCR input or testing

// Timing Configuration
#define SOLENOID_EXTEND_TIME 50    // ms - time to extend solenoid
#define SOLENOID_HOLD_TIME 950     // ms - time to hold position for reading
#define RETRACT_TIME 50            // ms - time between characters
#define CHAR_DISPLAY_TIME 1000     // ms - total display time per character

// Status LED (built-in)
#define STATUS_LED LED_BUILTIN

// ============================================
// PIN ARRAY FOR EASY ITERATION
// ============================================

const uint8_t SOLENOID_PINS[6] = {
    SOLENOID_1,
    SOLENOID_2,
    SOLENOID_3,
    SOLENOID_4,
    SOLENOID_5,
    SOLENOID_6
};

// ============================================
// BRAILLE CHARACTER STRUCTURES
// ============================================

// Braille pattern structure
struct BraillePattern {
    char character;
    uint8_t dots;  // 6-bit pattern (bits 0-5 represent dots 1-6)
};

// ============================================
// BRAILLE MAPPING ARRAYS
// ============================================

// English lowercase letters (a-z)
const BraillePattern ENGLISH_LOWER[] = {
    {'a', 0b000001},  // dot 1
    {'b', 0b000011},  // dots 1,2
    {'c', 0b001001},  // dots 1,4
    {'d', 0b011001},  // dots 1,4,5
    {'e', 0b010001},  // dots 1,5
    {'f', 0b001011},  // dots 1,2,4
    {'g', 0b011011},  // dots 1,2,4,5
    {'h', 0b010011},  // dots 1,2,5
    {'i', 0b001010},  // dots 2,4
    {'j', 0b011010},  // dots 2,4,5
    {'k', 0b000101},  // dots 1,3
    {'l', 0b000111},  // dots 1,2,3
    {'m', 0b001101},  // dots 1,3,4
    {'n', 0b011101},  // dots 1,3,4,5
    {'o', 0b010101},  // dots 1,3,5
    {'p', 0b001111},  // dots 1,2,3,4
    {'q', 0b011111},  // dots 1,2,3,4,5
    {'r', 0b010111},  // dots 1,2,3,5
    {'s', 0b001110},  // dots 2,3,4
    {'t', 0b011110},  // dots 2,3,4,5
    {'u', 0b100101},  // dots 1,3,6
    {'v', 0b100111},  // dots 1,2,3,6
    {'w', 0b111010},  // dots 2,4,5,6
    {'x', 0b101101},  // dots 1,3,4,6
    {'y', 0b111101},  // dots 1,3,4,5,6
    {'z', 0b110101}   // dots 1,3,5,6
};

// Numbers (0-9) - use number indicator + letters a-j
const BraillePattern NUMBERS[] = {
    {'0', 0b011010},  // j
    {'1', 0b000001},  // a
    {'2', 0b000011},  // b
    {'3', 0b001001},  // c
    {'4', 0b011001},  // d
    {'5', 0b010001},  // e
    {'6', 0b001011},  // f
    {'7', 0b011011},  // g
    {'8', 0b010011},  // h
    {'9', 0b001010}   // i
};

// Punctuation and special characters
const BraillePattern PUNCTUATION[] = {
    {' ', 0b000000},  // space (no dots)
    {'.', 0b010110},  // dots 2,3,5
    {',', 0b000010},  // dot 2
    {'?', 0b100110},  // dots 2,3,6
    {'!', 0b011110},  // dots 2,3,4,5
    {';', 0b000110},  // dots 2,3
    {':', 0b010010},  // dots 2,5
    {'-', 0b001100},  // dots 3,4
    {'\'', 0b000100}, // dot 3
    {'"', 0b100110},  // dots 2,3,6
    {'(', 0b010111},  // dots 1,2,3,5
    {')', 0b010111},  // dots 1,2,3,5
};

// Russian Cyrillic characters (а-я)
const BraillePattern RUSSIAN_CHARS[] = {
    {'а', 0b000001},  // a - dot 1
    {'б', 0b000011},  // b - dots 1,2
    {'в', 0b111010},  // v - dots 2,4,5,6
    {'г', 0b011011},  // g - dots 1,2,4,5
    {'д', 0b011001},  // d - dots 1,4,5
    {'е', 0b010001},  // e - dots 1,5
    {'ё', 0b010000},  // yo - dot 5
    {'ж', 0b101011},  // zh - dots 1,2,4,6
    {'з', 0b110101},  // z - dots 1,3,5,6
    {'и', 0b001010},  // i - dots 2,4
    {'й', 0b111111},  // y - dots 1,2,3,4,5,6
    {'к', 0b000101},  // k - dots 1,3
    {'л', 0b000111},  // l - dots 1,2,3
    {'м', 0b001101},  // m - dots 1,3,4
    {'н', 0b011101},  // n - dots 1,3,4,5
    {'о', 0b010101},  // o - dots 1,3,5
    {'п', 0b001111},  // p - dots 1,2,3,4
    {'р', 0b010111},  // r - dots 1,2,3,5
    {'с', 0b001110},  // s - dots 2,3,4
    {'т', 0b011110},  // t - dots 2,3,4,5
    {'у', 0b100101},  // u - dots 1,3,6
    {'ф', 0b001011},  // f - dots 1,2,4
    {'х', 0b110111},  // h - dots 1,2,3,5,6
    {'ц', 0b001001},  // ts - dots 1,4
    {'ч', 0b111011},  // ch - dots 1,2,4,5,6
    {'ш', 0b100100},  // sh - dot 6
    {'щ', 0b100001},  // shch - dots 1,6
    {'ъ', 0b111101},  // hard sign - dots 1,3,4,5,6
    {'ы', 0b111110},  // y - dots 2,3,4,5,6
    {'ь', 0b101110},  // soft sign - dots 2,3,4,6
    {'э', 0b101001},  // e - dots 1,4,6
    {'ю', 0b100111},  // yu - dots 1,2,3,6
    {'я', 0b111001}   // ya - dots 1,4,5,6
};

// Kazakh specific characters (additional to Russian)
const BraillePattern KAZAKH_CHARS[] = {
    {'ә', 0b110001},  // ä - dots 1,5,6
    {'ғ', 0b111100},  // ğ - dots 3,4,5,6
    {'қ', 0b100011},  // q - dots 1,2,6
    {'ң', 0b110011},  // ñ - dots 1,2,5,6
    {'ө', 0b101010},  // ö - dots 2,4,6
    {'ұ', 0b100110},  // ū - dots 2,3,6
    {'ү', 0b101111},  // ü - dots 1,2,3,4,6
    {'һ', 0b110010},  // h - dots 2,5,6
    {'і', 0b001010}   // i - dots 2,4 (same as и)
};

// Special indicators
const uint8_t CAPITAL_INDICATOR = 0b100000;  // dot 6
const uint8_t NUMBER_INDICATOR = 0b001111;   // dots 1,2,3,4

// ============================================
// GLOBAL VARIABLES
// ============================================

String inputBuffer = "";
bool processingComplete = false;

// ============================================
// FUNCTION DECLARATIONS
// ============================================

void setupPins();
void displayBraillePattern(uint8_t pattern);
void extendDots(uint8_t pattern);
void retractAllDots();
void testAllSolenoids();
void blinkStatus(int times);
uint8_t getBraillePattern(char c);
void processText(String text);
void displayBrailleChar(char c);

// ============================================
// SETUP
// ============================================

void setup() {
    // Initialize Serial for OCR input and debugging
    Serial.begin(115200);
    
    // Initialize GPIO pins for solenoids
    setupPins();
    
    // Wait for connections
    delay(2000);
    
    // Visual feedback
    blinkStatus(3);
    
    // Run self-test
    Serial.println("\n=================================");
    Serial.println("ESP32 Braille Display Device");
    Serial.println("=================================");
    Serial.println("Running self-test...");
    testAllSolenoids();
    
    Serial.println("\nSupported Languages:");
    Serial.println("- English");
    Serial.println("- Russian (Русский)");
    Serial.println("- Kazakh (Қазақша)");
    Serial.println("=================================");
    Serial.println("Ready to receive text...");
    Serial.println();
}

// ============================================
// MAIN LOOP
// ============================================

void loop() {
    // Check for incoming text from OCR or Serial
    if (Serial.available()) {
        char c = Serial.read();
        
        if (c == '\n' || c == '\r') {
            // End of line - process the buffer
            if (inputBuffer.length() > 0) {
                Serial.println("\n--- Processing Text ---");
                Serial.print("Input: ");
                Serial.println(inputBuffer);
                Serial.println("--- Translating to Braille ---");
                
                processText(inputBuffer);
                
                Serial.println("--- Complete ---\n");
                inputBuffer = "";
            }
        } else {
            // Add character to buffer
            inputBuffer += c;
        }
    }
    
    // Small delay to prevent overwhelming the processor
    delay(10);
}

// ============================================
// PIN SETUP
// ============================================

void setupPins() {
    // Configure all solenoid pins as outputs
    for (int i = 0; i < 6; i++) {
        pinMode(SOLENOID_PINS[i], OUTPUT);
        digitalWrite(SOLENOID_PINS[i], LOW);  // Ensure retracted
    }
    
    // Configure status LED
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW);
    
    Serial.println("GPIO pins initialized:");
    Serial.println("  Solenoid 1 (Dot 1): GPIO25");
    Serial.println("  Solenoid 2 (Dot 2): GPIO26");
    Serial.println("  Solenoid 3 (Dot 3): GPIO27");
    Serial.println("  Solenoid 4 (Dot 4): GPIO32");
    Serial.println("  Solenoid 5 (Dot 5): GPIO33");
    Serial.println("  Solenoid 6 (Dot 6): GPIO14");
}

// ============================================
// BRAILLE PATTERN LOOKUP
// ============================================

uint8_t getBraillePattern(char c) {
    // Convert to lowercase for lookup
    char lowerC = tolower(c);
    
    // Check English letters
    if (lowerC >= 'a' && lowerC <= 'z') {
        return ENGLISH_LOWER[lowerC - 'a'].dots;
    }
    
    // Check numbers
    if (c >= '0' && c <= '9') {
        return NUMBERS[c - '0'].dots;
    }
    
    // Check punctuation
    for (int i = 0; i < sizeof(PUNCTUATION) / sizeof(BraillePattern); i++) {
        if (PUNCTUATION[i].character == c) {
            return PUNCTUATION[i].dots;
        }
    }
    
    // Check Russian characters
    for (int i = 0; i < sizeof(RUSSIAN_CHARS) / sizeof(BraillePattern); i++) {
        if (RUSSIAN_CHARS[i].character == lowerC) {
            return RUSSIAN_CHARS[i].dots;
        }
    }
    
    // Check Kazakh-specific characters
    for (int i = 0; i < sizeof(KAZAKH_CHARS) / sizeof(BraillePattern); i++) {
        if (KAZAKH_CHARS[i].character == lowerC) {
            return KAZAKH_CHARS[i].dots;
        }
    }
    
    // Unknown character - return error pattern
    return 0b111111;
}

// ============================================
// PROCESS TEXT STRING
// ============================================

void processText(String text) {
    bool numberMode = false;
    
    for (int i = 0; i < text.length(); i++) {
        char c = text.charAt(i);
        
        // Check if we need to display capital indicator
        if (isupper(c) && isalpha(c)) {
            Serial.print("Character: [CAPITAL] ");
            displayBraillePattern(CAPITAL_INDICATOR);
            delay(100);
        }
        
        // Check if we need to display number indicator
        if (isdigit(c) && !numberMode) {
            Serial.print("Character: [NUMBER] ");
            displayBraillePattern(NUMBER_INDICATOR);
            numberMode = true;
            delay(100);
        } else if (!isdigit(c) && c != ',' && c != '.') {
            numberMode = false;
        }
        
        // Get and display the character pattern
        uint8_t pattern = getBraillePattern(c);
        
        Serial.print("Character: '");
        Serial.print(c);
        Serial.print("' → Pattern: 0b");
        Serial.print(pattern, BIN);
        Serial.print(" (dots: ");
        
        // Print which dots are active
        bool first = true;
        for (int j = 0; j < 6; j++) {
            if (pattern & (1 << j)) {
                if (!first) Serial.print(",");
                Serial.print(j + 1);
                first = false;
            }
        }
        if (first) Serial.print("none");
        Serial.println(")");
        
        // Display the pattern
        displayBraillePattern(pattern);
        delay(100);
    }
}

// ============================================
// DISPLAY BRAILLE PATTERN
// ============================================

void displayBraillePattern(uint8_t pattern) {
    // Step 1: Retract all dots (clean slate)
    retractAllDots();
    delay(RETRACT_TIME);
    
    // Step 2: Extend the dots specified in the pattern
    extendDots(pattern);
    
    // Step 3: Initial extension with full power
    delay(SOLENOID_EXTEND_TIME);
    
    // Step 4: Hold position for user to read
    delay(SOLENOID_HOLD_TIME);
    
    // Step 5: Retract all for next character
    retractAllDots();
}

// ============================================
// EXTEND SPECIFIC DOTS
// ============================================

void extendDots(uint8_t pattern) {
    // Iterate through each dot (bit)
    for (int i = 0; i < 6; i++) {
        // Check if this dot should be raised
        if (pattern & (1 << i)) {
            // Raise the solenoid (HIGH = extended)
            digitalWrite(SOLENOID_PINS[i], HIGH);
        } else {
            // Keep retracted (LOW = retracted)
            digitalWrite(SOLENOID_PINS[i], LOW);
        }
    }
}

// ============================================
// RETRACT ALL DOTS
// ============================================

void retractAllDots() {
    // Set all solenoid pins LOW (retracted position)
    for (int i = 0; i < 6; i++) {
        digitalWrite(SOLENOID_PINS[i], LOW);
    }
}

// ============================================
// SELF-TEST SEQUENCE
// ============================================

void testAllSolenoids() {
    Serial.println("Testing each solenoid individually...");
    
    // Test each solenoid individually
    for (int i = 0; i < 6; i++) {
        Serial.print("  Testing Dot ");
        Serial.print(i + 1);
        Serial.print(" (GPIO");
        Serial.print(SOLENOID_PINS[i]);
        Serial.println(")...");
        
        // Extend this solenoid
        digitalWrite(SOLENOID_PINS[i], HIGH);
        delay(200);
        
        // Retract this solenoid
        digitalWrite(SOLENOID_PINS[i], LOW);
        delay(100);
    }
    
    // Test all together
    Serial.println("Testing all solenoids together...");
    delay(200);
    for (int i = 0; i < 6; i++) {
        digitalWrite(SOLENOID_PINS[i], HIGH);
    }
    delay(300);
    retractAllDots();
    delay(200);
    
    Serial.println("Self-test complete!\n");
}

// ============================================
// STATUS LED BLINK
// ============================================

void blinkStatus(int times) {
    for (int i = 0; i < times; i++) {
        digitalWrite(STATUS_LED, HIGH);
        delay(100);
        digitalWrite(STATUS_LED, LOW);
        delay(100);
    }
}

/*
 * ============================================
 * USAGE NOTES
 * ============================================
 * 
 * To use this code:
 * 
 * 1. Upload to ESP32 board
 * 
 * 2. Connect 6 solenoids via MOSFETs:
 *    - GPIO25 → MOSFET → Solenoid 1 (Dot 1)
 *    - GPIO26 → MOSFET → Solenoid 2 (Dot 2)
 *    - GPIO27 → MOSFET → Solenoid 3 (Dot 3)
 *    - GPIO32 → MOSFET → Solenoid 4 (Dot 4)
 *    - GPIO33 → MOSFET → Solenoid 5 (Dot 5)
 *    - GPIO14 → MOSFET → Solenoid 6 (Dot 6)
 * 
 * 3. For testing, open Serial Monitor (115200 baud)
 *    Type text and press Enter
 * 
 * 4. For production OCR integration:
 *    - Replace Serial with your OCR module
 *    - Or use WiFi/Bluetooth to receive text
 * 
 * 5. UTF-8 Handling:
 *    - For proper Russian/Kazakh support, implement UTF-8 parsing
 *    - Current code is simplified for demonstration
 *    - Add UTF-8 library for production use
 * 
 * ============================================
 * MOSFET DRIVER CIRCUIT (per solenoid)
 * ============================================
 * 
 * ESP32 GPIO → 1kΩ resistor → MOSFET Gate (2N7002 or similar)
 * MOSFET Source → GND
 * MOSFET Drain → Solenoid (-) terminal
 * Solenoid (+) terminal → +5V (external power)
 * Flyback Diode (1N4007) across solenoid (cathode to +5V)
 * 
 * IMPORTANT: Use external 5V power supply for solenoids (3A+)
 *            Connect ESP32 GND to power supply GND (common ground!)
 * 
 * ============================================
 * POWER OPTIMIZATION (Optional)
 * ============================================
 * 
 * To reduce power consumption, implement PWM holding:
 * - Use 100% duty cycle for extension (50ms)
 * - Use 30-50% duty cycle for holding (950ms)
 * - This reduces power by 60-70%
 * 
 * Add to extendDots() after initial extension:
 *   ledcWrite(channel, 128);  // 50% duty cycle
 * 
 */
