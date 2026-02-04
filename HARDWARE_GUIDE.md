# Hardware Wiring Guide

## Complete Component List

### Essential Components
| Quantity | Component | Specification | Purpose |
|----------|-----------|---------------|---------|
| 1 | ESP8266 | NodeMCU or Wemos D1 Mini | Main controller |
| 1 | RGB LED | 4-pin Common Cathode/Anode | Light output |
| 4 | Push Buttons | Tactile switch, normally open | User input |
| 3 | Resistors | 220Ω, 1/4W | LED current limiting |
| 1 | Buzzer | Active 5V piezo buzzer | Audio feedback |
| 1 | Breadboard | 830 points recommended | Prototyping |
| - | Jumper wires | Male-to-male | Connections |

### Optional Components
| Quantity | Component | Specification | Purpose |
|----------|-----------|---------------|---------|
| 4 | Resistors | 10kΩ | External pull-ups (if needed) |
| 1 | Power supply | 5V 1A USB | Independent power |
| 1 | Switch | Toggle or rocker | Main power switch |
| 1 | Enclosure | Custom or 3D printed | Protection |

## Pin Mapping

### ESP8266 NodeMCU Pin Assignments

```
┌─────────────────────────────────────────────────────┐
│                  ESP8266 NodeMCU                    │
│                                                     │
│   D0 (GPIO16) ────────┐                            │
│                       │ Buzzer (+)                  │
│   D1 (GPIO5)  ────────┐                            │
│                       │ RGB LED - Red               │
│   D2 (GPIO4)  ────────┐                            │
│                       │ RGB LED - Green             │
│   D3 (GPIO0)  ────────┐                            │
│                       │ RGB LED - Blue              │
│   D5 (GPIO14) ────────┐                            │
│                       │ POWER Button                │
│   D6 (GPIO12) ────────┐                            │
│                       │ MODE Button                 │
│   D7 (GPIO13) ────────┐                            │
│                       │ COLOR Button                │
│   D8 (GPIO15) ────────┐                            │
│                       │ WIFI Button                 │
│   GND ────────────────┴─ Common Ground             │
│   3.3V/5V ──────────────  Power Supply             │
│   LED_BUILTIN ───────────  Status LED (on-board)   │
└─────────────────────────────────────────────────────┘
```

## Detailed Wiring Instructions

### 1. RGB LED Connection (Common Cathode)

**Common Cathode Configuration:**
```
RGB LED                    ESP8266
┌─────────┐
│    R    │───[220Ω]───── D1 (GPIO5)
│    G    │───[220Ω]───── D2 (GPIO4)
│    B    │───[220Ω]───── D3 (GPIO0)
│  Common │───────────── GND
└─────────┘
```

**If you have Common Anode RGB LED:**
```
RGB LED                    ESP8266
┌─────────┐
│    R    │───[220Ω]───── D1 (GPIO5)
│    G    │───[220Ω]───── D2 (GPIO4)
│    B    │───[220Ω]───── D3 (GPIO0)
│  Common │───────────── 3.3V or 5V
└─────────┘
```
*Note: For Common Anode, you'll need to invert PWM values in code (255 - value)*

**LED Identification:**
- Longest pin: Common (Cathode or Anode)
- Next to longest: Usually Red
- Second shortest: Usually Green
- Shortest: Usually Blue

### 2. Button Connections

All buttons use internal pull-up resistors, so no external resistors needed:

```
Button                     ESP8266
┌────────┐
│ POWER  │──── Pin 1 ───── D5 (GPIO14)
│        │──── Pin 2 ───── GND
└────────┘

┌────────┐
│  MODE  │──── Pin 1 ───── D6 (GPIO12)
│        │──── Pin 2 ───── GND
└────────┘

┌────────┐
│ COLOR  │──── Pin 1 ───── D7 (GPIO13)
│        │──── Pin 2 ───── GND
└────────┘

┌────────┐
│  WIFI  │──── Pin 1 ───── D8 (GPIO15)
│        │──── Pin 2 ───── GND
└────────┘
```

**Button Wiring Tips:**
- Tactile switches have 4 pins but only 2 are needed
- Pins on opposite sides are connected
- Use diagonal pins for proper operation
- Orient buttons with pins facing up/down on breadboard

### 3. Buzzer Connection

```
Buzzer                    ESP8266
┌────────┐
│   (+)  │───────────── D0 (GPIO16)
│   (-)  │───────────── GND
└────────┘
```

**Buzzer Notes:**
- Use ACTIVE buzzer (has internal oscillator)
- Polarity matters: + to D0, - to GND
- If no sound, try reversing polarity (some buzzers differ)

### 4. Power Supply

**Option A: USB Power (Recommended for Development)**
```
USB Cable ───────► ESP8266 USB port
```

**Option B: External 5V Supply**
```
5V Power Supply
    (+) ─────────► VIN pin (ESP8266)
    (-) ─────────► GND pin (ESP8266)
```

**Option C: Battery Power**
```
3.7V LiPo Battery ────► VIN via voltage regulator
```

## Breadboard Layout

```
                    Breadboard Layout
    ┌────────────────────────────────────────────┐
    │                                            │
    │  [BTN1] [BTN2] [BTN3] [BTN4]              │
    │  POWER  MODE   COLOR  WIFI                 │
    │    │      │      │      │                   │
    │    │      │      │      │                   │
    │  ┌─────────────────────────┐               │
    │  │                         │               │
    │  │     ESP8266 NodeMCU     │               │
    │  │                         │               │
    │  └─────────────────────────┘               │
    │              │ │ │                          │
    │              │ │ │                          │
    │            [220Ω resistors]                 │
    │              │ │ │                          │
    │           ┌──────┐                          │
    │           │ RGB  │                          │
    │           │ LED  │                          │
    │           └──────┘                          │
    │                                             │
    │         [BUZZER]                            │
    │                                             │
    └────────────────────────────────────────────┘
```

## Step-by-Step Assembly

### Step 1: Place ESP8266 on Breadboard
1. Insert ESP8266 into breadboard center
2. Ensure pins on both sides have good contact
3. Leave space above and below for connections

### Step 2: Connect RGB LED
1. Place RGB LED in breadboard
2. Connect 220Ω resistor to each color pin
3. Connect resistor other end to ESP8266:
   - Red → D1
   - Green → D2
   - Blue → D3
4. Connect common pin to GND

### Step 3: Connect Buttons
1. Place buttons on breadboard
2. Connect one pin of each button to respective GPIO
3. Connect other pin to GND rail
4. Label buttons for easy identification

### Step 4: Connect Buzzer
1. Identify buzzer polarity (+ and -)
2. Connect + to D0
3. Connect - to GND

### Step 5: Power Connection
1. Connect USB cable to ESP8266
2. Verify all connections before powering on

### Step 6: Test
1. Upload firmware
2. Power on system
3. Test each button
4. Verify RGB LED colors
5. Check buzzer sounds

## Testing Checklist

- [ ] Power LED on ESP8266 lights up
- [ ] Status LED blinks or stays solid
- [ ] POWER button toggles RGB LED
- [ ] MODE button changes effects
- [ ] COLOR button changes colors
- [ ] WIFI button triggers WiFi functions
- [ ] Buzzer beeps on actions
- [ ] RGB LED shows correct colors
- [ ] All buttons respond reliably
- [ ] No loose connections

## Common Issues and Solutions

### RGB LED not lighting
- Check LED orientation (common cathode/anode)
- Verify resistors are 220Ω, not too high
- Test each color pin with multimeter
- Ensure correct pin assignments in code

### Buttons not responding
- Check button orientation on breadboard
- Verify GND connections
- Test with multimeter (should show 0Ω when pressed)
- Try different GPIO pins if one is faulty

### Buzzer not working
- Verify it's an ACTIVE buzzer
- Try reversing polarity
- Check if pin D0 can be used (some boards differ)
- Test with LED first to verify GPIO works

### Intermittent connections
- Push components firmly into breadboard
- Use quality jumper wires
- Check for bent pins
- Consider soldering for permanent installation

## Advanced: PCB Design Tips

For permanent installation, consider designing a custom PCB:

1. **Layout Considerations:**
   - Place buttons for easy access
   - Position RGB LED prominently
   - Add mounting holes
   - Include power switch

2. **Components to Add:**
   - Voltage regulator for battery operation
   - Protection diodes
   - Capacitors for power filtering (100nF, 10µF)
   - LED indicators

3. **PCB Design Software:**
   - KiCad (free, open-source)
   - Eagle (free for hobbyists)
   - EasyEDA (online, free)

## Enclosure Ideas

1. **3D Printed Case:**
   - Custom fit for your components
   - Includes button extensions
   - LED window

2. **Commercial Enclosure:**
   - Plastic project box
   - Drill holes for buttons, LED, USB

3. **Acrylic Case:**
   - Laser-cut acrylic sheets
   - Modern transparent look
   - LED light diffusion

## Safety Notes

⚠️ **Important Safety Information:**

1. **Electrical Safety:**
   - Never exceed voltage ratings
   - Use appropriate resistors
   - Avoid short circuits

2. **Component Protection:**
   - ESP8266 is 3.3V logic (5V tolerant on some pins)
   - Don't connect 5V directly to GPIO pins
   - Use current-limiting resistors for LEDs

3. **Heat Dissipation:**
   - Ensure adequate ventilation
   - Don't enclose tightly during testing
   - Components will warm during operation

4. **ESD Protection:**
   - Touch grounded metal before handling
   - Store components in anti-static bags
   - Avoid static-prone environments

## Bill of Materials (BOM)

| Item | Quantity | Unit Price | Total | Source |
|------|----------|------------|-------|--------|
| ESP8266 NodeMCU | 1 | $5-8 | $5-8 | Amazon, AliExpress |
| RGB LED | 1 | $0.50 | $0.50 | Local electronics |
| Tactile Buttons | 4 | $0.10 | $0.40 | Local electronics |
| 220Ω Resistors | 3 | $0.05 | $0.15 | Local electronics |
| Active Buzzer 5V | 1 | $0.50 | $0.50 | Local electronics |
| Breadboard | 1 | $3 | $3 | Amazon, AliExpress |
| Jumper Wires | 1 pack | $2 | $2 | Amazon, AliExpress |
| USB Cable | 1 | $2 | $2 | Amazon, local |
| **Total** | | | **~$13-16** | |

*Prices are approximate and vary by region and supplier.*

## Support

For hardware questions or issues:
1. Check this wiring guide thoroughly
2. Review the main README troubleshooting section
3. Post in the Issues section with photos of your setup
4. Include multimeter readings if possible

Happy building! 🔧⚡
