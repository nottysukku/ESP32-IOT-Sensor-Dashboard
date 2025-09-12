# 🏃 PIR Motion Sensor Analytics

Advanced HC-SR501 PIR motion sensor project with real-time analytics, pattern recognition, and optimized response times for ESP32.

## 📋 Components Required

### Core Components
- **ESP32-DOWD-V3 Development Board** (1x)
- **HC-SR501 PIR Motion Sensor** (1x)
- **LED indicators** (2x - Red for motion, Blue for status)
- **Buzzer** (1x - Optional for audio alerts)
- **Resistors** (220Ω for LEDs)
- **Breadboard & Jumper Wires**

### Optional Components
- **External 5V power supply** (for improved PIR performance)
- **Enclosure** for permanent installation

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

### 🏃 HC-SR501 PIR Motion Sensor
```
PIR Pin → ESP32 Pin → Notes
VCC     → 5V (VIN)  → For best performance use 5V
GND     → GND       → Common ground
OUT     → D5 (GPIO5)→ Digital signal output
```

### 💡 LED Indicators
```
Component        → ESP32 Pin    → Resistor → Notes
Motion LED (Red) → D25 (GPIO25) → 220Ω    → Lights up on motion
Status LED (Blue)→ D26 (GPIO26) → 220Ω    → System status
Built-in LED     → D2 (GPIO2)   → None    → WiFi status
```

### 🔊 Buzzer (Optional)
```
Buzzer Pin → ESP32 Pin → Notes
Positive   → D18 (GPIO18) → PWM capable pin
Negative   → GND          → Common ground
```

## 🎛️ HC-SR501 PIR Sensor Configuration

### Physical Adjustments on PIR Module
```
┌─────────────────────────────┐
│    HC-SR501 PIR Module      │
│                             │
│  [Sensitivity]  [Time Delay]│
│      POT           POT      │
│       ↑             ↑       │
│   Clockwise     Clockwise   │
│   = Higher      = Longer    │
│   Sensitivity   Delay       │
│                             │
│   Jumper: [ • • ]          │
│           H   L             │
│    (Retriggering Mode)      │
└─────────────────────────────┘
```

### Recommended Settings
- **Sensitivity:** Medium (half turn clockwise)
- **Time Delay:** Minimum (fully counter-clockwise)
- **Jumper Position:** L (Low = Non-retriggering mode)
- **Detection Range:** 3-7 meters
- **Detection Angle:** ~120 degrees

## 📦 Required Libraries

Install these libraries through Arduino IDE Library Manager:

```
Tools → Manage Libraries → Search and Install:
```

1. **WiFi** (Built-in with ESP32)
2. **WebServer** (Built-in with ESP32)
3. **ArduinoJson** by Benoit Blanchon

### Installation Steps:
1. Open Arduino IDE
2. Go to `Tools` → `Manage Libraries`
3. Search for "ArduinoJson" → Install latest version

## ⚙️ Setup Instructions

### 1. Hardware Assembly

#### Step-by-Step Wiring:
1. **Power Off** ESP32 completely
2. **Connect PIR Sensor:**
   ```
   PIR VCC → ESP32 VIN (5V pin)
   PIR GND → ESP32 GND
   PIR OUT → ESP32 GPIO5
   ```
3. **Connect Motion LED (Red):**
   ```
   LED Long leg (+) → 220Ω Resistor → ESP32 GPIO25
   LED Short leg (-) → ESP32 GND
   ```
4. **Connect Status LED (Blue):**
   ```
   LED Long leg (+) → 220Ω Resistor → ESP32 GPIO26
   LED Short leg (-) → ESP32 GND
   ```
5. **Connect Buzzer (Optional):**
   ```
   Buzzer (+) → ESP32 GPIO18
   Buzzer (-) → ESP32 GND
   ```

#### PIR Sensor Calibration:
1. **Initial Setup:** Set sensitivity to middle, time delay to minimum
2. **Power On:** Allow 60 seconds for PIR initialization
3. **Test Motion:** Walk in front of sensor at different distances
4. **Adjust Sensitivity:** Fine-tune based on your detection needs

### 2. Software Configuration

#### Arduino IDE Setup:
1. **Install ESP32 Board Support:**
   - File → Preferences
   - Add URL: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Tools → Board → Boards Manager → Install "ESP32"

2. **Board Configuration:**
   - Tools → Board → ESP32 Arduino → "ESP32 Dev Module"
   - Tools → Port → Select your ESP32 port
   - Tools → Upload Speed → 921600
   - Tools → Flash Frequency → 80MHz

#### Code Configuration:
1. **Update WiFi Credentials:**
   ```cpp
   const char* ssid = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

2. **Adjust Detection Settings (Optional):**
   ```cpp
   #define MOTION_COOLDOWN 2000    // 2 seconds between detections
   #define DEBOUNCE_TIME 50        // 50ms debounce
   ```

### 3. Upload and Test

#### Upload Process:
1. Open `pir_sensor_analytics.ino` in Arduino IDE
2. Verify code compiles without errors
3. Upload to ESP32 (Ctrl+U)
4. Monitor serial output (115200 baud)

#### Initial Testing:
1. **Power On:** Blue status LED should blink during WiFi connection
2. **WiFi Connected:** Built-in LED (GPIO2) stays on
3. **Motion Test:** Red LED should light up when motion detected
4. **Serial Monitor:** Should show IP address and motion events

## 🌐 Web Dashboard Access

### Finding Your ESP32 IP Address:
1. **Serial Monitor Method:**
   - Open Tools → Serial Monitor
   - Set baud rate to 115200
   - Look for "IP address: 192.168.x.x"

2. **Router Admin Panel:**
   - Log into your router (usually 192.168.1.1)
   - Look for "ESP32" in connected devices

### Accessing the Dashboard:
1. **Open Web Browser** (Chrome, Firefox, Safari)
2. **Navigate to:** `http://192.168.x.x` (your ESP32's IP)
3. **Bookmark** the page for easy access

### Dashboard Features:
- **📊 Real-time Motion Chart:** Live motion detection timeline
- **🎯 Motion Statistics:** Count, frequency, patterns
- **⏱️ Response Analytics:** Detection speed and accuracy
- **📈 Pattern Recognition:** Daily/hourly motion trends
- **⚙️ Settings Panel:** Sensitivity and threshold adjustments
- **📱 Mobile Responsive:** Works on phones and tablets

## 🔧 Troubleshooting Guide

### ❌ PIR Sensor Not Detecting Motion

**Symptoms:** Red LED never lights up, no motion events in serial monitor

**Solutions:**
1. **Check Wiring:**
   ```
   Verify: PIR OUT → ESP32 GPIO5
   Verify: PIR VCC → ESP32 VIN (5V)
   Verify: PIR GND → ESP32 GND
   ```

2. **Power Issues:**
   - Use 5V for PIR VCC (not 3.3V)
   - Check power supply can provide enough current
   - Try different USB cable/power source

3. **PIR Calibration:**
   - Allow 60+ seconds for PIR initialization after power-on
   - Adjust sensitivity potentiometer (clockwise = more sensitive)
   - Set time delay to minimum (counter-clockwise)
   - Set jumper to L position (non-retriggering)

4. **Environmental Factors:**
   - PIR detects heat/IR changes, not just movement
   - Avoid pointing at heat sources (heaters, direct sunlight)
   - Optimal detection: room temperature, medium lighting
   - Detection range: 3-7 meters maximum

### ❌ False Motion Detections

**Symptoms:** Motion detected when no one is around

**Solutions:**
1. **Reduce Sensitivity:**
   - Turn sensitivity potentiometer counter-clockwise
   - Start with minimum sensitivity and gradually increase

2. **Environmental Stability:**
   - Ensure PIR is not near air vents or heat sources
   - Avoid direct sunlight on PIR sensor
   - Let system stabilize for 2-3 minutes after power-on

3. **Electrical Interference:**
   - Keep PIR wires away from power cables
   - Use shorter jumper wires if possible
   - Add 100nF capacitor between PIR VCC and GND

### ❌ WiFi Connection Issues

**Symptoms:** Blue LED keeps blinking, no IP address in serial monitor

**Solutions:**
1. **Network Compatibility:**
   - Ensure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
   - Check WiFi password is correct (case-sensitive)
   - Move ESP32 closer to router

2. **WiFi Credentials:**
   ```cpp
   const char* ssid = "Exact_WiFi_Name";     // Case sensitive
   const char* password = "Exact_Password";   // Special chars OK
   ```

3. **Router Settings:**
   - Ensure WiFi is not hidden
   - Check MAC address filtering isn't blocking ESP32
   - Try restarting router

### ❌ Web Dashboard Not Loading

**Symptoms:** Browser can't reach the IP address

**Solutions:**
1. **Network Issues:**
   - Verify ESP32 and device are on same WiFi network
   - Try accessing from different device (phone, tablet)
   - Ping the ESP32 IP from command prompt

2. **IP Address Changes:**
   - ESP32 IP may change after router restart
   - Check serial monitor for current IP
   - Set static IP in router if needed

3. **Browser Issues:**
   - Clear browser cache and cookies
   - Try different browser (Chrome, Firefox, Safari)
   - Disable ad blockers or browser extensions

### ❌ Slow or Delayed Response

**Symptoms:** Motion detected but LED/dashboard updates slowly

**Solutions:**
1. **Code Optimization:**
   - Current code is optimized for 10ms polling
   - Ensure no additional delays in main loop
   - Check WiFi signal strength

2. **PIR Settings:**
   - Set time delay potentiometer to minimum
   - Use non-retriggering mode (jumper on L)
   - Ensure proper 5V power supply

## 📊 Technical Specifications

### Performance Metrics
- **Detection Speed:** <100ms response time
- **Polling Rate:** 10ms (very fast)
- **False Positive Rate:** <1% with proper calibration
- **Range:** 3-7 meters (adjustable)
- **Angle:** ~120 degrees detection cone
- **Power Consumption:** ~150mA active, ~20mA idle

### Environmental Specifications
- **Operating Temperature:** -15°C to +70°C
- **Operating Humidity:** 93% RH max
- **Detection Heat Source:** >37°C (human body temperature)
- **Optimal Range:** 3-5 meters for best accuracy

### Communication Specifications
- **WiFi Standard:** 802.11 b/g/n (2.4GHz only)
- **Web Server:** HTTP on port 80
- **Update Rate:** 1 second dashboard refresh
- **Data Retention:** Last 100 motion events

## 📈 Analytics Features

### Real-time Monitoring
- **Live Motion Chart:** Continuous timeline of detections
- **Current Status:** Active/inactive motion state
- **Response Time:** Actual detection speed measurement
- **Event Counter:** Total motions detected this session

### Historical Analysis
- **Pattern Recognition:** Daily and hourly motion patterns
- **Frequency Analysis:** Motion events per hour/day
- **Response Statistics:** Average, min, max response times
- **Heat Maps:** Time-based motion intensity

### Data Export
- **CSV Download:** Complete motion log with timestamps
- **JSON API:** Real-time data access for integration
- **Chart Screenshots:** Save visual analytics
- **Statistical Reports:** Automated pattern summaries

## 🔮 Advanced Features

### Customization Options
- **Detection Sensitivity:** Software-based fine-tuning
- **Motion Timeout:** Configurable reset periods
- **Alert Thresholds:** Custom motion frequency alerts
- **Dashboard Themes:** Multiple UI color schemes

### Integration Capabilities
- **MQTT Support:** Ready for home automation
- **Webhook Notifications:** Send alerts to external services
- **API Endpoints:** RESTful API for custom applications
- **Multiple Sensors:** Support for sensor networks

## 🎯 Use Cases & Applications

### Home Security
- **Intrusion Detection:** Monitor entry points
- **Perimeter Security:** Outdoor motion sensing
- **Room Occupancy:** Track space utilization
- **Pet Monitoring:** Detect pet activity

### Smart Home Integration
- **Automatic Lighting:** Motion-triggered lights
- **HVAC Control:** Occupancy-based temperature
- **Security Cameras:** Motion-activated recording
- **Energy Saving:** Presence-based automation

### Commercial Applications
- **Visitor Counting:** Track foot traffic
- **Security Monitoring:** Warehouse/office security
- **Energy Management:** Occupancy-based controls
- **Analytics Dashboard:** Business intelligence

## 🔍 Debugging Tips

### Serial Monitor Messages
```
Normal Operation:
- "WiFi connected! IP address: X.X.X.X"
- "PIR Motion Sensor Started!"
- "Motion detected at: [timestamp]"
- "Motion ended at: [timestamp]"

Error Messages:
- "WiFi connection failed" → Check credentials
- "PIR sensor timeout" → Check wiring
- "Memory low" → Restart ESP32
```

### LED Status Indicators
- **Built-in LED (GPIO2):** WiFi connection status
  - Blinking: Connecting to WiFi
  - Solid ON: Connected to WiFi
  - OFF: WiFi disconnected

- **Blue LED (GPIO26):** System status
  - Slow blink: Normal operation
  - Fast blink: Processing motion
  - Solid: System error

- **Red LED (GPIO25):** Motion detection
  - ON: Motion currently detected
  - OFF: No motion

### Testing Procedures
1. **Power-On Test:** All LEDs should briefly flash
2. **WiFi Test:** Built-in LED should turn solid after connection
3. **Motion Test:** Walk in front of sensor, red LED should light
4. **Dashboard Test:** Access web interface and see real-time updates
5. **Range Test:** Test detection at various distances

---

## 🎯 Quick Start Checklist

- [ ] ESP32-DOWD-V3 board ready
- [ ] HC-SR501 PIR sensor acquired
- [ ] All wiring completed per diagram
- [ ] Arduino IDE with ESP32 support installed
- [ ] ArduinoJson library installed
- [ ] WiFi credentials updated in code
- [ ] Code uploaded successfully
- [ ] PIR sensor calibrated (60 seconds initialization)
- [ ] Serial monitor shows IP address
- [ ] Web dashboard accessible and updating
- [ ] Motion detection working properly
- [ ] LEDs responding to motion events

**🎉 Your PIR Motion Sensor Analytics system is ready!**
