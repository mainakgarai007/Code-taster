# RGB Lighting OS v1.0

Complete IoT RGB lighting control system for ESP8266 with React web interface.

## 🌟 Features

### Hardware Support
- **ESP8266** (NodeMCU, Wemos D1 Mini, etc.)
- **4-pin RGB LED** (Common Anode or Cathode)
- **4 Push Buttons** (POWER, MODE, COLOR, WIFI)
- **Buzzer** for audio feedback
- **Status LED** for system status indication

### Software Features
- **Dual Website System**:
  1. ESP8266 AP-mode Setup Website (minimal HTML)
  2. React-based Control Website (GitHub-hosted)
- **10 Pre-defined Colors**: Red, Green, Blue, White, Yellow, Magenta, Cyan, Orange, Purple, Pink
- **6 Lighting Effects**: Solid, Fade, Breathe, Strobe, Pulse, Rainbow
- **State Persistence**: Saves settings to EEPROM/Flash
- **WiFi Management**: AP mode for setup, STA mode for operation
- **Safe Mode**: Recovery mode if WiFi credentials fail
- **HTTP API**: Full control via REST endpoints

## 📋 Hardware Requirements

### Components
- 1x ESP8266 board (NodeMCU or Wemos D1 Mini)
- 1x RGB LED (Common Anode or Common Cathode)
- 4x Push buttons (tactile switches)
- 4x 10kΩ resistors (for pull-up)
- 3x 220Ω resistors (for LED current limiting)
- 1x Active buzzer (5V)
- 1x Breadboard or PCB
- Jumper wires

### Wiring Diagram

```
ESP8266 Pin Connections:
┌─────────────────────────────────────────┐
│ ESP8266 Pin    │ Component              │
├─────────────────────────────────────────┤
│ D1 (GPIO5)     │ RGB LED - Red          │
│ D2 (GPIO4)     │ RGB LED - Green        │
│ D3 (GPIO0)     │ RGB LED - Blue         │
│ D5 (GPIO14)    │ POWER Button           │
│ D6 (GPIO12)    │ MODE Button            │
│ D7 (GPIO13)    │ COLOR Button           │
│ D8 (GPIO15)    │ WIFI Button            │
│ D0 (GPIO16)    │ Buzzer                 │
│ LED_BUILTIN    │ Status LED (built-in)  │
│ GND            │ Common Ground          │
│ 3.3V/5V        │ Power Supply           │
└─────────────────────────────────────────┘

RGB LED Wiring (Common Cathode):
- Red pin → 220Ω resistor → D1
- Green pin → 220Ω resistor → D2
- Blue pin → 220Ω resistor → D3
- Common pin → GND

Button Wiring (with internal pull-up):
- One side → GPIO pin
- Other side → GND
```

## 🚀 Quick Start

### 1. ESP8266 Firmware Setup

#### Prerequisites
- Arduino IDE 1.8.x or later
- ESP8266 Board Package installed

#### Installation Steps

1. **Install Arduino IDE**
   ```bash
   # Download from https://www.arduino.cc/en/software
   ```

2. **Add ESP8266 Board Support**
   - Open Arduino IDE
   - Go to: File → Preferences
   - Add to "Additional Board Manager URLs":
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - Go to: Tools → Board → Boards Manager
   - Search for "esp8266" and install

3. **Upload Firmware**
   - Open `ESP8266_RGB_Lighting/ESP8266_RGB_Lighting.ino`
   - Select your board: Tools → Board → ESP8266 Boards → (your board)
   - Select port: Tools → Port → (your COM port)
   - Click Upload

### 2. Initial WiFi Setup

1. **Power on the ESP8266**
   - On first boot, it starts in **AP Mode**
   - Status LED will be solid ON

2. **Connect to AP**
   - SSID: `RGB_Lighting_Setup`
   - Password: `12345678`

3. **Configure WiFi**
   - Open browser and go to: `http://192.168.4.1`
   - Enter your WiFi SSID and Password
   - Click "Save & Connect"
   - Device will restart and connect to your WiFi

4. **Find Device IP**
   - Check serial monitor (115200 baud)
   - Or check your router's connected devices

### 3. React Control Website Setup

#### Option A: Run Locally

```bash
cd RGB_Control_Website
npm install
npm start
```

The app will open at `http://localhost:3000`

#### Option B: Build for GitHub Pages

```bash
cd RGB_Control_Website
npm install
npm run build
```

Deploy the `build` folder to GitHub Pages or any static hosting service.

### 4. Connect to Device

1. Open the React control website
2. Enter your ESP8266 IP address (e.g., `192.168.1.100`)
3. Click "Connect"
4. Start controlling your RGB LED!

## 🎮 Button Controls

### Physical Buttons on ESP8266

#### POWER Button
- **Single Press**: Toggle LED ON/OFF
- **Feedback**: Short beep

#### MODE Button
- **Single Press**: Cycle through lighting effects
  - Solid → Fade → Breathe → Strobe → Pulse → Rainbow
- **Long Press (2s)**: Enable/disable auto-cycle mode
- **Feedback**: Short beep (single press), Double beep (long press)

#### COLOR Button
- **Single Press**: Cycle through colors
  - Red → Green → Blue → White → Yellow → Magenta → Cyan → Orange → Purple → Pink
- **Double Press**: Toggle rainbow/auto-color mode
- **Feedback**: Short beep (single press), Long beep (double press)

#### WIFI Button
- **Long Press (3s)**: Switch between AP and STA mode
- **Long Press (5s) with POWER**: Enter SAFE MODE
- **Feedback**: Double long beeps (mode switch)

### Safe Mode
If WiFi credentials are incorrect or you can't connect:
1. Hold **POWER** + **WIFI** buttons for 5 seconds
2. Device enters SAFE MODE (clears WiFi credentials)
3. Device restarts in AP mode
4. Reconfigure WiFi settings

## 🌐 HTTP API

### Base URL
- **AP Mode**: `http://192.168.4.1`
- **STA Mode**: `http://<device-ip>`

### Endpoints

#### GET /state
Get current device state

**Response:**
```json
{
  "power": "on",
  "color": "Red",
  "mode": "Solid",
  "brightness": 255,
  "speed": 5,
  "autoCycle": false,
  "rainbow": false,
  "wifi": "connected",
  "ip": "192.168.1.100",
  "mode_type": "STA"
}
```

#### POST /command
Send control commands

**Request Body (text/plain):**
```
power:on
power:off
power:toggle
mode:next
mode:Solid
mode:Fade
mode:Breathe
mode:Strobe
mode:Pulse
mode:Rainbow
color:next
color:Red
color:Green
color:Blue
color:White
color:Yellow
color:Magenta
color:Cyan
color:Orange
color:Purple
color:Pink
brightness:255
speed:5
```

**Example with curl:**
```bash
# Turn on
curl -X POST http://192.168.1.100/command -d "power:on"

# Change color to blue
curl -X POST http://192.168.1.100/command -d "color:Blue"

# Set brightness to 50%
curl -X POST http://192.168.1.100/command -d "brightness:128"
```

## 🔊 Buzzer Feedback

- **Mode/Color Change**: 1 short beep (50ms)
- **WiFi Connected/Disconnected**: 2 short beeps (100ms each)
- **AP ↔ STA Switching**: 2 long beeps (200ms each)
- **Safe Mode**: 3 short beeps + 1 long beep
- **Startup**: 2 short beeps

## 💡 Status LED Patterns

- **Solid ON**: WiFi disconnected
- **Slow Blink (1s)**: WiFi connected and ready
- **Fast Blink**: Mode or color change
- **Blinking Pattern**: During AP/STA switching

## 💾 State Persistence

The following settings are saved to EEPROM and restored on boot:
- Power state (ON/OFF)
- Selected color
- Selected mode
- Brightness level
- Speed setting
- WiFi credentials
- WiFi enabled status

## 🛠️ Troubleshooting

### Device won't connect to WiFi
1. Enter SAFE MODE (hold POWER + WIFI for 5s)
2. Reconnect to AP mode
3. Reconfigure WiFi settings

### Can't find device IP
1. Check serial monitor (115200 baud)
2. Check your router's DHCP client list
3. Use network scanner app (e.g., Fing)

### Buttons not responding
1. Check button wiring and pull-up resistors
2. Test each button in serial monitor
3. Adjust debounce timing if needed

### LED colors incorrect
1. Check if using Common Anode or Common Cathode
2. Adjust PWM values in code if needed
3. Verify current-limiting resistors

### Web interface won't connect
1. Verify device IP address
2. Check CORS settings (should allow all origins)
3. Ensure device and computer are on same network

## 📱 React Control Interface Features

- **Real-time Status**: Auto-updates every 2 seconds
- **Power Control**: One-click ON/OFF toggle
- **Color Selection**: 10 pre-defined colors with visual buttons
- **Effect Selection**: 6 lighting effects
- **Brightness Control**: 0-100% slider
- **Speed Control**: Adjust effect animation speed
- **Quick Actions**: Next color/mode buttons
- **Device Status**: Shows WiFi, IP, mode, and connection status
- **Responsive Design**: Works on desktop and mobile

## 🔒 Security Notes

- Default AP password should be changed in production
- No authentication on HTTP endpoints (add if needed)
- Store sensitive WiFi credentials securely in EEPROM
- Consider using HTTPS for production deployments

## 📄 License

This project is open source and available under the MIT License.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 📧 Support

For issues, questions, or suggestions, please open an issue on GitHub.

## 🔗 Resources

- [ESP8266 Arduino Core Documentation](https://arduino-esp8266.readthedocs.io/)
- [React Documentation](https://react.dev/)
- [ESP8266 Community Forum](https://www.esp8266.com/)

---

**RGB Lighting OS v1.0** - Built with ❤️ for makers and IoT enthusiasts