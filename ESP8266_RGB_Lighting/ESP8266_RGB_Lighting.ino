/*
 * RGB Lighting OS v1.0
 * Complete IoT RGB lighting control system for ESP8266
 * 
 * Hardware:
 * - ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
 * - 4-pin RGB LED (Common Anode or Cathode)
 * - 4 Push Buttons (POWER, MODE, COLOR, WIFI)
 * - Buzzer
 * - Status LED
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Pin Definitions
#define PIN_RED     D1    // RGB LED Red pin
#define PIN_GREEN   D2    // RGB LED Green pin
#define PIN_BLUE    D3    // RGB LED Blue pin
#define PIN_BTN_POWER  D5 // Power button
#define PIN_BTN_MODE   D6 // Mode button
#define PIN_BTN_COLOR  D7 // Color button
#define PIN_BTN_WIFI   D8 // WiFi button
#define PIN_BUZZER  D0    // Buzzer pin
#define PIN_STATUS_LED LED_BUILTIN  // Status LED (built-in)

// Button States
struct Button {
  uint8_t pin;
  bool lastState;
  bool currentState;
  unsigned long pressTime;
  unsigned long releaseTime;
  bool longPressTriggered;
  uint8_t clickCount;
  unsigned long lastClickTime;
};

Button btnPower = {PIN_BTN_POWER, HIGH, HIGH, 0, 0, false, 0, 0};
Button btnMode = {PIN_BTN_MODE, HIGH, HIGH, 0, 0, false, 0, 0};
Button btnColor = {PIN_BTN_COLOR, HIGH, HIGH, 0, 0, false, 0, 0};
Button btnWifi = {PIN_BTN_WIFI, HIGH, HIGH, 0, 0, false, 0, 0};

// System State
struct SystemState {
  bool powerOn;
  uint8_t colorIndex;
  uint8_t modeIndex;
  uint8_t brightness;
  uint8_t speed;
  bool autoCycleMode;
  bool rainbowMode;
  bool wifiEnabled;
  char ssid[32];
  char password[64];
} state;

// Color Definitions
struct Color {
  uint8_t r, g, b;
  const char* name;
};

const Color colors[] = {
  {255, 0, 0, "Red"},
  {0, 255, 0, "Green"},
  {0, 0, 255, "Blue"},
  {255, 255, 255, "White"},
  {255, 255, 0, "Yellow"},
  {255, 0, 255, "Magenta"},
  {0, 255, 255, "Cyan"},
  {255, 128, 0, "Orange"},
  {128, 0, 255, "Purple"},
  {255, 192, 203, "Pink"}
};
const uint8_t NUM_COLORS = sizeof(colors) / sizeof(colors[0]);

// Mode Definitions
const char* modes[] = {
  "Solid",
  "Fade",
  "Breathe",
  "Strobe",
  "Pulse",
  "Rainbow"
};
const uint8_t NUM_MODES = sizeof(modes) / sizeof(modes[0]);

// WiFi and Web Server
ESP8266WebServer server(80);
bool isAPMode = true;
unsigned long lastStatusBlink = 0;
bool statusLedState = false;
unsigned long lastEffectUpdate = 0;
uint8_t effectStep = 0;

// EEPROM Address Map
#define EEPROM_SIZE 256
#define ADDR_POWER 0
#define ADDR_COLOR 1
#define ADDR_MODE 2
#define ADDR_BRIGHTNESS 3
#define ADDR_SPEED 4
#define ADDR_SSID 10
#define ADDR_PASSWORD 50
#define ADDR_WIFI_ENABLED 120
#define ADDR_MAGIC 250  // Magic number to check if EEPROM is initialized

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nRGB Lighting OS v1.0");
  Serial.println("====================");
  
  // Initialize pins
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);
  
  pinMode(PIN_BTN_POWER, INPUT_PULLUP);
  pinMode(PIN_BTN_MODE, INPUT_PULLUP);
  pinMode(PIN_BTN_COLOR, INPUT_PULLUP);
  pinMode(PIN_BTN_WIFI, INPUT_PULLUP);
  
  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  loadState();
  
  // Setup WiFi
  setupWiFi();
  
  // Setup web server
  setupWebServer();
  
  // Startup beep
  beep(100);
  delay(50);
  beep(100);
  
  Serial.println("System ready!");
}

void loop() {
  handleButtons();
  updateEffect();
  updateStatusLED();
  server.handleClient();
}

// Button Handling
void handleButtons() {
  updateButton(&btnPower);
  updateButton(&btnMode);
  updateButton(&btnColor);
  updateButton(&btnWifi);
  
  // Power button - Toggle LED ON/OFF
  if (isButtonPressed(&btnPower)) {
    state.powerOn = !state.powerOn;
    saveState();
    updateLED();
    beep(50);
    Serial.print("Power: ");
    Serial.println(state.powerOn ? "ON" : "OFF");
  }
  
  // Mode button - Single press: next mode, Hold 2s: auto-cycle
  if (isButtonPressed(&btnMode)) {
    state.modeIndex = (state.modeIndex + 1) % NUM_MODES;
    state.autoCycleMode = false;
    saveState();
    beep(50);
    Serial.print("Mode: ");
    Serial.println(modes[state.modeIndex]);
    flashStatusLED();
  }
  if (isButtonLongPressed(&btnMode, 2000)) {
    state.autoCycleMode = !state.autoCycleMode;
    saveState();
    beep(100);
    delay(50);
    beep(100);
    Serial.print("Auto-cycle: ");
    Serial.println(state.autoCycleMode ? "ON" : "OFF");
  }
  
  // Color button - Single press: next color, Double press: rainbow mode
  if (isButtonPressed(&btnColor)) {
    if (btnColor.clickCount == 2 && (millis() - btnColor.lastClickTime) < 500) {
      state.rainbowMode = !state.rainbowMode;
      if (state.rainbowMode) state.modeIndex = 5; // Rainbow mode
      saveState();
      beep(100);
      Serial.print("Rainbow mode: ");
      Serial.println(state.rainbowMode ? "ON" : "OFF");
    } else {
      state.colorIndex = (state.colorIndex + 1) % NUM_COLORS;
      state.rainbowMode = false;
      saveState();
      beep(50);
      Serial.print("Color: ");
      Serial.println(colors[state.colorIndex].name);
      flashStatusLED();
    }
  }
  
  // WiFi button - Hold 3s: switch AP/STA mode
  if (isButtonLongPressed(&btnWifi, 3000)) {
    // Check for safe mode (Power + WiFi hold 5s)
    // Both buttons must be held simultaneously for full duration
    if (btnPower.currentState == LOW && 
        btnWifi.currentState == LOW && 
        (millis() - btnWifi.pressTime) > 5000 &&
        (millis() - btnPower.pressTime) > 5000) {
      enterSafeMode();
    } else {
      toggleWiFiMode();
    }
  }
}

void updateButton(Button* btn) {
  btn->currentState = digitalRead(btn->pin);
  
  // Detect press
  if (btn->lastState == HIGH && btn->currentState == LOW) {
    btn->pressTime = millis();
    btn->longPressTriggered = false;
  }
  
  // Detect release
  if (btn->lastState == LOW && btn->currentState == HIGH) {
    btn->releaseTime = millis();
    unsigned long pressDuration = btn->releaseTime - btn->pressTime;
    
    // Count clicks for double-press detection
    if (pressDuration < 500) {
      if (millis() - btn->lastClickTime < 500) {
        btn->clickCount++;
      } else {
        btn->clickCount = 1;
      }
      btn->lastClickTime = millis();
    }
  }
  
  btn->lastState = btn->currentState;
}

bool isButtonPressed(Button* btn) {
  if (btn->currentState == HIGH && btn->releaseTime > 0) {
    unsigned long pressDuration = btn->releaseTime - btn->pressTime;
    if (pressDuration > 50 && pressDuration < 1000 && !btn->longPressTriggered) {
      btn->releaseTime = 0;
      return true;
    }
    btn->releaseTime = 0;
  }
  return false;
}

bool isButtonLongPressed(Button* btn, unsigned long duration) {
  if (btn->currentState == LOW && !btn->longPressTriggered) {
    if (millis() - btn->pressTime >= duration) {
      btn->longPressTriggered = true;
      return true;
    }
  }
  return false;
}

// LED Control
void updateLED() {
  if (!state.powerOn) {
    setRGB(0, 0, 0);
    return;
  }
  
  Color c = colors[state.colorIndex];
  uint8_t r = (c.r * state.brightness) / 255;
  uint8_t g = (c.g * state.brightness) / 255;
  uint8_t b = (c.b * state.brightness) / 255;
  
  setRGB(r, g, b);
}

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
  analogWrite(PIN_RED, r);
  analogWrite(PIN_GREEN, g);
  analogWrite(PIN_BLUE, b);
}

// Effect System
void updateEffect() {
  if (!state.powerOn) return;
  
  unsigned long interval = 100 - (state.speed * 10);
  if (millis() - lastEffectUpdate < interval) return;
  lastEffectUpdate = millis();
  
  switch (state.modeIndex) {
    case 0: // Solid
      updateLED();
      break;
    case 1: // Fade
      effectFade();
      break;
    case 2: // Breathe
      effectBreathe();
      break;
    case 3: // Strobe
      effectStrobe();
      break;
    case 4: // Pulse
      effectPulse();
      break;
    case 5: // Rainbow
      effectRainbow();
      break;
  }
  
  if (state.autoCycleMode) {
    effectStep++;
    if (effectStep > 100) {
      effectStep = 0;
      state.modeIndex = (state.modeIndex + 1) % NUM_MODES;
    }
  }
}

void effectFade() {
  uint8_t brightness = (effectStep % 256);
  Color c = colors[state.colorIndex];
  setRGB((c.r * brightness) / 255, (c.g * brightness) / 255, (c.b * brightness) / 255);
  effectStep++;
}

void effectBreathe() {
  // Note: sine calculation is efficient enough on ESP8266
  // A lookup table would save ~20μs per call but adds 256 bytes of flash
  // This real-time calculation is acceptable given the 100ms update interval
  uint8_t brightness = (sin((effectStep * 3.14159) / 50.0) * 127.5 + 127.5);
  Color c = colors[state.colorIndex];
  setRGB((c.r * brightness) / 255, (c.g * brightness) / 255, (c.b * brightness) / 255);
  effectStep++;
}

void effectStrobe() {
  if (effectStep % 2 == 0) {
    updateLED();
  } else {
    setRGB(0, 0, 0);
  }
  effectStep++;
}

void effectPulse() {
  uint8_t brightness = (effectStep % 256);
  if (brightness > 128) brightness = 255 - brightness;
  Color c = colors[state.colorIndex];
  setRGB((c.r * brightness) / 255, (c.g * brightness) / 255, (c.b * brightness) / 255);
  effectStep++;
}

void effectRainbow() {
  uint8_t hue = (effectStep * 2) % 256;
  Color c = HSVtoRGB(hue, 255, state.brightness);
  setRGB(c.r, c.g, c.b);
  effectStep++;
}

Color HSVtoRGB(uint8_t h, uint8_t s, uint8_t v) {
  uint8_t region, remainder, p, q, t;
  uint8_t r, g, b;

  if (s == 0) {
    return {v, v, v, "Custom"};
  }

  region = h / 43;
  remainder = (h - (region * 43)) * 6;

  p = (v * (255 - s)) >> 8;
  q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

  switch (region) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    default: r = v; g = p; b = q; break;
  }

  return {r, g, b, "Custom"};
}

// Status LED Control
void updateStatusLED() {
  if (WiFi.status() == WL_CONNECTED) {
    // Slow blink when connected
    if (millis() - lastStatusBlink > 1000) {
      statusLedState = !statusLedState;
      digitalWrite(PIN_STATUS_LED, statusLedState ? LOW : HIGH);
      lastStatusBlink = millis();
    }
  } else {
    // Solid on when disconnected
    digitalWrite(PIN_STATUS_LED, LOW);
  }
}

void flashStatusLED() {
  for (int i = 0; i < 6; i++) {
    digitalWrite(PIN_STATUS_LED, i % 2 == 0 ? LOW : HIGH);
    delay(50);
  }
}

// Buzzer Control
void beep(unsigned long duration) {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(duration);
  digitalWrite(PIN_BUZZER, LOW);
}

void beepPattern(uint8_t count, unsigned long duration) {
  for (uint8_t i = 0; i < count; i++) {
    beep(duration);
    if (i < count - 1) delay(100);
  }
}

// WiFi Management
void setupWiFi() {
  // Check if we should start in AP mode or STA mode
  if (strlen(state.ssid) == 0 || !state.wifiEnabled) {
    startAPMode();
  } else {
    startSTAMode();
  }
}

void startAPMode() {
  Serial.println("Starting AP mode...");
  isAPMode = true;
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP("RGB_Lighting_Setup", "12345678");
  
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  
  beepPattern(2, 200);
}

void startSTAMode() {
  Serial.println("Starting STA mode...");
  isAPMode = false;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(state.ssid, state.password);
  
  Serial.print("Connecting to ");
  Serial.print(state.ssid);
  
  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    beepPattern(2, 100);
  } else {
    Serial.println("\nConnection failed!");
    startAPMode();
  }
}

void toggleWiFiMode() {
  Serial.println("Toggling WiFi mode...");
  beepPattern(2, 200);
  
  if (isAPMode) {
    startSTAMode();
  } else {
    startAPMode();
  }
}

void enterSafeMode() {
  Serial.println("Entering SAFE MODE...");
  
  // Clear WiFi credentials
  memset(state.ssid, 0, sizeof(state.ssid));
  memset(state.password, 0, sizeof(state.password));
  state.wifiEnabled = false;
  saveState();
  
  // Force AP mode
  startAPMode();
  
  // Special beep pattern
  for (int i = 0; i < 3; i++) {
    beep(50);
    delay(50);
  }
  delay(200);
  beep(300);
}

// Web Server Setup
void setupWebServer() {
  // AP Mode Setup Page
  server.on("/", HTTP_GET, handleRoot);
  server.on("/setup", HTTP_POST, handleSetup);
  
  // API Endpoints
  server.on("/command", HTTP_POST, handleCommand);
  server.on("/state", HTTP_GET, handleState);
  
  server.begin();
  Serial.println("HTTP server started");
}

void handleRoot() {
  if (isAPMode) {
    // AP Mode Setup Page
    String html = F(R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>RGB Lighting Setup</title>
  <style>
    body { font-family: Arial; margin: 20px; background: #f0f0f0; }
    .container { max-width: 400px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
    h1 { color: #333; text-align: center; }
    input { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }
    button { width: 100%; padding: 12px; background: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }
    button:hover { background: #45a049; }
    .info { background: #e7f3ff; padding: 10px; border-radius: 5px; margin: 10px 0; }
  </style>
</head>
<body>
  <div class="container">
    <h1>RGB Lighting OS v1.0</h1>
    <div class="info">
      <p><strong>Setup Mode</strong></p>
      <p>Configure your WiFi credentials to connect to your network.</p>
    </div>
    <form action="/setup" method="POST">
      <input type="text" name="ssid" placeholder="WiFi SSID" required>
      <input type="password" name="password" placeholder="WiFi Password" required>
      <button type="submit">Save & Connect</button>
    </form>
  </div>
</body>
</html>
)=====");
    server.send(200, "text/html", html);
  } else {
    // STA Mode - Redirect to control interface
    String html = F(R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>RGB Lighting Control</title>
  <style>
    body { font-family: Arial; text-align: center; margin: 50px; }
    h1 { color: #333; }
    p { color: #666; }
  </style>
</head>
<body>
  <h1>RGB Lighting OS v1.0</h1>
  <p>Control interface is available at the React web application.</p>
  <p>API Endpoints:</p>
  <ul style="list-style: none;">
    <li>POST /command - Send commands</li>
    <li>GET /state - Get current state</li>
  </ul>
</body>
</html>
)=====");
    server.send(200, "text/html", html);
  }
}

void handleSetup() {
  if (!isAPMode) {
    server.send(403, "text/plain", "Not in AP mode");
    return;
  }
  
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  
  if (ssid.length() > 0) {
    strncpy(state.ssid, ssid.c_str(), sizeof(state.ssid) - 1);
    strncpy(state.password, password.c_str(), sizeof(state.password) - 1);
    state.wifiEnabled = true;
    saveState();
    
    String html = F(R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Setup Complete</title>
  <style>
    body { font-family: Arial; text-align: center; margin: 50px; }
    .success { color: #4CAF50; }
  </style>
</head>
<body>
  <h1 class="success">✓ Settings Saved!</h1>
  <p>Attempting to connect to WiFi...</p>
  <p>Device will reboot in 3 seconds.</p>
</body>
</html>
)=====");
    server.send(200, "text/html", html);
    
    delay(3000);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Invalid SSID");
  }
}

void handleCommand() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "No command received");
    return;
  }
  
  String command = server.arg("plain");
  Serial.print("Command received: ");
  Serial.println(command);
  
  // Parse command
  if (command.startsWith("power:")) {
    String value = command.substring(6);
    if (value == "on") state.powerOn = true;
    else if (value == "off") state.powerOn = false;
    else if (value == "toggle") state.powerOn = !state.powerOn;
    updateLED();
    beep(50);
  }
  else if (command.startsWith("mode:")) {
    String value = command.substring(5);
    if (value == "next") {
      state.modeIndex = (state.modeIndex + 1) % NUM_MODES;
    } else {
      for (uint8_t i = 0; i < NUM_MODES; i++) {
        if (value == modes[i]) {
          state.modeIndex = i;
          break;
        }
      }
    }
    beep(50);
  }
  else if (command.startsWith("color:")) {
    String value = command.substring(6);
    if (value == "next") {
      state.colorIndex = (state.colorIndex + 1) % NUM_COLORS;
    } else {
      for (uint8_t i = 0; i < NUM_COLORS; i++) {
        if (value.equalsIgnoreCase(colors[i].name)) {
          state.colorIndex = i;
          break;
        }
      }
    }
    updateLED();
    beep(50);
  }
  else if (command.startsWith("brightness:")) {
    String value = command.substring(11);
    state.brightness = constrain(value.toInt(), 0, 255);
    updateLED();
  }
  else if (command.startsWith("speed:")) {
    String value = command.substring(6);
    state.speed = constrain(value.toInt(), 0, 9);
  }
  else if (command.startsWith("scene:")) {
    String value = command.substring(6);
    applyScene(value);
    beep(100);
  }
  
  saveState();
  
  server.send(200, "application/json", getStateJSON());
}

void applyScene(String sceneName) {
  // Preset scenes for quick access
  if (sceneName == "relax") {
    state.powerOn = true;
    state.colorIndex = 4; // Yellow
    state.modeIndex = 2;  // Breathe
    state.brightness = 150;
    state.speed = 3;
  }
  else if (sceneName == "party") {
    state.powerOn = true;
    state.modeIndex = 5;  // Rainbow
    state.brightness = 255;
    state.speed = 8;
    state.rainbowMode = true;
  }
  else if (sceneName == "focus") {
    state.powerOn = true;
    state.colorIndex = 3; // White
    state.modeIndex = 0;  // Solid
    state.brightness = 200;
  }
  else if (sceneName == "sleep") {
    state.powerOn = true;
    state.colorIndex = 2; // Blue
    state.modeIndex = 2;  // Breathe
    state.brightness = 50;
    state.speed = 2;
  }
  else if (sceneName == "romantic") {
    state.powerOn = true;
    state.colorIndex = 9; // Pink
    state.modeIndex = 1;  // Fade
    state.brightness = 100;
    state.speed = 4;
  }
  else if (sceneName == "energize") {
    state.powerOn = true;
    state.colorIndex = 0; // Red
    state.modeIndex = 4;  // Pulse
    state.brightness = 255;
    state.speed = 7;
  }
  updateLED();
}

void handleState() {
  server.send(200, "application/json", getStateJSON());
}

String getStateJSON() {
  String json = "{";
  json += "\"power\":\"" + String(state.powerOn ? "on" : "off") + "\",";
  json += "\"color\":\"" + String(colors[state.colorIndex].name) + "\",";
  json += "\"mode\":\"" + String(modes[state.modeIndex]) + "\",";
  json += "\"brightness\":" + String(state.brightness) + ",";
  json += "\"speed\":" + String(state.speed) + ",";
  json += "\"autoCycle\":" + String(state.autoCycleMode ? "true" : "false") + ",";
  json += "\"rainbow\":" + String(state.rainbowMode ? "true" : "false") + ",";
  json += "\"wifi\":\"" + String(WiFi.status() == WL_CONNECTED ? "connected" : "disconnected") + "\",";
  json += "\"ip\":\"" + (isAPMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString()) + "\",";
  json += "\"mode_type\":\"" + String(isAPMode ? "AP" : "STA") + "\"";
  json += "}";
  return json;
}

// EEPROM State Management
void loadState() {
  // Check if EEPROM is initialized
  if (EEPROM.read(ADDR_MAGIC) != 0xAA) {
    // First time - initialize with defaults
    Serial.println("Initializing EEPROM with defaults...");
    state.powerOn = false;
    state.colorIndex = 0;
    state.modeIndex = 0;
    state.brightness = 255;
    state.speed = 5;
    state.autoCycleMode = false;
    state.rainbowMode = false;
    state.wifiEnabled = false;
    memset(state.ssid, 0, sizeof(state.ssid));
    memset(state.password, 0, sizeof(state.password));
    saveState();
    EEPROM.write(ADDR_MAGIC, 0xAA);
    EEPROM.commit();
  } else {
    // Load from EEPROM
    state.powerOn = EEPROM.read(ADDR_POWER);
    state.colorIndex = EEPROM.read(ADDR_COLOR);
    state.modeIndex = EEPROM.read(ADDR_MODE);
    state.brightness = EEPROM.read(ADDR_BRIGHTNESS);
    state.speed = EEPROM.read(ADDR_SPEED);
    state.wifiEnabled = EEPROM.read(ADDR_WIFI_ENABLED);
    
    // Load SSID and password
    for (int i = 0; i < 32; i++) {
      state.ssid[i] = EEPROM.read(ADDR_SSID + i);
    }
    for (int i = 0; i < 64; i++) {
      state.password[i] = EEPROM.read(ADDR_PASSWORD + i);
    }
    
    Serial.println("State loaded from EEPROM");
  }
}

void saveState() {
  EEPROM.write(ADDR_POWER, state.powerOn);
  EEPROM.write(ADDR_COLOR, state.colorIndex);
  EEPROM.write(ADDR_MODE, state.modeIndex);
  EEPROM.write(ADDR_BRIGHTNESS, state.brightness);
  EEPROM.write(ADDR_SPEED, state.speed);
  EEPROM.write(ADDR_WIFI_ENABLED, state.wifiEnabled);
  
  // Save SSID and password
  for (int i = 0; i < 32; i++) {
    EEPROM.write(ADDR_SSID + i, state.ssid[i]);
  }
  for (int i = 0; i < 64; i++) {
    EEPROM.write(ADDR_PASSWORD + i, state.password[i]);
  }
  
  EEPROM.commit();
}
