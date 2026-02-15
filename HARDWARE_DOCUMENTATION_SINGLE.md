# ESP32 Braille Device - Hardware Implementation Documentation (Single-Chip Version)

## Table of Contents
1. [System Overview](#system-overview)
2. [ESP32 Code Explanation](#esp32-code-explanation)
3. [How the Device Works](#how-the-device-works)
4. [Hardware Setup Guide](#hardware-setup-guide)
5. [Troubleshooting](#troubleshooting)

---

## System Overview

This is a **simplified single-chip** Braille display device using only the ESP32 microcontroller.

### **ESP32** - Complete System (Translation + Output)
- Receives text input from OCR or Serial
- Translates text to Braille patterns
- Supports English, Russian, and Kazakh languages
- **Directly controls 6 solenoids** via GPIO pins
- No additional microcontroller needed

### **Why Single-Chip Design?**

**Advantages:**
✅ Simpler wiring (no inter-chip communication)
✅ Lower cost (one microcontroller instead of two)
✅ Easier to program and debug
✅ Fewer components to fail
✅ ESP32 has enough GPIO pins and processing power

**ESP32 Capabilities:**
- **34 GPIO pins** (more than enough for 6 solenoids)
- **Fast processor** (240 MHz dual-core)
- **Built-in WiFi/Bluetooth** (for wireless OCR input)
- **Multiple Serial ports** (for OCR modules)
- **PWM channels** (for power optimization)

---

## ESP32 Code Explanation

### 1. **Complete Architecture**

The ESP32 handles **everything** in one chip:

```
Input → Translation → Pattern Generation → GPIO Control → Solenoids
  ↓         ↓              ↓                    ↓              ↓
Serial   Arrays        6-bit pattern      digitalWrite()   Physical
/OCR     Lookup                                              Output
```

### 2. **GPIO Pin Assignment**

```cpp
#define SOLENOID_1 GPIO_NUM_25  // Dot 1 (top left)
#define SOLENOID_2 GPIO_NUM_26  // Dot 2 (middle left)
#define SOLENOID_3 GPIO_NUM_27  // Dot 3 (bottom left)
#define SOLENOID_4 GPIO_NUM_32  // Dot 4 (top right)
#define SOLENOID_5 GPIO_NUM_33  // Dot 5 (middle right)
#define SOLENOID_6 GPIO_NUM_14  // Dot 6 (bottom right)
```

**Pin Selection Rationale:**
- GPIO 25-27, 32-33: Standard output pins
- GPIO 14: Safe for output (no boot issues)
- All pins support 3.3V output for MOSFET control
- Not using GPIO 0, 2, 12, 15 (boot mode pins)

**Physical Braille Layout:**
```
●  ●    (Dot 1)  (Dot 4)    GPIO25  GPIO32
●  ●    (Dot 2)  (Dot 5)    GPIO26  GPIO33
●  ●    (Dot 3)  (Dot 6)    GPIO27  GPIO14
```

### 3. **Array-Based Translation (Same as Before)**

The translation logic remains **identical** to the two-chip version:

```cpp
const BraillePattern ENGLISH_LOWER[26];  // a-z
const BraillePattern RUSSIAN_CHARS[33];  // а-я
const BraillePattern KAZAKH_CHARS[9];    // ә,ғ,қ,ң,ө,ұ,ү,һ,і
const BraillePattern NUMBERS[10];        // 0-9
const BraillePattern PUNCTUATION[12];    // . , ? ! etc
```

**Lookup Function:**
```cpp
uint8_t getBraillePattern(char c) {
    // Same logic as before
    // Returns 6-bit pattern for any character
}
```

**Example:**
```cpp
getBraillePattern('h') → 0b010011
                          ││││││
                          │││││└─ Bit 0 = 1 → Dot 1
                          ││││└── Bit 1 = 1 → Dot 2
                          │││└─── Bit 2 = 0 → Dot 3
                          ││└──── Bit 3 = 0 → Dot 4
                          │└───── Bit 4 = 1 → Dot 5
                          └────── Bit 5 = 0 → Dot 6
```

### 4. **Direct GPIO Control**

**Key Function: `extendDots()`**

```cpp
void extendDots(uint8_t pattern) {
    for (int i = 0; i < 6; i++) {
        if (pattern & (1 << i)) {
            digitalWrite(SOLENOID_PINS[i], HIGH);  // Extend dot
        } else {
            digitalWrite(SOLENOID_PINS[i], LOW);   // Keep retracted
        }
    }
}
```

**How It Works:**

For pattern 0b010011 (letter 'h'):

```
i=0: pattern & 0b000001 = 1  → GPIO25 = HIGH (Dot 1 extends)
i=1: pattern & 0b000010 = 1  → GPIO26 = HIGH (Dot 2 extends)
i=2: pattern & 0b000100 = 0  → GPIO27 = LOW  (Dot 3 stays down)
i=3: pattern & 0b001000 = 0  → GPIO32 = LOW  (Dot 4 stays down)
i=4: pattern & 0b010000 = 1  → GPIO33 = HIGH (Dot 5 extends)
i=5: pattern & 0b100000 = 0  → GPIO14 = LOW  (Dot 6 stays down)
```

**Result:**
```
Physical Output:
●  ○    Dots 1,2,5 raised
●  ●    = letter 'h'
○  ○
```

### 5. **Display Sequence**

```cpp
void displayBraillePattern(uint8_t pattern) {
    // 1. Retract all (50ms) - Clear previous character
    retractAllDots();
    delay(RETRACT_TIME);
    
    // 2. Extend specified dots (instant)
    extendDots(pattern);
    
    // 3. Hold for extension (50ms) - Full power to push out
    delay(SOLENOID_EXTEND_TIME);
    
    // 4. Hold for reading (950ms) - User feels the pattern
    delay(SOLENOID_HOLD_TIME);
    
    // 5. Retract all - Ready for next character
    retractAllDots();
}
```

**Timing Breakdown:**

| Phase | Duration | GPIO State | Purpose |
|-------|----------|------------|---------|
| Retract | 50ms | All LOW | Clear previous |
| Extend | Instant | Set per pattern | Position solenoids |
| Power | 50ms | Hold HIGH | Push out fully |
| Hold | 950ms | Hold HIGH | User reads |
| **Total** | **1050ms** | → LOW | Next character |

### 6. **Self-Test Sequence**

```cpp
void testAllSolenoids() {
    // Test each solenoid individually (visual feedback)
    for (int i = 0; i < 6; i++) {
        digitalWrite(SOLENOID_PINS[i], HIGH);  // Extend
        delay(200);
        digitalWrite(SOLENOID_PINS[i], LOW);   // Retract
        delay(100);
    }
    
    // Test all together (verify power supply)
    for (int i = 0; i < 6; i++) {
        digitalWrite(SOLENOID_PINS[i], HIGH);
    }
    delay(300);
    retractAllDots();
}
```

**What This Tests:**
1. **Individual solenoids:** Verifies wiring and mechanical operation
2. **Sequential pattern:** Easy to see which dot is which
3. **All together:** Tests power supply under maximum load
4. **Runs on boot:** Immediate feedback that system is working

---

## How the Device Works

### **Complete Data Flow (Simplified)**

```
┌─────────────────────────────────────────────────┐
│              ESP32 COMPLETE SYSTEM               │
│                                                  │
│  ┌────────────────────────────────────────────┐ │
│  │  Step 1: INPUT                             │ │
│  │  • Serial/OCR receives text                │ │
│  │  • Buffer accumulates characters           │ │
│  └────────────┬───────────────────────────────┘ │
│               │                                  │
│               ▼                                  │
│  ┌────────────────────────────────────────────┐ │
│  │  Step 2: PROCESSING                        │ │
│  │  • Detect language (EN/RU/KZ)             │ │
│  │  • Handle capitals & numbers               │ │
│  └────────────┬───────────────────────────────┘ │
│               │                                  │
│               ▼                                  │
│  ┌────────────────────────────────────────────┐ │
│  │  Step 3: TRANSLATION                       │ │
│  │  • Array lookup                            │ │
│  │  • Generate 6-bit pattern                  │ │
│  └────────────┬───────────────────────────────┘ │
│               │                                  │
│               ▼                                  │
│  ┌────────────────────────────────────────────┐ │
│  │  Step 4: GPIO CONTROL                      │ │
│  │  • Set 6 pins HIGH/LOW                     │ │
│  │  • Control timing sequence                 │ │
│  └────────────┬───────────────────────────────┘ │
└───────────────┼──────────────────────────────────┘
                │
                │ GPIO signals (3.3V)
                ▼
┌─────────────────────────────────────────────────┐
│              MOSFET DRIVER ARRAY                 │
│  (Amplifies 3.3V signals to drive solenoids)    │
└────────────┬────────────────────────────────────┘
             │
             │ Power flow (5V)
             ▼
┌─────────────────────────────────────────────────┐
│               SOLENOID ARRAY                     │
│         ●──●    (1) (4)                          │
│         ●──●    (2) (5)                          │
│         ●──●    (3) (6)                          │
└─────────────────────────────────────────────────┘
```

### **Example: Displaying "Hi"**

**Input:** User types "Hi"

**Character 1: 'H' (uppercase)**

1. **ESP32 detects:** Capital letter
2. **Send capital indicator pattern:** 0b100000
3. **GPIO control:**
   - GPIO14 = HIGH (Dot 6 only)
   - All others = LOW
4. **Physical:** Only dot 6 raises (capital marker)
5. **Wait:** 1000ms
6. **Retract all**

7. **ESP32 translates:** 'h' → lookup → 0b010011
8. **GPIO control:**
   - GPIO25 = HIGH (Dot 1)
   - GPIO26 = HIGH (Dot 2)
   - GPIO33 = HIGH (Dot 5)
   - Others = LOW
9. **Physical:** Dots 1,2,5 raise (letter 'h')
10. **Wait:** 1000ms

**Character 2: 'i'**

1. **ESP32 translates:** 'i' → 0b001010
2. **GPIO control:**
   - GPIO26 = HIGH (Dot 2)
   - GPIO32 = HIGH (Dot 4)
   - Others = LOW
3. **Physical:** Dots 2,4 raise (letter 'i')
4. **Wait:** 1000ms

**Total time:** ~3 seconds for "Hi" (capital marker + H + i)

### **Comparison: Single-Chip vs Two-Chip**

| Aspect | Two-Chip (ESP32+ATtiny) | Single-Chip (ESP32 only) |
|--------|------------------------|-------------------------|
| Components | 2 microcontrollers | 1 microcontroller |
| Wiring | UART connection needed | Direct GPIO |
| Programming | Upload 2 codes | Upload 1 code |
| Debugging | Check UART communication | Check GPIO directly |
| Cost | Higher | Lower |
| Complexity | More complex | Simpler |
| **Best For** | Large systems, modularity | Most projects, simplicity |

---

## Hardware Setup Guide

### **Required Components**

#### **Microcontroller:**
- 1× ESP32 DevKit (any variant with sufficient GPIO)

#### **Solenoids & Drivers:**
- 6× Push-pull solenoids (5V, 200-500mA each)
- 6× N-channel MOSFETs (2N7002, IRLZ44N, or similar)
- 6× 1kΩ resistors (gate resistors)
- 6× 1N4007 diodes (flyback protection)

#### **Power:**
- 1× 5V power supply (4A minimum for solenoids)
- Optional: 3.3V regulator if powering ESP32 separately

#### **Connectivity:**
- Jumper wires
- Breadboard or custom PCB

### **Complete Wiring Diagram**

```
                    ┌─────────────────┐
                    │  5V POWER       │
                    │  SUPPLY (4A)    │
                    └────┬────────────┘
                         │
              ┌──────────┴─────────┐
              │                    │
              ▼                    ▼
         ┌────────┐          ┌─────────┐
         │  +5V   │          │  GND    │
         └───┬────┘          └────┬────┘
             │                    │
    ┌────────┴────────────────────┴────────┐
    │     ESP32 + SOLENOID SYSTEM          │
    └──────────────────────────────────────┘

ESP32 Connections:

VIN  ────→ +5V (or use USB power for ESP32)
GND  ────→ GND (CRITICAL: Common ground with solenoid power!)

GPIO25 ──[1kΩ]── MOSFET1 Gate ── Solenoid1 ── +5V
GPIO26 ──[1kΩ]── MOSFET2 Gate ── Solenoid2 ── +5V
GPIO27 ──[1kΩ]── MOSFET3 Gate ── Solenoid3 ── +5V
GPIO32 ──[1kΩ]── MOSFET4 Gate ── Solenoid4 ── +5V
GPIO33 ──[1kΩ]── MOSFET5 Gate ── Solenoid5 ── +5V
GPIO14 ──[1kΩ]── MOSFET6 Gate ── Solenoid6 ── +5V

All MOSFET Sources → GND
All MOSFET Drains → Solenoid negative terminals
All Solenoid positive terminals → +5V
Flyback diodes across each solenoid
```

### **MOSFET Driver Circuit (Detailed)**

**For EACH of the 6 solenoids:**

```
ESP32 GPIOxx
      │
      └──[1kΩ]───┬─── Gate (MOSFET)
                 │
                GND

+5V External
    │
    ├─── Solenoid (+)
    │         │
   ═══       ═══
   │││ Diode │││ 1N4007 (Flyback protection)
   ═══       ═══
    │         │
   GND   Solenoid (-)
              │
            Drain (MOSFET)
              │
          [N-Channel]
          [MOSFET]
              │
           Source
              │
             GND
```

**Component Placement:**
1. **Gate Resistor (1kΩ):** Between ESP32 GPIO and MOSFET gate
   - Limits current
   - Protects ESP32 pin
   
2. **Flyback Diode (1N4007):** Across solenoid
   - Cathode (stripe) to +5V
   - Anode to solenoid negative/MOSFET drain
   - Protects against voltage spikes

3. **MOSFET:** N-channel, logic-level
   - Gate from ESP32 (via 1kΩ)
   - Drain to solenoid negative
   - Source to GND

### **Power Considerations**

**Critical: Common Ground**
```
ESP32 GND ←──────→ Solenoid Power GND
          └──────→ All MOSFET Sources
```

**ALL grounds MUST connect together!** Without common ground:
- MOSFETs won't switch properly
- Solenoids won't activate
- Possible damage to components

**Power Budget:**

| Component | Current | Notes |
|-----------|---------|-------|
| ESP32 | 150mA (active) | Can be powered via USB or VIN |
| Solenoid × 1 | 200-500mA | When active |
| All 6 solenoids | 3A max | Worst case: all active |
| **Total Required** | **3.5A** | Use 4A supply for safety margin |

**Power Supply Options:**

1. **USB + Separate 5V for solenoids (Recommended for testing)**
   - ESP32 powered via USB (programming + power)
   - 5V/4A supply for solenoids only
   - Common ground between both

2. **Single 5V/4A supply (Production)**
   - Powers both ESP32 (via VIN) and solenoids
   - Simpler wiring
   - Use good quality regulated supply

### **Breadboard Layout Example**

```
ESP32          Breadboard         Solenoids
DevKit         Power Rails        (via MOSFETs)

[USB]          +5V ═══════        [Sol 1-6]
  │                                   │
VIN ───────────┘                      │
GND ─────────── GND ═══════ ──────────┘
  │                                (Common GND)
GPIO25 ─────── [1kΩ] ─── MOS1 ─── Sol1
GPIO26 ─────── [1kΩ] ─── MOS2 ─── Sol2
GPIO27 ─────── [1kΩ] ─── MOS3 ─── Sol3
GPIO32 ─────── [1kΩ] ─── MOS4 ─── Sol4
GPIO33 ─────── [1kΩ] ─── MOS5 ─── Sol5
GPIO14 ─────── [1kΩ] ─── MOS6 ─── Sol6
```

---

## Uploading and Testing

### **Step 1: Upload Code**

```bash
# Arduino IDE
1. Open ESP32_BrailleDevice_Complete.ino
2. Tools → Board → ESP32 Dev Module
3. Tools → Port → (select your ESP32)
4. Tools → Upload Speed → 115200
5. Click Upload

# PlatformIO (alternative)
1. Create new project with ESP32 board
2. Copy code to src/main.cpp
3. platformio run --target upload
```

### **Step 2: Open Serial Monitor**

```bash
Baud Rate: 115200
Line Ending: Newline

You should see:
=================================
ESP32 Braille Display Device
=================================
Running self-test...
Testing each solenoid individually...
  Testing Dot 1 (GPIO25)...
  Testing Dot 2 (GPIO26)...
  ...
```

**During self-test:**
- Each solenoid should activate individually
- Then all 6 together
- Listen/feel for mechanical activation

### **Step 3: Test Text Input**

**In Serial Monitor, type:**
```
hello
```
**Press Enter**

**You should see:**
```
--- Processing Text ---
Input: hello
--- Translating to Braille ---
Character: 'h' → Pattern: 0b10011 (dots: 1,2,5)
Character: 'e' → Pattern: 0b10001 (dots: 1,5)
Character: 'l' → Pattern: 0b111 (dots: 1,2,3)
Character: 'l' → Pattern: 0b111 (dots: 1,2,3)
Character: 'o' → Pattern: 0b10101 (dots: 1,3,5)
--- Complete ---
```

**And solenoids display each character for 1 second**

### **Step 4: Test Multiple Languages**

**English:**
```
Hello World
```

**Russian:**
```
Привет
```

**Kazakh:**
```
Сәлем
```

---

## Troubleshooting

### **Issue 1: No Solenoids Activate**

**Symptoms:**
- Self-test shows nothing
- No clicking/movement from solenoids

**Check:**
1. ✅ Power supply connected and ON?
2. ✅ 5V present at solenoid power rail?
3. ✅ Common ground connected? (ESP32 GND to power supply GND)
4. ✅ MOSFETs oriented correctly?
5. ✅ Code uploaded successfully?

**Test:**
```cpp
// Add to loop() for manual test
digitalWrite(SOLENOID_1, HIGH);
delay(1000);
digitalWrite(SOLENOID_1, LOW);
delay(1000);
```

### **Issue 2: Wrong Dots Activate**

**Symptoms:**
- Self-test works but wrong patterns display
- Letter 'a' shows wrong dots

**Check:**
1. ✅ Solenoid physical positions match code?
2. ✅ Wiring matches pin assignments?
3. ✅ SOLENOID_PINS array in correct order?

**Fix:**
Remap pins in code to match your physical layout:
```cpp
const uint8_t SOLENOID_PINS[6] = {
    SOLENOID_1,  // Change order to match
    SOLENOID_2,  // your physical layout
    // ...
};
```

### **Issue 3: Solenoids Too Weak**

**Symptoms:**
- Solenoids barely move
- Some work, others don't
- All fail when activated together

**Check:**
1. ✅ Power supply current rating (needs 4A minimum)
2. ✅ Voltage at solenoid = 5V when active?
3. ✅ Wire gauge sufficient? (22-20 AWG recommended)
4. ✅ MOSFET type correct? (Logic-level, Rds(on) < 0.1Ω)

**Solutions:**
- Use shorter, thicker wires
- Upgrade power supply to higher current
- Use MOSFETs with lower Rds(on)
- Test solenoids individually (disconnect 5 to test 1)

### **Issue 4: Overheating**

**Symptoms:**
- MOSFETs get hot
- Solenoids get very hot
- Power supply thermal shutdown

**Causes:**
- Solenoids on for too long
- Current too high
- Insufficient heat dissipation

**Solutions:**
```cpp
// Reduce holding time
#define SOLENOID_HOLD_TIME 500  // From 950ms to 500ms

// Or implement PWM holding current (advanced)
// After initial extension, reduce duty cycle to 40%
```

### **Issue 5: Garbled Serial Output**

**Symptoms:**
- Strange characters in Serial Monitor
- Can't see debug messages

**Check:**
1. ✅ Baud rate = 115200 in both code and Serial Monitor?
2. ✅ USB cable good quality (data + power)?
3. ✅ Correct COM port selected?

**Fix:**
```cpp
// In setup()
Serial.begin(115200);  // Make sure this matches Serial Monitor
```

### **Issue 6: ESP32 Resets/Crashes**

**Symptoms:**
- ESP32 restarts during operation
- Brownout detector triggered

**Causes:**
- Insufficient power to ESP32
- Voltage drop when solenoids activate
- Drawing too much current from ESP32 pins

**Solutions:**
1. Power ESP32 separately from USB during testing
2. Add 1000µF capacitor across power supply output
3. Verify MOSFETs (not drawing current from GPIO)
4. Use external power supply, not USB for final version

---

## Power Optimization (Optional)

### **PWM for Reduced Power Consumption**

Instead of keeping solenoids at 100% power for 950ms, use PWM:

```cpp
// Add at top of file
#define PWM_FREQ 1000      // 1kHz
#define PWM_RESOLUTION 8   // 8-bit (0-255)

// In setup()
for (int i = 0; i < 6; i++) {
    ledcSetup(i, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(SOLENOID_PINS[i], i);
}

// Modified extendDots() for PWM
void extendDots(uint8_t pattern) {
    for (int i = 0; i < 6; i++) {
        if (pattern & (1 << i)) {
            ledcWrite(i, 255);  // 100% for extension
        } else {
            ledcWrite(i, 0);
        }
    }
}

// In displayBraillePattern(), after SOLENOID_EXTEND_TIME:
// Reduce to 40% duty cycle for holding
for (int i = 0; i < 6; i++) {
    if (pattern & (1 << i)) {
        ledcWrite(i, 102);  // 40% duty cycle (102/255)
    }
}
delay(SOLENOID_HOLD_TIME);
```

**Benefits:**
- Reduces power consumption by ~60%
- Less heat generation
- Longer solenoid life
- Battery operation becomes feasible

---

## OCR Integration

### **Serial OCR Module**

```cpp
// Replace Serial with Serial2 for OCR
#define OCR_INPUT_SERIAL Serial2

void setup() {
    Serial.begin(115200);   // Debug
    Serial2.begin(9600);    // OCR input
    // ...
}

void loop() {
    if (Serial2.available()) {
        // Process OCR data
    }
}
```

### **WiFi Text Reception**

```cpp
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

void setup() {
    // Connect to WiFi
    WiFi.begin("SSID", "password");
    
    // Set up web endpoint
    server.on("/braille", HTTP_POST, [](AsyncWebServerRequest *request){
        String text = request->arg("text");
        processText(text);
        request->send(200, "text/plain", "OK");
    });
    
    server.begin();
}
```

### **Bluetooth Text Reception**

```cpp
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

void setup() {
    SerialBT.begin("BrailleDevice");
    // ...
}

void loop() {
    if (SerialBT.available()) {
        String text = SerialBT.readStringUntil('\n');
        processText(text);
    }
}
```

---

## Performance Specifications

### **Speed**
- Characters per second: ~1
- Total display time: 1050ms per character
- Processing time: <1ms per character
- Pattern switching: 50ms

### **Power**
- ESP32 idle: 80mA
- ESP32 active: 150mA
- Per solenoid: 200-500mA
- **Maximum (all 6 on):** 3.15A
- **Typical (3 dots avg):** 1.5A

### **Memory**
- Flash used: ~300KB (plenty of room for expansion)
- RAM used: ~10KB (minimal)
- Available GPIO: 28 remaining pins (for future features)

---

## Future Enhancements

### **Software**
1. **UTF-8 Support** - Proper multi-byte character handling
2. **Grade 2 Braille** - Contracted Braille for faster reading
3. **WiFi Configuration** - Web interface for settings
4. **Text-to-Speech** - Audio feedback
5. **Word Mode** - Display whole words on multiple cells

### **Hardware**
1. **Multiple Cells** - Display entire lines of text
2. **Touch Input** - Buttons for navigation
3. **Battery Power** - Rechargeable with power management
4. **Custom PCB** - Professional form factor
5. **3D Printed Case** - Polished enclosure

---

## Summary

### **Key Points:**

✅ **One chip does it all** - ESP32 handles translation AND control
✅ **Simple wiring** - Direct GPIO to MOSFETs to solenoids  
✅ **Easy to debug** - No inter-chip communication to troubleshoot
✅ **Lower cost** - Half the microcontrollers
✅ **Powerful** - ESP32 has WiFi/Bluetooth for wireless operation
✅ **Scalable** - Plenty of pins for expansion

### **Next Steps:**

1. **Build breadboard prototype** - Test with 1-2 solenoids first
2. **Upload code** - Verify self-test works
3. **Add all solenoids** - Complete 6-dot array
4. **Test translation** - Try English, Russian, Kazakh
5. **Integrate OCR** - Add your input method
6. **Design enclosure** - Make it portable

**This simplified single-chip design is perfect for most Braille display projects!**
