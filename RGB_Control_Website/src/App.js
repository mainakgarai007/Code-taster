import React, { useState, useEffect } from 'react';
import axios from 'axios';
import './App.css';

function App() {
  const [deviceIP, setDeviceIP] = useState(localStorage.getItem('deviceIP') || '');
  const [connected, setConnected] = useState(false);
  const [state, setState] = useState({
    power: 'off',
    color: 'Red',
    mode: 'Solid',
    brightness: 255,
    speed: 5,
    autoCycle: false,
    rainbow: false,
    wifi: 'disconnected',
    ip: '',
    mode_type: 'AP'
  });

  const colors = [
    'Red', 'Green', 'Blue', 'White', 'Yellow', 
    'Magenta', 'Cyan', 'Orange', 'Purple', 'Pink'
  ];

  const modes = [
    'Solid', 'Fade', 'Breathe', 'Strobe', 'Pulse', 'Rainbow'
  ];

  useEffect(() => {
    if (deviceIP) {
      fetchState();
      const interval = setInterval(fetchState, 2000);
      return () => clearInterval(interval);
    }
  }, [deviceIP]);

  const fetchState = async () => {
    try {
      const response = await axios.get(`http://${deviceIP}/state`, { timeout: 5000 });
      setState(response.data);
      setConnected(true);
    } catch (error) {
      console.error('Failed to fetch state:', error);
      setConnected(false);
    }
  };

  const sendCommand = async (command) => {
    try {
      await axios.post(`http://${deviceIP}/command`, command, {
        headers: { 'Content-Type': 'text/plain' },
        timeout: 5000
      });
      setTimeout(fetchState, 200);
    } catch (error) {
      console.error('Failed to send command:', error);
    }
  };

  const handleConnect = () => {
    localStorage.setItem('deviceIP', deviceIP);
    fetchState();
  };

  const togglePower = () => {
    sendCommand('power:toggle');
  };

  const changeColor = (color) => {
    sendCommand(`color:${color}`);
  };

  const changeMode = (mode) => {
    sendCommand(`mode:${mode}`);
  };

  const changeBrightness = (e) => {
    sendCommand(`brightness:${e.target.value}`);
  };

  const changeSpeed = (e) => {
    sendCommand(`speed:${e.target.value}`);
  };

  if (!deviceIP || !connected) {
    return (
      <div className="App">
        <div className="connect-container">
          <h1>RGB Lighting OS v1.0</h1>
          <div className="connect-box">
            <h2>Connect to Device</h2>
            <p>Enter your ESP8266 IP address</p>
            <input
              type="text"
              placeholder="192.168.1.100 or 192.168.4.1"
              value={deviceIP}
              onChange={(e) => setDeviceIP(e.target.value)}
              onKeyPress={(e) => e.key === 'Enter' && handleConnect()}
            />
            <button onClick={handleConnect}>Connect</button>
            <div className="help">
              <p><strong>AP Mode:</strong> 192.168.4.1</p>
              <p><strong>STA Mode:</strong> Check serial monitor or router</p>
            </div>
          </div>
        </div>
      </div>
    );
  }

  return (
    <div className="App">
      <header className="header">
        <h1>RGB Lighting OS v1.0</h1>
        <div className="connection-status">
          <span className={`status-dot ${connected ? 'connected' : 'disconnected'}`}></span>
          <span>{deviceIP}</span>
          <button className="disconnect-btn" onClick={() => { setDeviceIP(''); setConnected(false); }}>
            Disconnect
          </button>
        </div>
      </header>

      <div className="control-panel">
        {/* Power Control */}
        <div className="control-section">
          <h2>Power</h2>
          <button 
            className={`power-btn ${state.power === 'on' ? 'on' : 'off'}`}
            onClick={togglePower}
          >
            {state.power === 'on' ? '● ON' : '○ OFF'}
          </button>
        </div>

        {/* Color Selection */}
        <div className="control-section">
          <h2>Colors</h2>
          <div className="color-grid">
            {colors.map(color => (
              <button
                key={color}
                className={`color-btn ${state.color === color ? 'active' : ''}`}
                style={{ 
                  backgroundColor: color.toLowerCase(),
                  color: ['White', 'Yellow', 'Cyan', 'Pink'].includes(color) ? '#333' : '#fff'
                }}
                onClick={() => changeColor(color)}
              >
                {color}
              </button>
            ))}
          </div>
        </div>

        {/* Mode Selection */}
        <div className="control-section">
          <h2>Effects</h2>
          <div className="mode-grid">
            {modes.map(mode => (
              <button
                key={mode}
                className={`mode-btn ${state.mode === mode ? 'active' : ''}`}
                onClick={() => changeMode(mode)}
              >
                {mode}
              </button>
            ))}
          </div>
        </div>

        {/* Brightness Control */}
        <div className="control-section">
          <h2>Brightness: {Math.round((state.brightness / 255) * 100)}%</h2>
          <input
            type="range"
            min="0"
            max="255"
            value={state.brightness}
            onChange={changeBrightness}
            className="slider"
          />
        </div>

        {/* Speed Control */}
        <div className="control-section">
          <h2>Speed: {state.speed}/9</h2>
          <input
            type="range"
            min="0"
            max="9"
            value={state.speed}
            onChange={changeSpeed}
            className="slider"
          />
        </div>

        {/* Quick Actions */}
        <div className="control-section">
          <h2>Quick Actions</h2>
          <div className="quick-actions">
            <button onClick={() => sendCommand('color:next')}>Next Color</button>
            <button onClick={() => sendCommand('mode:next')}>Next Mode</button>
          </div>
        </div>

        {/* Scenes */}
        <div className="control-section">
          <h2>Scenes</h2>
          <div className="scene-grid">
            <button className="scene-btn" onClick={() => sendCommand('scene:relax')}>🌅 Relax</button>
            <button className="scene-btn" onClick={() => sendCommand('scene:party')}>🎉 Party</button>
            <button className="scene-btn" onClick={() => sendCommand('scene:focus')}>💡 Focus</button>
            <button className="scene-btn" onClick={() => sendCommand('scene:sleep')}>😴 Sleep</button>
            <button className="scene-btn" onClick={() => sendCommand('scene:romantic')}>💕 Romantic</button>
            <button className="scene-btn" onClick={() => sendCommand('scene:energize')}>⚡ Energize</button>
          </div>
        </div>

        {/* Status Info */}
        <div className="control-section status-info">
          <h2>Device Status</h2>
          <div className="status-grid">
            <div className="status-item">
              <span className="label">WiFi:</span>
              <span className={`value ${state.wifi === 'connected' ? 'success' : 'warning'}`}>
                {state.wifi}
              </span>
            </div>
            <div className="status-item">
              <span className="label">Mode:</span>
              <span className="value">{state.mode_type}</span>
            </div>
            <div className="status-item">
              <span className="label">IP:</span>
              <span className="value">{state.ip}</span>
            </div>
            <div className="status-item">
              <span className="label">Auto Cycle:</span>
              <span className="value">{state.autoCycle ? 'ON' : 'OFF'}</span>
            </div>
          </div>
        </div>
      </div>

      <footer className="footer">
        <p>RGB Lighting OS v1.0 - IoT RGB LED Control System</p>
      </footer>
    </div>
  );
}

export default App;
