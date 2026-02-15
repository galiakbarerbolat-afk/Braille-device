# Block Diagrams - ESP32 Braille Device (Single-Chip Version)

## 1. System Architecture Overview (Simplified)

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    COMPLETE SYSTEM FLOW (SIMPLIFIED)                    │
└─────────────────────────────────────────────────────────────────────────┘

                    ┌──────────────────┐
                    │   TEXT INPUT     │
                    │  (OCR / Serial)  │
                    └────────┬─────────┘
                             │
                             │ Text String
                             │ "Hello World"
                             ▼
        ┌────────────────────────────────────────────────────┐
        │              ESP32 COMPLETE SYSTEM                 │
        │          (Translation + Solenoid Control)          │
        │                                                    │
        │  ┌──────────────────────────────────────────────┐  │
        │  │  Text Reception & Buffering                  │  │
        │  │  Serial.available() → inputBuffer            │  │
        │  └────────────┬─────────────────────────────────┘  │
        │               │                                    │
        │               ▼                                    │
        │  ┌──────────────────────────────────────────────┐  │
        │  │  Character-by-Character Processing           │  │
        │  │  • Detect Language (EN/RU/KZ)                │  │
        │  │  • Handle Capitals & Numbers                 │  │
        │  └────────────┬─────────────────────────────────┘  │
        │               │                                    │
        │               ▼                                    │
        │  ┌──────────────────────────────────────────────┐  │
        │  │  Array Lookup                                │  │
        │  │  • ENGLISH_LOWER[26]                         │  │
        │  │  • RUSSIAN_CHARS[33]                         │  │
        │  │  • KAZAKH_CHARS[9]                           │  │
        │  │  • NUMBERS[10] + PUNCTUATION[12]             │  │
        │  └────────────┬─────────────────────────────────┘  │
        │               │                                    │
        │               ▼                                    │
        │  ┌──────────────────────────────────────────────┐  │
        │  │  6-Bit Pattern Generation                    │  │
        │  │  Example: 'h' → 0b010011                     │  │
        │  └────────────┬─────────────────────────────────┘  │
        │               │                                    │
        │               ▼                                    │
        │  ┌──────────────────────────────────────────────┐  │
        │  │  GPIO Pin Control                            │  │
        │  │  • GPIO25 (Dot 1)                            │  │
        │  │  • GPIO26 (Dot 2)                            │  │
        │  │  • GPIO27 (Dot 3)                            │  │
        │  │  • GPIO32 (Dot 4)                            │  │
        │  │  • GPIO33 (Dot 5)                            │  │
        │  │  • GPIO14 (Dot 6)                            │  │
        │  │  digitalWrite(pin, HIGH/LOW)                 │  │
        │  └────────────┬─────────────────────────────────┘  │
        │               │                                    │
        │               ▼                                    │
        │  ┌──────────────────────────────────────────────┐  │
        │  │  Timing Control                              │  │
        │  │  • Retract: 50ms                             │  │
        │  │  • Extend: 50ms                              │  │
        │  │  • Hold: 950ms                               │  │
        │  └────────────┬─────────────────────────────────┘  │
        └───────────────┼────────────────────────────────────┘
                        │
                        │ 6 GPIO Signals (3.3V)
                        │ GPIO25, 26, 27, 32, 33, 14
                        ▼
        ┌────────────────────────────────────────────────┐
        │          MOSFET DRIVER ARRAY (×6)              │
        │                                                │
        │  ┌──────┐  ┌──────┐  ┌──────┐                  │
        │  │ MOS1 │  │ MOS2 │  │ MOS3 │                  │
        │  └──┬───┘  └──┬───┘  └──┬───┘                  │
        │     │         │         │                      │
        │  ┌──────┐  ┌──────┐  ┌──────┐                  │
        │  │ MOS4 │  │ MOS5 │  │ MOS6 │                  │
        │  └──┬───┘  └──┬───┘  └──┬───┘                  │
        └─────┼─────────┼─────────┼──────────────────────┘
              │         │         │
              │ Control Solenoids (5V power) │
              ▼         ▼         ▼
        ┌────────────────────────────────────────────┐
        │           SOLENOID ARRAY                   │
        │                                            │
        │         ●──●    (1) (4)                    │
        │         │  │                               │
        │         ●──●    (2) (5)                    │
        │         │  │                               │
        │         ●──●    (3) (6)                    │
        │                                            │
        │    Dots 1,2,5 raised = letter 'h'          │
        └────────────────────────────────────────────┘
                        │
                        │ Physical Tactile Output
                        ▼
                  ┌──────────┐
                  │   USER   │
                  │  READING │
                  └──────────┘

```

---

## 2. ESP32 Code Flow Diagram

```
┌────────────────────────────────────────────────────────────────────────┐
│                    ESP32 COMPLETE PROCESSING FLOW                       │
└────────────────────────────────────────────────────────────────────────┘

START
  │
  ▼
┌─────────────────────┐
│  Initialize System  │
│  • Setup GPIO pins  │
│  • Configure Serial │
│  • Run self-test    │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│  Wait for Input     │
│  Serial.available() │
│  Buffer characters  │
└──────────┬──────────┘
           │
           │ Character received
           ▼
      ┌────────┐
      │ '\n' ? │──── NO ──→ Add to buffer ──┐
      └────┬───┘                             │
           │ YES                             │
           ▼                                 │
    ┌──────────────┐                        │
    │ Process Text │                        │
    └──────┬───────┘                        │
           │                                 │
           ▼                                 │
    ┌─────────────────────┐                 │
    │ FOR each character  │←────────────────┘
    │ in buffer           │
    └──────┬──────────────┘
           │
           ▼
    ┌───────────────┐
    │ Is Uppercase? │─── YES ──→ ┌─────────────────────┐
    └───────┬───────┘            │ Display CAPITAL_IND │
            │ NO                 │ (dot 6 only)        │
            │                    │ Wait 1000ms         │
            │                    └──────────┬──────────┘
            ▼                               │
    ┌───────────────┐                       │
    │ Is Number?    │─── YES ──→ ┌──────────┴──────────┐
    └───────┬───────┘            │ Display NUMBER_IND  │
            │ NO                 │ (dots 1,2,3,4)      │
            │                    │ Set numberMode=true │
            │                    │ Wait 1000ms         │
            │                    └──────────┬──────────┘
            ▼                               │
    ┌──────────────────────┐                │
    │ Array Lookup         │ ←──────────────┘
    │ getBraillePattern(c) │
    │ Returns: 6-bit value │
    └────────┬─────────────┘
             │
             ▼
    ┌────────────────────┐
    │ Display Pattern    │
    │ ┌────────────────┐ │
    │ │ 1. Retract all │ │
    │ │ 2. Extend dots │ │
    │ │ 3. Hold 1000ms │ │
    │ │ 4. Retract all │ │
    │ └────────────────┘ │
    └────────┬───────────┘
             │
             ▼
    ┌─────────────────────┐
    │ More characters?    │─── YES ──→ Loop back
    └────────┬────────────┘
             │ NO
             ▼
    ┌─────────────────────┐
    │ Clear buffer        │
    │ Ready for next text │
    └────────┬────────────┘
             │
             ▼
           DONE

## 3. GPIO Control Detail

```
┌────────────────────────────────────────────────────────────────────────┐
│              GPIO PIN CONTROL - DETAILED FLOW                           │
└────────────────────────────────────────────────────────────────────────┘

displayBraillePattern(0b010011)  // Example: letter 'h'
        │
        ▼
┌─────────────────────┐
│ Step 1: RETRACT ALL │
│                     │
│ FOR i = 0 to 5      │
│   GPIO[i] = LOW     │
│                     │
│ Result:             │
│   GPIO25 = LOW      │
│   GPIO26 = LOW      │
│   GPIO27 = LOW      │
│   GPIO32 = LOW      │
│   GPIO33 = LOW      │
│   GPIO14 = LOW      │
│                     │
│ delay(50ms)         │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────────────┐
│ Step 2: EXTEND DOTS         │
│                             │
│ extendDots(0b010011)        │
│                             │
│ FOR i = 0 to 5              │
│   IF pattern & (1 << i)     │
│     GPIO[i] = HIGH          │
│   ELSE                      │
│     GPIO[i] = LOW           │
│                             │
│ Bit Analysis:               │
│   Bit 0 = 1 → GPIO25 = HIGH │
│   Bit 1 = 1 → GPIO26 = HIGH │
│   Bit 2 = 0 → GPIO27 = LOW  │
│   Bit 3 = 0 → GPIO32 = LOW  │
│   Bit 4 = 1 → GPIO33 = HIGH │
│   Bit 5 = 0 → GPIO14 = LOW  │
│                             │
│ Physical Result:            │
│   Solenoid 1: EXTENDS       │
│   Solenoid 2: EXTENDS       │
│   Solenoid 3: retracted     │
│   Solenoid 4: retracted     │
│   Solenoid 5: EXTENDS       │
│   Solenoid 6: retracted     │
└──────────┬──────────────────┘
           │
           ▼
┌─────────────────────┐
│ Step 3: POWER PUSH  │
│                     │
│ delay(50ms)         │
│                     │
│ Full power applied  │
│ Solenoids extend    │
│ fully               │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Step 4: HOLD        │
│                     │
│ delay(950ms)        │
│                     │
│ Maintain HIGH state │
│ User reads pattern  │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Step 5: RETRACT     │
│                     │
│ All GPIO = LOW      │
│                     │
│ Ready for next char │
└─────────────────────┘

TIMING SUMMARY:
━━━━━━━━━━━━━━━━━━━━━━━━
  0ms  : All LOW (retract)
 50ms  : Set pattern
100ms  : Full extension
1000ms : Still holding
1050ms : All LOW (done)
```

---

## 4. Data Flow - Single Character Example

```
┌────────────────────────────────────────────────────────────────────────┐
│         COMPLETE DATA FLOW: Displaying Letter 'h' (Single-Chip)        │
└────────────────────────────────────────────────────────────────────────┘

USER INPUT: "h"
     │
     ▼
┌──────────────────────┐
│  Serial Receives     │
│  char c = 'h'        │
│  inputBuffer += 'h'  │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────────┐
│  User presses ENTER      │
│  Triggers processing     │
└──────────┬───────────────┘
           │
           ▼
┌─────────────────────────────┐
│  Classify Character         │
│  • Not uppercase            │
│  • Not number               │
│  • Is English letter        │
│  • Is lowercase             │
└──────────┬──────────────────┘
           │
           ▼
┌─────────────────────────────┐
│  Array Lookup               │
│  index = 'h' - 'a' = 7      │
│  ENGLISH_LOWER[7].dots      │
│  = 0b010011                 │
└──────────┬──────────────────┘
           │
           ▼
┌─────────────────────────────┐
│  GPIO Control Begins        │
│  displayBraillePattern()    │
└──────────┬──────────────────┘
           │
           ▼
┌─────────────────────────────┐
│  RETRACT Phase (50ms)       │
│  All 6 GPIO = LOW           │
│                             │
│  GPIO25 = LOW               │
│  GPIO26 = LOW               │
│  GPIO27 = LOW               │
│  GPIO32 = LOW               │
│  GPIO33 = LOW               │
│  GPIO14 = LOW               │
└──────────┬──────────────────┘
           │
           ▼
┌─────────────────────────────┐
│  EXTEND Phase (instant)     │
│  Pattern: 0b010011          │
│                             │
│  GPIO25 = HIGH (Dot 1) ✓    │
│  GPIO26 = HIGH (Dot 2) ✓    │
│  GPIO27 = LOW  (Dot 3)      │
│  GPIO32 = LOW  (Dot 4)      │
│  GPIO33 = HIGH (Dot 5) ✓    │
│  GPIO14 = LOW  (Dot 6)      │
└──────────┬──────────────────┘
           │
           │ 3.3V GPIO signals
           ▼
┌─────────────────────────────────┐
│  MOSFET Driver Array            │
│                                 │
│  MOSFET1: Gate HIGH → ON        │
│  MOSFET2: Gate HIGH → ON        │
│  MOSFET3: Gate LOW  → OFF       │
│  MOSFET4: Gate LOW  → OFF       │
│  MOSFET5: Gate HIGH → ON        │
│  MOSFET6: Gate LOW  → OFF       │
└──────────┬──────────────────────┘
           │
           │ 5V Power switched
           ▼
┌─────────────────────────────────┐
│  Solenoid Activation            │
│                                 │
│  Solenoid 1: +5V → EXTENDS      │
│  Solenoid 2: +5V → EXTENDS      │
│  Solenoid 3:  0V → retracted    │
│  Solenoid 4:  0V → retracted    │
│  Solenoid 5: +5V → EXTENDS      │
│  Solenoid 6:  0V → retracted    │
└──────────┬──────────────────────┘
           │
           ▼
┌─────────────────────────────────┐
│  Physical Braille Pattern       │
│                                 │
│       ●  ○   ← Dots 1,4        │
│       │  │                      │
│       ●  ●   ← Dots 2,5        │
│       │  │                      │
│       ○  ○   ← Dots 3,6        │
│                                 │
│  Raised: 1, 2, 5 = letter 'h'  │
└──────────┬──────────────────────┘
           │
           ▼
┌─────────────────────────────────┐
│  HOLD Phase (1000ms)            │
│  GPIO state maintained          │
│  User feels the pattern         │
└──────────┬──────────────────────┘
           │
           ▼
┌─────────────────────────────────┐
│  RETRACT Phase                  │
│  All GPIO = LOW                 │
│  All solenoids retract          │
│  Ready for next character       │
└─────────────────────────────────┘

TOTAL TIME: ~1100ms
  • 50ms retract
  • 50ms extend
  • 950ms hold
  • 50ms final retract

## 5. Hardware Wiring Diagram

```
┌────────────────────────────────────────────────────────────────────────┐
│              PHYSICAL WIRING - ESP32 ONLY VERSION                       │
└────────────────────────────────────────────────────────────────────────┘

                    ┌─────────────────┐
                    │  5V POWER       │
                    │  SUPPLY (4A)    │
                    └────┬───────┬────┘
                         │       │
                     +5V │       │ GND
                         │       │
       ┌─────────────────┴───────┴─────────────────┐
       │                                            │
       │         POWER DISTRIBUTION                 │
       │                                            │
       │  +5V Rail ═══════════ GND Rail ═══════    │
       └──┬───┬───┬───┬───┬───┬──────┬────────────┘
          │   │   │   │   │   │      │
          │   │   │   │   │   │      │
       ┌──┴───┴───┴───┴───┴───┴──┐   │
       │  Solenoid Power (×6)    │   │
       │  All (+) terminals       │   │
       └──┬───┬───┬───┬───┬───┬──┘   │
          │   │   │   │   │   │      │
          ▼   ▼   ▼   ▼   ▼   ▼      ▼
       [Sol1][Sol2][Sol3][Sol4][Sol5][Sol6]
          │   │   │   │   │   │
         ═══ ═══ ═══ ═══ ═══ ═══   Flyback
          │   │   │   │   │   │    Diodes
          │   │   │   │   │   │
          ▼   ▼   ▼   ▼   ▼   ▼
       [MOS1][MOS2][MOS3][MOS4][MOS5][MOS6]
          │   │   │   │   │   │
          └───┴───┴───┴───┴───┴───────→ GND
          ▲   ▲   ▲   ▲   ▲   ▲
          │   │   │   │   │   │
       1kΩ 1kΩ 1kΩ 1kΩ 1kΩ 1kΩ  Gate Resistors
          │   │   │   │   │   │
          ▲   ▲   ▲   ▲   ▲   ▲
       ┌──┴───┴───┴───┴───┴───┴──┐
       │      ESP32 DevKit        │
       │                          │
       │  GPIO25  GPIO26  GPIO27  │
       │  GPIO32  GPIO33  GPIO14  │
       │                          │
       │  VIN ← +5V (optional)    │
       │  GND ← GND (CRITICAL!)   │
       │  USB ← Programming       │
       └──────────────────────────┘


DETAILED MOSFET CIRCUIT (Example: Solenoid 1)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

ESP32 GPIO25
      │
      └──[1kΩ]───┬─── Gate (MOSFET)
                 │
                GND

+5V Power Supply
    │
    ├────[Solenoid 1]────┬─── Drain (MOSFET)
    │                    │
   ═══                  ═══
   │││ 1N4007           │││  
   ═══ (Flyback)        ═══
    │                    │
   GND              [N-Channel]
                    [  MOSFET  ]
                         │
                      Source
                         │
                        GND


CRITICAL CONNECTIONS:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
1. ESP32 GND ←──────→ Power Supply GND
2. All MOSFET Sources → GND
3. All Flyback Diode Anodes → MOSFET Drains
4. All Flyback Diode Cathodes → +5V
5. All Solenoid (+) → +5V
6. Gate Resistor between GPIO and MOSFET Gate

WITHOUT COMMON GROUND, NOTHING WILL WORK!


POWER ROUTING:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Option 1: Separate Power (Testing)
  USB → ESP32 (programming + power)
  5V Supply → Solenoids only
  Common: GND connected

Option 2: Single Supply (Production)
  5V Supply → ESP32 VIN + Solenoids
  Common: All GND together
```

---

## 6. State Machine Diagram

```
┌────────────────────────────────────────────────────────────────────────┐
│                    ESP32 STATE MACHINE (SIMPLIFIED)                     │
└────────────────────────────────────────────────────────────────────────┘

                    ┌──────────┐
                    │  START   │
                    └─────┬────┘
                          │
                          ▼
                   ┌─────────────┐
                   │  INIT       │
                   │ Setup GPIO  │
                   │ Setup Serial│
                   │ Self-Test   │
                   └──────┬──────┘
                          │
                          ▼
                   ┌─────────────┐
            ┌─────>│    IDLE     │<──────────────┐
            │      │ Waiting for │                │
            │      │   Serial    │                │
            │      └──────┬──────┘                │
            │             │                       │
            │             │ Data received         │
            │             ▼                       │
            │      ┌──────────────┐               │
            │      │  BUFFERING   │               │
            │      │ Accumulating │               │
            │      │  characters  │               │
            │      └──────┬───────┘               │
            │             │                       │
            │             │ Newline               │
            │             ▼                       │
            │      ┌──────────────┐               │
            │      │ PROCESSING   │               │
            │      │ Loop through │               │
            │      │ each char    │               │
            │      └──────┬───────┘               │
            │             │                       │
            │             ▼                       │
            │      ┌──────────────┐               │
            │      │  TRANSLATE   │               │
            │      │ Array lookup │               │
            │      │ Get pattern  │               │
            │      └──────┬───────┘               │
            │             │                       │
            │             ▼                       │
            │      ┌──────────────┐               │
            │      │  RETRACTING  │               │
            │      │ All GPIO LOW │               │
            │      │   (50ms)     │               │
            │      └──────┬───────┘               │
            │             │                       │
            │             ▼                       │
            │      ┌──────────────┐               │
            │      │  EXTENDING   │               │
            │      │ Set GPIO per │               │
            │      │   pattern    │               │
            │      │   (50ms)     │               │
            │      └──────┬───────┘               │
            │             │                       │
            │             ▼                       │
            │      ┌──────────────┐               │
            │      │   HOLDING    │               │
            │      │ Maintain pos.│               │
            │      │   (950ms)    │               │
            │      └──────┬───────┘               │
            │             │                       │
            │             ▼                       │
            │      ┌──────────────┐               │
            │      │ More chars?  │──YES→Loop back│
            │      └──────┬───────┘               │
            │             │ NO                    │
            └─────────────┴───────────────────────┘



```

---

## 7. Memory Map Diagram

```
┌────────────────────────────────────────────────────────────────────────┐
│                   ESP32 MEMORY LAYOUT (SINGLE-CHIP)                    │
└────────────────────────────────────────────────────────────────────────┘

FLASH (Program Memory - 4MB typical)
┌─────────────────────────────────┐
│  Arduino Core                   │ ~1MB
├─────────────────────────────────┤
│  Program Code                   │ ~300KB
│  • setup()                      │
│  • loop()                       │
│  • All functions                │
├─────────────────────────────────┤
│  CONSTANT ARRAYS                │
│  ┌───────────────────────────┐  │
│  │ ENGLISH_LOWER[26]         │  │ 52 bytes
│  │ NUMBERS[10]               │  │ 20 bytes
│  │ PUNCTUATION[12]           │  │ 24 bytes
│  │ RUSSIAN_CHARS[33]         │  │ 66 bytes
│  │ KAZAKH_CHARS[9]           │  │ 18 bytes
│  └───────────────────────────┘  │
├─────────────────────────────────┤
│  SOLENOID_PINS[6]               │ 6 bytes
│  Timing Constants               │ 16 bytes
├─────────────────────────────────┤
│  String Constants               │ ~2KB
└─────────────────────────────────┘

RAM (SRAM - 520KB total)
┌─────────────────────────────────┐
│  Stack (Core 0)                 │ ~8KB
├─────────────────────────────────┤
│  Stack (Core 1)                 │ ~8KB
├─────────────────────────────────┤
│  VARIABLES                      │
│  ┌───────────────────────────┐  │
│  │ inputBuffer (String)      │  │ Variable
│  │ processingComplete (bool) │  │ 1 byte
│  │ Current pattern (uint8_t) │  │ 1 byte
│  └───────────────────────────┘  │
├─────────────────────────────────┤
│  Serial Buffer                  │ 256 bytes
├─────────────────────────────────┤
│  Available Heap                 │ ~500KB+
└─────────────────────────────────┘

GPIO MAPPING (Hardware Registers)
┌─────────────────────────────────┐
│  GPIO25 → Solenoid 1            │
│  GPIO26 → Solenoid 2            │
│  GPIO27 → Solenoid 3            │
│  GPIO32 → Solenoid 4            │
│  GPIO33 → Solenoid 5            │
│  GPIO14 → Solenoid 6            │
└─────────────────────────────────┘

AVAILABLE RESOURCES:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Flash:   ~2.7MB remaining
RAM:     ~500KB remaining
GPIO:    28 pins available
Timers:  4 hardware timers
PWM:     16 channels
ADC:     18 channels
UART:    3 hardware UARTs

PLENTY OF ROOM FOR EXPANSION!
```

---
