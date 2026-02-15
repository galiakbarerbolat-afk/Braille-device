# ESP32 Braille Device - Complete Project (Single-Chip Version)

## 📦 What's Included - Simplified Version

This package contains everything you need for a **simplified single-chip** Braille display device using only ESP32.

### **Why Single-Chip?**

✅ **Simpler** - No inter-chip communication  
✅ **Cheaper** - One microcontroller instead of two  
✅ **Easier** - One code to upload and debug  
✅ **Powerful** - ESP32 can do both translation AND control  
✅ **Expandable** - WiFi/Bluetooth built-in  

---

## 📂 Files in This Package

| File | Purpose |
|------|---------|
| **braille-converter-ai.jsx** | Web demo (same as before) |
| **ESP32_BrailleDevice_Complete.ino** | Single Arduino code for everything |
| **HARDWARE_DOCUMENTATION_SINGLE.md** | Complete hardware guide |
| **BLOCK_DIAGRAMS_SINGLE.md** | System architecture diagrams |
| **README_SINGLE.md** | This file |

---

## 🎯 System Overview

### **Complete Architecture (Simplified)**

```
TEXT INPUT (OCR/Serial)
         ↓
    ┌─────────────────────┐
    │       ESP32         │
    │  • Translation      │
    │  • GPIO Control     │
    │  • Timing           │
    └─────────┬───────────┘
              │
              │ 6 GPIO Pins
              ▼
      ┌───────────────┐
      │  6 MOSFETs    │
      └───────┬───────┘
              │
              ▼
      ┌───────────────┐
      │  6 Solenoids  │
      │  Physical     │
      │  Braille Dots │
      └───────────────┘
```

### **What Changed from Two-Chip Version?**

| Aspect | Two-Chip | Single-Chip |
|--------|----------|-------------|
| Microcontrollers | ESP32 + ATtiny | ESP32 only |
| Code Files | 2 (.ino files) | 1 (.ino file) |
| Wiring | UART connection | Direct GPIO |
| Upload Steps | Upload twice | Upload once |
| Debugging | Check UART | Check GPIO |
| Cost | ~$15-20 | ~$8-10 |
| **Complexity** | **More** | **Less** |

---

## 🚀 Quick Start Guide

### **Step 1: Get Components**

**Microcontroller:**
- 1× ESP32 DevKit ($5-8)

**Solenoids & Drivers:**
- 6× Push-pull solenoids 5V ($2-4 each)
- 6× N-channel MOSFETs (2N7002 or IRLZ44N)
- 6× 1kΩ resistors
- 6× 1N4007 diodes

**Power:**
- 5V power supply, 4A minimum ($8-12)

**Total Cost:** ~$35-50

### **Step 2: Upload Code**

```bash
1. Open Arduino IDE
2. Install ESP32 board support
3. Open ESP32_BrailleDevice_Complete.ino
4. Select Board: "ESP32 Dev Module"
5. Select Port: (your ESP32 port)
6. Click Upload
```

**That's it!** One upload, done.

### **Step 3: Test**

```bash
1. Open Serial Monitor (115200 baud)
2. Watch self-test (solenoids activate)
3. Type: hello
4. Press Enter
5. Watch each letter display
```

---

## 🔧 Hardware Connections

### **GPIO Pin Assignments**

```
ESP32 Pin → Solenoid (via MOSFET)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
GPIO25 → Solenoid 1 (Dot 1 - top left)
GPIO26 → Solenoid 2 (Dot 2 - middle left)
GPIO27 → Solenoid 3 (Dot 3 - bottom left)
GPIO32 → Solenoid 4 (Dot 4 - top right)
GPIO33 → Solenoid 5 (Dot 5 - middle right)
GPIO14 → Solenoid 6 (Dot 6 - bottom right)
```

### **Complete Wiring Diagram**

```
ESP32 DevKit
├── VIN ← +5V (or use USB)
├── GND ← GND (CRITICAL: Common ground!)
│
├── GPIO25 ─[1kΩ]─→ MOSFET1 → Solenoid1
├── GPIO26 ─[1kΩ]─→ MOSFET2 → Solenoid2
├── GPIO27 ─[1kΩ]─→ MOSFET3 → Solenoid3
├── GPIO32 ─[1kΩ]─→ MOSFET4 → Solenoid4
├── GPIO33 ─[1kΩ]─→ MOSFET5 → Solenoid5
└── GPIO14 ─[1kΩ]─→ MOSFET6 → Solenoid6

5V Power Supply
├── +5V → All solenoid (+) terminals
└── GND → All MOSFET sources + ESP32 GND
```

**MOSFET Circuit (repeat 6×):**
```
ESP32 GPIO → 1kΩ → MOSFET Gate
                      ↓
                   Solenoid
                      ↓
                   Drain
                      ↓
                   Source → GND

Flyback Diode across solenoid:
  Cathode → +5V
  Anode → MOSFET Drain
```

---

## 💻 Code Explanation

### **Main Components**

#### **1. Translation Arrays (Same as before)**

```cpp
const BraillePattern ENGLISH_LOWER[26];
const BraillePattern RUSSIAN_CHARS[33];
const BraillePattern KAZAKH_CHARS[9];
const BraillePattern NUMBERS[10];
const BraillePattern PUNCTUATION[12];
```

**Fast array lookup:**
```cpp
'h' → ENGLISH_LOWER[7] → 0b010011
```

#### **2. Direct GPIO Control (NEW)**

```cpp
void extendDots(uint8_t pattern) {
    for (int i = 0; i < 6; i++) {
        if (pattern & (1 << i)) {
            digitalWrite(SOLENOID_PINS[i], HIGH);
        } else {
            digitalWrite(SOLENOID_PINS[i], LOW);
        }
    }
}
```

**What it does:**
- Takes 6-bit pattern
- Sets each GPIO HIGH or LOW
- Directly controls solenoids
- **No UART, no delays, no communication**

#### **3. Display Sequence**

```cpp
void displayBraillePattern(uint8_t pattern) {
    retractAllDots();         // Clear previous
    delay(50);
    
    extendDots(pattern);      // Set new pattern
    delay(50);                // Push out
    
    delay(950);               // User reads
    
    retractAllDots();         // Ready for next
}
```

**Timing:**
- 50ms: Retract all
- 50ms: Extend specified dots
- 950ms: Hold for reading
- **Total: 1050ms per character**

---

## 📚 How It Works

### **Complete Data Flow**

```
1. USER TYPES: "Hi"
   ↓
2. ESP32 RECEIVES via Serial
   inputBuffer = "Hi"
   ↓
3. PROCESS CHARACTER 'H'
   • Detect: Uppercase
   • Display: Capital indicator (dot 6)
   • Wait: 1000ms
   ↓
4. PROCESS CHARACTER 'H' (after indicator)
   • Lookup: 'h' → 0b010011
   • GPIO Control:
     - GPIO25 = HIGH (Dot 1)
     - GPIO26 = HIGH (Dot 2)  
     - GPIO33 = HIGH (Dot 5)
     - Others = LOW
   • Physical: Dots 1,2,5 extend
   • Wait: 1000ms
   ↓
5. PROCESS CHARACTER 'i'
   • Lookup: 'i' → 0b001010
   • GPIO Control:
     - GPIO26 = HIGH (Dot 2)
     - GPIO32 = HIGH (Dot 4)
     - Others = LOW
   • Physical: Dots 2,4 extend
   • Wait: 1000ms
   ↓
6. DONE
```

### **Example: Letter 'h'**

**Step-by-Step:**

```
Pattern: 0b010011

Bit Analysis:
  Bit 0 = 1 → GPIO25 = HIGH → Solenoid 1 extends
  Bit 1 = 1 → GPIO26 = HIGH → Solenoid 2 extends
  Bit 2 = 0 → GPIO27 = LOW  → Solenoid 3 stays down
  Bit 3 = 0 → GPIO32 = LOW  → Solenoid 4 stays down
  Bit 4 = 1 → GPIO33 = HIGH → Solenoid 5 extends
  Bit 5 = 0 → GPIO14 = LOW  → Solenoid 6 stays down

Physical Result:
  ●  ○    Dots 1,2,5 raised
  ●  ●    = letter 'h' in Braille
  ○  ○
```

---

## 🌍 Multi-Language Support

### **Supported Languages**

**English (26 letters)**
```
hello → ⠓⠑⠇⠇⠕
```

**Russian (33 characters)**
```
привет → ⠏⠗⠊⠧⠑⠞
```

**Kazakh (Russian + 9 special)**
```
сәлем → ⠎⠜⠇⠑⠍
```

### **How Language Detection Works**

The code automatically detects language based on character:

```cpp
// English letter?
if (c >= 'a' && c <= 'z') {
    return ENGLISH_LOWER[c - 'a'].dots;
}

// Russian Cyrillic?
for (int i = 0; i < 33; i++) {
    if (RUSSIAN_CHARS[i].character == c) {
        return RUSSIAN_CHARS[i].dots;
    }
}

// Kazakh-specific?
for (int i = 0; i < 9; i++) {
    if (KAZAKH_CHARS[i].character == c) {
        return KAZAKH_CHARS[i].dots;
    }
}
```

**No configuration needed - just type in any language!**

---

## 🛠️ Customization

### **Change Display Time**

```cpp
// In code, find these lines:
#define SOLENOID_HOLD_TIME 950  // ms

// For faster reading (600ms total):
#define SOLENOID_HOLD_TIME 550  // ms

// For slower reading (2000ms total):
#define SOLENOID_HOLD_TIME 1950  // ms
```

### **Add New Language**

```cpp
// Add your language array:
const BraillePattern FRENCH_CHARS[] = {
    {'à', 0b110001},
    {'é', 0b111001},
    {'è', 0b101001},
    // ... more characters
};

// Add to getBraillePattern() function:
// Check French characters
for (int i = 0; i < sizeof(FRENCH_CHARS) / sizeof(BraillePattern); i++) {
    if (FRENCH_CHARS[i].character == lowerC) {
        return FRENCH_CHARS[i].dots;
    }
}
```

### **Change GPIO Pins**

```cpp
// Don't like the default pins? Change them:
#define SOLENOID_1 GPIO_NUM_4   // Instead of 25
#define SOLENOID_2 GPIO_NUM_5   // Instead of 26
// ... etc

// Just avoid GPIO 0, 2, 12, 15 (boot mode pins)
```

---

## 🔍 Troubleshooting

### **Problem: Solenoids don't activate**

**Check:**
1. ✅ Power supply ON and 5V present?
2. ✅ Common ground connected (ESP32 GND to power GND)?
3. ✅ Code uploaded successfully?
4. ✅ Self-test runs (check Serial Monitor)?

**Test Manually:**
```cpp
// Add to loop() temporarily:
digitalWrite(SOLENOID_1, HIGH);
delay(1000);
digitalWrite(SOLENOID_1, LOW);
delay(1000);
```

### **Problem: Wrong pattern displays**

**Check:**
1. ✅ Solenoid physical positions match pin numbers?
2. ✅ All 6 solenoids connected correctly?
3. ✅ MOSFETs oriented correctly?

**Debug:**
- Run self-test
- Each solenoid activates in order (1→2→3→4→5→6)
- Verify which physical solenoid corresponds to which number

### **Problem: Some solenoids weak/don't work**

**Check:**
1. ✅ Power supply current (needs 4A minimum)
2. ✅ Wire gauge (use 22-20 AWG)
3. ✅ MOSFET type (logic-level, Rds(on) < 0.1Ω)
4. ✅ Flyback diode polarity correct

**Test:**
- Disconnect 5 solenoids
- Test one at a time
- If works individually → power supply issue

### **Problem: ESP32 resets**

**Causes:**
- Insufficient power
- Brownout when solenoids activate
- Drawing too much current

**Solutions:**
- Power ESP32 via USB during testing
- Add 1000µF capacitor across power supply
- Use separate regulator for ESP32

---

## 📊 Technical Specifications

### **Performance**

| Metric | Value |
|--------|-------|
| Characters/second | ~1 |
| Display time/char | 1050ms |
| Processing time | <1ms |
| Languages supported | 3 (EN, RU, KZ) |
| Total characters | 70+ |

### **Power Requirements**

| Component | Current | Notes |
|-----------|---------|-------|
| ESP32 active | 150mA | WiFi off |
| Solenoid each | 200-500mA | When active |
| All 6 solenoids | 3A max | Worst case |
| **Total required** | **3.5A** | Use 4A supply |

### **Memory Usage**

| Type | Used | Available |
|------|------|-----------|
| Flash | ~300KB | ~3.7MB |
| RAM | ~10KB | ~510KB |
| GPIO | 6 pins | 28 pins |

**Plenty of room for expansion!**

---

## 🎓 Learning Path

### **Beginner Path**

1. ✅ Try the web demo (`braille-converter-ai.jsx`)
2. ✅ Understand Braille conversion
3. ✅ Read: HARDWARE_DOCUMENTATION_SINGLE.md → Overview
4. ✅ View: BLOCK_DIAGRAMS_SINGLE.md → Diagram 1
5. ✅ Upload code and test with Serial Monitor

### **Intermediate Path**

1. ✅ Build breadboard prototype (1 solenoid first)
2. ✅ Study the code
3. ✅ Read full documentation
4. ✅ Add all 6 solenoids
5. ✅ Test full character set

### **Advanced Path**

1. ✅ Add OCR input module
2. ✅ Implement WiFi text reception
3. ✅ Add PWM for power optimization
4. ✅ Design custom PCB
5. ✅ Create professional enclosure

---

## 🔌 OCR Integration Examples

### **Serial OCR Module**

```cpp
// Use Serial2 for OCR
void setup() {
    Serial.begin(115200);   // Debug
    Serial2.begin(9600);    // OCR module
    setupPins();
    testAllSolenoids();
}

void loop() {
    if (Serial2.available()) {
        char c = Serial2.read();
        // Process OCR character
        if (c == '\n') {
            processText(inputBuffer);
            inputBuffer = "";
        } else {
            inputBuffer += c;
        }
    }
}
```

### **WiFi Text Reception**

```cpp
#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

void setup() {
    // ... existing setup ...
    
    WiFi.begin("YourSSID", "password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    
    server.on("/send", HTTP_POST, [](){
        String text = server.arg("text");
        processText(text);
        server.send(200, "text/plain", "OK");
    });
    
    server.begin();
}

void loop() {
    server.handleClient();
    // ... existing loop ...
}
```

Now send text via:
```
http://192.168.1.xxx/send?text=Hello
```

### **Bluetooth Input**

```cpp
#include <BluetoothSerial.h>

BluetoothSerial BT;

void setup() {
    // ... existing setup ...
    BT.begin("BrailleDevice");
}

void loop() {
    if (BT.available()) {
        String text = BT.readStringUntil('\n');
        processText(text);
    }
}
```

Connect from phone and send text!

---

## 🌟 Advanced Features

### **Power Optimization with PWM**

Reduce power consumption by 60%:

```cpp
// Add at top:
#define USE_PWM true

// In setup():
if (USE_PWM) {
    for (int i = 0; i < 6; i++) {
        ledcSetup(i, 1000, 8);  // 1kHz, 8-bit
        ledcAttachPin(SOLENOID_PINS[i], i);
    }
}

// Modified extendDots():
void extendDots(uint8_t pattern, bool holding = false) {
    for (int i = 0; i < 6; i++) {
        if (pattern & (1 << i)) {
            if (USE_PWM) {
                int duty = holding ? 102 : 255;  // 40% or 100%
                ledcWrite(i, duty);
            } else {
                digitalWrite(SOLENOID_PINS[i], HIGH);
            }
        } else {
            if (USE_PWM) {
                ledcWrite(i, 0);
            } else {
                digitalWrite(SOLENOID_PINS[i], LOW);
            }
        }
    }
}

// In displayBraillePattern():
extendDots(pattern, false);  // 100% power
delay(50);
extendDots(pattern, true);   // 40% power (holding)
delay(950);
```

**Benefits:**
- Less heat
- Longer solenoid life
- Battery operation possible

### **Multiple Cells**

To display whole words:

```cpp
// Change pin assignments:
const uint8_t CELL1_PINS[6] = {25, 26, 27, 32, 33, 14};
const uint8_t CELL2_PINS[6] = {4, 5, 18, 19, 21, 22};

void displayWord(String word) {
    // Display first char on cell 1
    displayOnCell(word[0], CELL1_PINS);
    // Display second char on cell 2
    displayOnCell(word[1], CELL2_PINS);
}
```

---

## 📞 Summary & Next Steps

### **What You Have**

✅ **Single Arduino file** - Upload once, done  
✅ **Direct GPIO control** - No complex communication  
✅ **Multi-language** - English, Russian, Kazakh  
✅ **Expandable** - WiFi, Bluetooth, PWM ready  
✅ **Well documented** - Every line explained  

### **Recommended Workflow**

**Week 1:**
- [ ] Get components
- [ ] Build breadboard with 1 solenoid
- [ ] Upload code and test
- [ ] Verify self-test works

**Week 2:**
- [ ] Add remaining 5 solenoids
- [ ] Test full alphabet
- [ ] Try Russian and Kazakh
- [ ] Adjust timing if needed

**Week 3:**
- [ ] Design/build enclosure
- [ ] Add OCR module OR WiFi input
- [ ] Optimize power (PWM)
- [ ] Create PCB (optional)

**Week 4:**
- [ ] Polish and refine
- [ ] Test with real users
- [ ] Document any changes
- [ ] Consider adding features

### **Key Advantages of This Version**

| Feature | Benefit |
|---------|---------|
| Single chip | Simpler, cheaper, easier |
| Direct GPIO | Faster, more reliable |
| ESP32 power | WiFi, Bluetooth, processing |
| One code file | Upload once, debug once |
| Well documented | Easy to understand and modify |

### **When to Use Two-Chip Version**

Only if you need:
- **>30 solenoids** (GPIO limitation)
- **Absolute real-time** guarantees
- **Modular system** for easy swapping
- **Distributed processing**

For typical projects: **This single-chip version is better!**

---

## 🎉 Final Checklist

**Before You Start:**
- [ ] Read this README
- [ ] Read HARDWARE_DOCUMENTATION_SINGLE.md
- [ ] View BLOCK_DIAGRAMS_SINGLE.md
- [ ] Order components

**Building:**
- [ ] Install Arduino IDE + ESP32 support
- [ ] Upload ESP32_BrailleDevice_Complete.ino
- [ ] Test self-test sequence
- [ ] Build driver circuit for 1 solenoid
- [ ] Test with actual text
- [ ] Add remaining solenoids
- [ ] Verify all patterns

**Customizing:**
- [ ] Adjust timing if needed
- [ ] Add your input method (OCR/WiFi/BT)
- [ ] Implement PWM (optional)
- [ ] Design enclosure

**Done!** You have a working Braille display device! 🎊

---

## 📄 File Guide

- **ESP32_BrailleDevice_Complete.ino** - Upload this to ESP32
- **HARDWARE_DOCUMENTATION_SINGLE.md** - Read for details
- **BLOCK_DIAGRAMS_SINGLE.md** - Visual explanations
- **braille-converter-ai.jsx** - Web demo for testing

**Start with the .ino file - everything else supports it!**

---

**Questions? Everything is explained in the documentation. Good luck building! 🚀**
