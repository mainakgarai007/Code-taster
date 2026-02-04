# API Documentation

## Base URL

- **AP Mode**: `http://192.168.4.1`
- **STA Mode**: `http://<device-ip>` (Check serial monitor or router for IP)

## Authentication

No authentication required (consider adding for production use).

## Endpoints

### GET /state

Get the current state of all device parameters.

**Request:**
```http
GET /state HTTP/1.1
Host: 192.168.1.100
```

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

**Response Fields:**
- `power`: Current power state ("on" or "off")
- `color`: Current color name
- `mode`: Current lighting effect
- `brightness`: Brightness level (0-255)
- `speed`: Effect speed (0-9)
- `autoCycle`: Auto-cycle mode status
- `rainbow`: Rainbow mode status
- `wifi`: WiFi connection status
- `ip`: Device IP address
- `mode_type`: WiFi mode ("AP" or "STA")

### POST /command

Send commands to control the device.

**Request:**
```http
POST /command HTTP/1.1
Host: 192.168.1.100
Content-Type: text/plain

power:toggle
```

**Response:**
Returns the updated state (same format as GET /state).

## Command Reference

### Power Commands

| Command | Description |
|---------|-------------|
| `power:on` | Turn LED on |
| `power:off` | Turn LED off |
| `power:toggle` | Toggle power state |

**Example:**
```bash
curl -X POST http://192.168.1.100/command -d "power:on"
```

### Mode Commands

| Command | Description |
|---------|-------------|
| `mode:next` | Switch to next effect |
| `mode:Solid` | Solid color mode |
| `mode:Fade` | Fading effect |
| `mode:Breathe` | Breathing effect |
| `mode:Strobe` | Strobe effect |
| `mode:Pulse` | Pulse effect |
| `mode:Rainbow` | Rainbow effect |

**Example:**
```bash
curl -X POST http://192.168.1.100/command -d "mode:Breathe"
```

### Color Commands

| Command | Description |
|---------|-------------|
| `color:next` | Switch to next color |
| `color:Red` | Set color to Red |
| `color:Green` | Set color to Green |
| `color:Blue` | Set color to Blue |
| `color:White` | Set color to White |
| `color:Yellow` | Set color to Yellow |
| `color:Magenta` | Set color to Magenta |
| `color:Cyan` | Set color to Cyan |
| `color:Orange` | Set color to Orange |
| `color:Purple` | Set color to Purple |
| `color:Pink` | Set color to Pink |

**Example:**
```bash
curl -X POST http://192.168.1.100/command -d "color:Blue"
```

### Brightness Commands

| Command | Description |
|---------|-------------|
| `brightness:<0-255>` | Set brightness level |

**Examples:**
```bash
# 100% brightness
curl -X POST http://192.168.1.100/command -d "brightness:255"

# 50% brightness
curl -X POST http://192.168.1.100/command -d "brightness:128"

# 0% brightness (off)
curl -X POST http://192.168.1.100/command -d "brightness:0"
```

### Speed Commands

| Command | Description |
|---------|-------------|
| `speed:<0-9>` | Set effect speed (0=slowest, 9=fastest) |

**Example:**
```bash
curl -X POST http://192.168.1.100/command -d "speed:7"
```

## Setup Endpoints (AP Mode Only)

### GET /

Shows the WiFi setup page when in AP mode, or a simple status page when in STA mode.

**AP Mode Response:**
HTML form for WiFi configuration

**STA Mode Response:**
HTML page with API endpoint information

### POST /setup

Configure WiFi credentials (only available in AP mode).

**Request:**
```http
POST /setup HTTP/1.1
Host: 192.168.4.1
Content-Type: application/x-www-form-urlencoded

ssid=MyWiFiNetwork&password=MyPassword123
```

**Response:**
HTML confirmation page. Device will restart after 3 seconds.

## Error Responses

### 400 Bad Request
```json
{
  "error": "Invalid command"
}
```

### 403 Forbidden
```json
{
  "error": "Not in AP mode"
}
```

## Integration Examples

### JavaScript (Fetch API)

```javascript
// Get current state
const getState = async (ip) => {
  const response = await fetch(`http://${ip}/state`);
  const state = await response.json();
  console.log(state);
};

// Send command
const sendCommand = async (ip, command) => {
  const response = await fetch(`http://${ip}/command`, {
    method: 'POST',
    headers: {
      'Content-Type': 'text/plain',
    },
    body: command,
  });
  const state = await response.json();
  console.log(state);
};

// Usage
await getState('192.168.1.100');
await sendCommand('192.168.1.100', 'power:on');
await sendCommand('192.168.1.100', 'color:Blue');
```

### Python (requests)

```python
import requests

# Get current state
def get_state(ip):
    response = requests.get(f'http://{ip}/state')
    return response.json()

# Send command
def send_command(ip, command):
    response = requests.post(
        f'http://{ip}/command',
        data=command,
        headers={'Content-Type': 'text/plain'}
    )
    return response.json()

# Usage
state = get_state('192.168.1.100')
print(state)

send_command('192.168.1.100', 'power:on')
send_command('192.168.1.100', 'color:Blue')
send_command('192.168.1.100', 'brightness:200')
```

### Node.js (axios)

```javascript
const axios = require('axios');

// Get current state
const getState = async (ip) => {
  const response = await axios.get(`http://${ip}/state`);
  return response.data;
};

// Send command
const sendCommand = async (ip, command) => {
  const response = await axios.post(`http://${ip}/command`, command, {
    headers: { 'Content-Type': 'text/plain' },
  });
  return response.data;
};

// Usage
(async () => {
  const state = await getState('192.168.1.100');
  console.log(state);
  
  await sendCommand('192.168.1.100', 'power:on');
  await sendCommand('192.168.1.100', 'mode:Rainbow');
})();
```

### cURL Examples

```bash
# Get state
curl http://192.168.1.100/state

# Turn on
curl -X POST http://192.168.1.100/command -d "power:on"

# Change to rainbow mode
curl -X POST http://192.168.1.100/command -d "mode:Rainbow"

# Set brightness to 75%
curl -X POST http://192.168.1.100/command -d "brightness:191"

# Set color to purple and brightness to 50%
curl -X POST http://192.168.1.100/command -d "color:Purple" && \
curl -X POST http://192.168.1.100/command -d "brightness:128"
```

## Home Automation Integration

### Home Assistant

```yaml
# configuration.yaml
light:
  - platform: rest
    name: RGB Lighting
    resource: http://192.168.1.100/state
    state_resource: http://192.168.1.100/state
    method: POST
    headers:
      Content-Type: text/plain
    payload: 'power:toggle'
```

### Node-RED

Create HTTP request nodes with the endpoints above.

## Rate Limiting

No rate limiting implemented. For intensive polling, recommend:
- Maximum 1 request per second for state polling
- Minimum 100ms delay between commands

## WebSocket Support

Not currently implemented. Consider adding for real-time bidirectional communication in future versions.

## CORS

CORS is enabled for all origins to allow web applications to communicate with the device.

## Security Considerations

⚠️ **Important Security Notes:**

1. **No Authentication**: The API has no authentication. Anyone on your network can control the device.
2. **HTTP Only**: Data is transmitted unencrypted.
3. **Production Use**: For production deployments, consider:
   - Adding authentication (API keys, OAuth, etc.)
   - Implementing HTTPS
   - Adding rate limiting
   - Restricting CORS origins
   - Implementing user management

## Changelog

### v1.0.0
- Initial API release
- Basic control commands
- State retrieval
- WiFi setup endpoints
