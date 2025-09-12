# 🔐 ESP32 IoT Security System

A comprehensive multi-sensor security system with real-time web dashboard, temperature monitoring, motion detection, and emergency controls.

## 📋 Components Required

### Core Components
- **ESP32-DOWD-V3 Development Board** (1x)
- **DHT11 Temperature & Humidity Sensor** (1x)
- **HC-SR501 PIR Motion Sensor** (1x)
- **LED indicators** (2x - Red & Green)
- **Buzzer/Alarm** (1x)
- **Relay Module** (1x - for emergency devices)
- **Push Button** (1x - Emergency button)
- **Resistors** (220Ω for LEDs, 10kΩ pull-up for button)
- **Breadboard & Jumper Wires**

### Optional Components
- **External power supply** (5V/2A recommended for relays)
- **Enclosure box** for permanent installation

## 🔌 Wiring Connections

### ESP32-DOWD-V3 Pin Layout Reference
```
               ESP32-DOWD-V3
    ┌─────────────────────────────┐
    │ 3V3  [ ]               [ ] GND │
    │ EN   [ ]               [ ] D23 │
    │ D36  [ ]  (Not Available) [ ] D22 │
    │ D39  [ ]               [ ] D21 │
    │ D34  [ ]               [ ] D19 │
    │ D35  [ ]               [ ] D18 │
    │ D32  [ ]               [ ] D5  │
    │ D33  [ ]               [ ] D17 │
    │ D25  [ ]               [ ] D16 │
    │ D26  [ ]               [ ] D4  │
    │ D27  [ ]               [ ] D0  │
    │ D14  [ ]               [ ] D2  │
    │ D12  [ ]               [ ] D15 │
    │ GND  [ ]               [ ] 3V3 │
    │ D13  [ ]               [ ] VIN │
    └─────────────────────────────┘
```

### 🌡️ DHT11 Temperature Sensor
```
DHT11 Pin → ESP32 Pin
VCC       → 3V3
GND       → GND
DATA      → D4 (GPIO4)
```

### 🏃 PIR Motion Sensor (HC-SR501)
```
PIR Pin   → ESP32 Pin
VCC       → 3V3 or 5V (check your module)
GND       → GND
OUT       → D5 (GPIO5)
```

### 💡 LED Indicators
```
Component     → ESP32 Pin    → Resistor
Red LED (+)   → D25 (GPIO25) → 220Ω → GND
Green LED (+) → D26 (GPIO26) → 220Ω → GND
Built-in LED  → D2 (GPIO2)   → (No resistor needed)
```

### 🔊 Buzzer/Alarm
```
Buzzer Pin → ESP32 Pin
Positive   → D18 (GPIO18)
Negative   → GND
```

### 🔌 Relay Module
```
Relay Pin → ESP32 Pin
VCC       → 5V (VIN) or 3V3
GND       → GND
IN        → D19 (GPIO19)
```

### 🚨 Emergency Button
```
Button Terminal → Connection
Terminal 1      → D23 (GPIO23)
Terminal 2      → GND
(Internal pull-up resistor used)
```

## 📦 Required Libraries

Install these libraries through Arduino IDE Library Manager:

```
Tools → Manage Libraries → Search and Install:
```

1. **WiFi** (Built-in with ESP32)
2. **WebServer** (Built-in with ESP32)
3. **ArduinoJson** by Benoit Blanchon
4. **DHT sensor library** by Adafruit
5. **Adafruit Unified Sensor** (dependency for DHT)

### Installation Steps:
1. Open Arduino IDE
2. Go to `Tools` → `Manage Libraries`
3. Search for "ArduinoJson" → Install latest version
4. Search for "DHT sensor library" → Install Adafruit version
5. Search for "Adafruit Unified Sensor" → Install latest version

## ⚙️ Setup Instructions

### 1. Hardware Assembly
1. **Power Off** your ESP32 before making connections
2. Follow the wiring diagram above carefully
3. **Double-check all connections** before powering on
4. Use a breadboard for prototyping
5. Ensure proper power supply (USB or external 5V)

### 2. Software Configuration
1. **Download Arduino IDE** (latest version)
2. **Install ESP32 Board Support:**
   - File → Preferences
   - Add this URL to "Additional Board Manager URLs":
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
   - Tools → Board → Boards Manager → Search "ESP32" → Install

3. **Board Selection:**
   - Tools → Board → ESP32 Arduino → "ESP32 Dev Module"
   - Tools → Port → Select your ESP32 port (usually COM3, COM4, etc.)

### 3. Code Upload
1. **Update WiFi Credentials** in the code:
   ```cpp
   const char* ssid = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

2. **Upload the Code:**
   - Open `sketch_sep9a_main.ino` in Arduino IDE
   - Press Ctrl+U or click Upload button
   - Wait for "Done uploading" message

3. **Monitor Serial Output:**
   - Tools → Serial Monitor
   - Set baud rate to 115200
   - Look for WiFi connection and IP address

## 🌐 Web Dashboard Access

### Accessing the Dashboard
1. **Find ESP32 IP Address:**
   - Check Serial Monitor after upload
   - Look for message: "IP address: 192.168.x.x"

2. **Open Web Browser:**
   - Navigate to: `http://192.168.x.x` (replace with your IP)
   - Bookmark the page for easy access

### Dashboard Features
- **🏠 Main Dashboard:** Real-time sensor readings
- **📊 Analytics:** Historical data and charts
- **⚙️ Settings:** Threshold adjustments and calibration
- **🚨 Emergency Controls:** Manual alarm triggers
- **📱 Mobile Friendly:** Responsive design

## 🔧 Troubleshooting

### Common Issues & Solutions

#### ❌ ESP32 Not Connecting to WiFi
**Solution:**
- Check WiFi credentials are correct
- Ensure WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
- Move ESP32 closer to router
- Check for special characters in password

#### ❌ DHT11 Reading NaN/Invalid Values
**Solution:**
- Check wiring connections (VCC, GND, DATA)
- DHT11 needs 3.3V, not 5V
- Add small delay between readings
- Try different DHT11 module (might be faulty)

#### ❌ PIR Sensor Always Triggering
**Solution:**
- Adjust sensitivity potentiometer on PIR module
- Check wiring (PIR OUT pin to GPIO5)
- Allow 60 seconds for PIR calibration after power-on
- Keep PIR away from heat sources

#### ❌ Web Dashboard Not Loading
**Solution:**
- Check ESP32 IP address in Serial Monitor
- Ensure ESP32 and device are on same WiFi network
- Try refreshing browser or clearing cache
- Check firewall settings

#### ❌ Compilation Errors
**Solution:**
- Install all required libraries
- Select correct ESP32 board type
- Update Arduino IDE to latest version
- Check for syntax errors in code

## 📊 System Specifications

### Technical Details
- **Operating Voltage:** 3.3V (ESP32)
- **Input Voltage:** 5V via USB or VIN
- **Current Consumption:** ~150mA (active), ~10mA (sleep)
- **WiFi Standard:** 802.11 b/g/n (2.4GHz)
- **Temperature Range:** -40°C to +80°C (DHT11)
- **Humidity Range:** 5% to 95% RH (DHT11)
- **Motion Detection:** 3-7 meters range (PIR)

### Performance Metrics
- **Response Time:** <100ms for all sensors
- **Data Update Rate:** 1 second intervals
- **Web Dashboard Refresh:** Real-time updates
- **Memory Usage:** ~60% Flash, ~30% RAM

## 🔒 Security Features

### Built-in Security
- **Motion Detection:** Instant alerts with timestamp
- **Temperature Monitoring:** Overheating protection
- **Emergency Button:** Manual alarm activation
- **Relay Control:** Automatic device management
- **Alert System:** Visual and audio notifications

### Alert Conditions
- **High Temperature:** >30°C (configurable)
- **Motion Detected:** Immediate notification
- **Emergency Button:** Manual trigger
- **System Offline:** Connection loss detection

## 📈 Monitoring & Analytics

### Real-time Data
- Temperature and humidity trends
- Motion detection events
- System status indicators
- Alert history log

### Historical Analysis
- 24-hour data visualization
- Event pattern recognition
- Performance statistics
- Export data capability

## 🔮 Future Enhancements

### Possible Upgrades
- **Camera Integration:** ESP32-CAM module
- **SMS Notifications:** GSM module integration
- **Cloud Logging:** Firebase or AWS IoT
- **Mobile App:** Dedicated smartphone app
- **Multiple Sensors:** Expand sensor network
- **Battery Backup:** UPS integration

## 📞 Support & Documentation

### Additional Resources
- **ESP32 Official Documentation:** https://docs.espressif.com/
- **Arduino ESP32 Guide:** https://github.com/espressif/arduino-esp32
- **DHT11 Datasheet:** Available online
- **PIR Sensor Guide:** HC-SR501 documentation

### Contact Information
- **Project Repository:** Check GitHub for updates
- **Community Support:** Arduino forums and ESP32 communities
- **Hardware Issues:** Check component datasheets

---

## 🎯 Quick Start Checklist

- [ ] All components acquired
- [ ] Wiring completed according to diagram
- [ ] Arduino IDE installed with ESP32 support
- [ ] Required libraries installed
- [ ] WiFi credentials updated in code
- [ ] Code uploaded successfully
- [ ] Serial Monitor shows IP address
- [ ] Web dashboard accessible
- [ ] All sensors responding correctly
- [ ] Emergency features tested

**🎉 Congratulations! Your ESP32 IoT Security System is now ready!**
