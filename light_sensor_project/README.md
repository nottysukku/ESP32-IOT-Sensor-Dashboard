# ☀️ LDR Light Sensor Analytics Project

Advanced photosensitive resistor (LDR) project with OLED display, real-time analytics, automatic LED control, and comprehensive light level monitoring for ESP32.

## 📋 Components Required

### Core Components
- **ESP32-DOWD-V3 Development Board** (1x)
- **LDR Photosensitive Resistor** (1x) - Any standard LDR
- **0.96" OLED Display (SSD1306)** (1x) - I2C interface
- **10kΩ Resistor** (1x) - Pull-up for LDR voltage divider
- **220Ω Resistors** (2x) - For LED current limiting
- **LEDs** (2x) - External LED for auto-control + status LED
- **Buzzer** (1x) - Audio alerts for light changes
- **Push Button** (1x) - Manual calibration trigger
- **Breadboard & Jumper Wires**

### Optional Components
- **Enclosure** for permanent installation
- **External 5V power supply** for brighter LEDs
- **Light hood** for LDR to reduce ambient interference

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

### 💡 LDR Voltage Divider Circuit
```
               3.3V
                |
             ┌──┴──┐
             │10kΩ │ (Pull-up Resistor)
             │     │
             └──┬──┘
                │
                ├─────── To ESP32 GPIO35 (ADC1_CH7)
                │
             ┌──┴──┐
             │ LDR │ (Light Dependent Resistor)
             │     │ (Resistance varies: 1kΩ-1MΩ)
             └──┬──┘
                │
               GND

Note: In bright light, LDR resistance decreases
      In dark conditions, LDR resistance increases
```

### 🔌 Detailed Pin Connections

#### 🌟 LDR Light Sensor
```
Connection Point → ESP32 Pin → Notes
LDR Terminal 1   → 3V3       → Via 10kΩ pull-up resistor
LDR Terminal 2   → GND       → Direct connection
Voltage Tap      → D35 (GPIO35) → ADC1_CH7 analog input
```

#### 📺 OLED Display (SSD1306 I2C)
```
OLED Pin → ESP32 Pin → Notes
VCC      → 3V3       → 3.3V power supply
GND      → GND       → Common ground
SCL      → D22 (GPIO22) → I2C clock line
SDA      → D21 (GPIO21) → I2C data line
```

#### 💡 External LED (Auto Control)
```
LED Component    → ESP32 Pin    → Resistor → Notes
External LED (+) → D19 (GPIO19) → 220Ω    → Auto-controlled based on light
External LED (-) → GND          → None    → Common ground
```

#### 🔔 Status & Alert Components
```
Component        → ESP32 Pin    → Resistor → Notes
Built-in LED     → D2 (GPIO2)   → None    → System status indicator
Buzzer (+)       → D18 (GPIO18) → None    → Audio alerts for low light
Buzzer (-)       → GND          → None    → Common ground
Button Terminal 1→ D23 (GPIO23) → None    → Manual calibration (pull-up enabled)
Button Terminal 2→ GND          → None    → Common ground
```

## 📦 Required Libraries

Install these libraries through Arduino IDE Library Manager:

```
Tools → Manage Libraries → Search and Install:
```

1. **WiFi** (Built-in with ESP32)
2. **WebServer** (Built-in with ESP32)
3. **ArduinoJson** by Benoit Blanchon
4. **Wire** (Built-in with ESP32) - for I2C communication
5. **Adafruit GFX Library** by Adafruit
6. **Adafruit SSD1306** by Adafruit

### Library Installation Steps:
1. **Open Arduino IDE**
2. **Go to Tools → Manage Libraries**
3. **Install the following in order:**
   ```
   Search "ArduinoJson" → Install latest version
   Search "Adafruit GFX" → Install latest version  
   Search "Adafruit SSD1306" → Install latest version
   ```

### Important Library Notes:
- **Adafruit SSD1306** depends on **Adafruit GFX**
- Install **GFX library first**, then **SSD1306**
- If compilation errors occur, update all Adafruit libraries

## ⚙️ Setup Instructions

### 1. Hardware Assembly

#### Step 1: LDR Voltage Divider Circuit
```
Assembly Order:
1. Place 10kΩ resistor on breadboard
2. Connect one end to ESP32 3V3 pin
3. Connect other end to breadboard rail (this becomes the tap point)
4. Connect LDR between the tap point and GND
5. Connect wire from tap point to ESP32 GPIO35
```

#### Step 2: OLED Display Connection
```
OLED Wiring:
1. OLED VCC  → ESP32 3V3
2. OLED GND  → ESP32 GND  
3. OLED SCL  → ESP32 GPIO22 (I2C Clock)
4. OLED SDA  → ESP32 GPIO21 (I2C Data)
```

#### Step 3: External Components
```
LED Connection:
External LED (+) → 220Ω resistor → ESP32 GPIO19
External LED (-) → ESP32 GND

Buzzer Connection:
Buzzer (+) → ESP32 GPIO18
Buzzer (-) → ESP32 GND

Button Connection:
Button terminal 1 → ESP32 GPIO23
Button terminal 2 → ESP32 GND
(Internal pull-up resistor is used)
```

### 2. Software Configuration

#### Arduino IDE Setup:
1. **Install ESP32 Board Support:**
   - File → Preferences
   - Add this URL to "Additional Board Manager URLs":
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
   - Tools → Board → Boards Manager → Search "ESP32" → Install

2. **Board Configuration:**
   ```
   Tools → Board → ESP32 Arduino → "ESP32 Dev Module"
   Tools → Port → Select your ESP32 port (COM3, COM4, etc.)
   Tools → Upload Speed → 921600
   Tools → Flash Frequency → 80MHz
   Tools → Flash Size → 4MB (32Mb)
   ```

#### Code Configuration:
1. **Update WiFi Credentials:**
   ```cpp
   const char* ssid = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

2. **OLED I2C Address (if needed):**
   ```cpp
   #define OLED_ADDRESS 0x3C  // Most common address
   // If OLED doesn't work, try 0x3D
   ```

### 3. Upload and Initial Testing

#### Upload Process:
1. **Compile Code:** Click "Verify" button to check for errors
2. **Upload Code:** Click "Upload" button (or Ctrl+U)
3. **Monitor Output:** Open Tools → Serial Monitor (115200 baud)

#### Initial System Test:
1. **Power On Sequence:**
   - OLED should display "Initializing..."
   - WiFi connection process shown
   - IP address displayed on OLED and serial monitor

2. **OLED Display Test:**
   - Should show real-time light readings
   - Values should change when covering/uncovering LDR
   - Display updates every second

3. **Light Response Test:**
   - Cover LDR with hand → readings should drop
   - Shine light on LDR → readings should increase
   - External LED should turn on in low light

## 🔧 Calibration Procedures

### Automatic Calibration (Recommended)
1. **Power On System** in normal room lighting
2. **Wait 10 seconds** for system to initialize
3. **System Auto-Calibrates** based on current light level
4. **Manual Calibration:** Press the button for manual recalibration

### Manual Calibration Steps
1. **Press Calibration Button** (connected to GPIO23)
2. **OLED displays "CALIBRATING..."**
3. **System takes 20 readings** over 2 seconds
4. **Calibration values calculated** automatically
5. **"Calibration Done!"** message appears

### Calibration Theory
```
The system uses a two-point calibration:
- Current Reading = "Normal" light (50%)
- Bright Threshold = Current × 0.5 (brighter conditions)
- Dark Threshold = Current × 2.0 (darker conditions)

This provides automatic adaptation to your environment.
```

## 🌐 Web Dashboard Access

### Finding Your ESP32 IP Address:
1. **Serial Monitor Method:**
   - Tools → Serial Monitor (115200 baud)
   - Look for "IP address: 192.168.x.x"

2. **OLED Display Method:**
   - IP address briefly shown on OLED during startup
   - Also displayed in status messages

3. **Router Method:**
   - Log into router admin panel
   - Look for "ESP32" in connected devices list

### Accessing the Dashboard:
1. **Open Web Browser** (Chrome, Firefox, Safari, Edge)
2. **Navigate to:** `http://[ESP32_IP_ADDRESS]`
3. **Bookmark** the page for easy future access

### Dashboard Features:
- **🌞 Real-time Light Level:** Current percentage and condition
- **📊 Light Timeline Chart:** Live updating graph
- **⚡ Resistance Analysis:** LDR resistance calculations
- **📈 Light Condition Distribution:** Pie chart of bright/normal/dim/dark
- **🎛️ Control Panel:** Calibration and data export tools
- **📱 Mobile Responsive:** Works perfectly on smartphones and tablets

## 🔧 Troubleshooting Guide

### ❌ OLED Display Not Working

**Symptoms:** Blank OLED screen, no display output

**Solutions:**
1. **Check Wiring:**
   ```
   Verify: OLED VCC → ESP32 3V3
   Verify: OLED GND → ESP32 GND
   Verify: OLED SCL → ESP32 GPIO22
   Verify: OLED SDA → ESP32 GPIO21
   ```

2. **I2C Address Issues:**
   - Try different I2C address: Change `0x3C` to `0x3D` in code
   - Some OLED modules use different addresses

3. **Power Issues:**
   - Ensure OLED gets 3.3V (not 5V)
   - Check if OLED has power LED indicator
   - Try different ESP32 board (possible 3V3 regulator issue)

4. **Library Issues:**
   - Update Adafruit libraries to latest versions
   - Ensure Adafruit_GFX is installed before SSD1306
   - Try different OLED library if problem persists

### ❌ LDR Readings Always 0 or 4095

**Symptoms:** Light sensor readings stuck at minimum (0) or maximum (4095)

**Solutions:**
1. **Check Voltage Divider:**
   ```
   Measure voltage at GPIO35 with multimeter:
   - Should be 0.5V - 3.0V in normal lighting
   - If 0V: Check LDR connection to GND
   - If 3.3V: Check pull-up resistor connection
   ```

2. **Component Issues:**
   - **Test LDR:** Measure resistance with multimeter
     - Bright light: Should be 1kΩ - 10kΩ
     - Dark: Should be 100kΩ - 1MΩ
   - **Test 10kΩ resistor:** Should measure ~10,000 ohms

3. **Wiring Problems:**
   - **Loose connections:** Ensure all breadboard connections are firm
   - **Wrong GPIO:** Confirm using GPIO35 (ADC1_CH7)
   - **Short circuits:** Check for accidental wire bridges

4. **ADC Configuration:**
   - GPIO35 is ADC1_CH7 on ESP32-DOWD-V3
   - Ensure WiFi is not interfering with ADC (use ADC1 pins only)

### ❌ LDR Values Not Responding to Light Changes

**Symptoms:** Readings don't change when covering/uncovering LDR

**Solutions:**
1. **LDR Orientation:**
   - LDR has no polarity, but ensure light can reach the sensor surface
   - Remove any covering from the LDR sensor element
   - Position LDR away from shadows or obstructions

2. **Environmental Factors:**
   - **Room Lighting:** Significant light change needed to see response
   - **Test Range:** Try extreme conditions (flashlight vs covered)
   - **Response Time:** Allow 1-2 seconds for readings to stabilize

3. **Calibration Issues:**
   - **Re-calibrate:** Press button to recalibrate in current lighting
   - **Manual Calibration:** Set known good values in code

### ❌ External LED Not Responding

**Symptoms:** External LED doesn't turn on in low light

**Solutions:**
1. **LED Circuit:**
   ```
   Check: LED (+) → 220Ω resistor → ESP32 GPIO19
   Check: LED (-) → ESP32 GND
   Verify: LED polarity (long leg = positive)
   ```

2. **Code Logic:**
   ```cpp
   // LED turns on when light < 30%
   if (currentLight.light_percentage < 30) {
       digitalWrite(EXTERNAL_LED_PIN, HIGH);
   }
   ```

3. **GPIO Issues:**
   - Test GPIO19 with multimeter (should show 3.3V when LED should be on)
   - Try different GPIO pin if needed

### ❌ WiFi Connection Problems

**Symptoms:** System can't connect to WiFi, no IP address shown

**Solutions:**
1. **Network Compatibility:**
   - **2.4GHz Only:** ESP32 doesn't support 5GHz WiFi
   - **WPA2 Security:** Ensure router uses WPA2 (not WPA3)
   - **Network Name:** Avoid special characters in SSID

2. **Credentials:**
   ```cpp
   const char* ssid = "Exact_WiFi_Name";     // Case sensitive!
   const char* password = "Exact_Password";   // Check for typos
   ```

3. **Signal Strength:**
   - Move ESP32 closer to router during testing
   - Check for interference from other devices

### ❌ Web Dashboard Not Loading

**Symptoms:** Browser shows "Site can't be reached" or timeout

**Solutions:**
1. **Network Issues:**
   - Ensure computer/phone is on same WiFi network as ESP32
   - Try accessing from different device to isolate issue
   - Check firewall settings on computer

2. **IP Address Changes:**
   - Router may assign different IP after restart
   - Check serial monitor for current IP address
   - Consider setting static IP in router

3. **Browser Issues:**
   - Try different browser (Chrome, Firefox, Safari)
   - Clear browser cache and cookies
   - Disable ad blockers temporarily

## 📊 Technical Specifications

### Hardware Specifications
- **LDR Response Time:** 20-30ms typical
- **ADC Resolution:** 12-bit (0-4095 values)
- **Voltage Range:** 0-3.3V (ESP32 ADC)
- **Light Detection Range:** 1 lux to 10,000+ lux
- **OLED Display:** 128x64 pixels, I2C interface
- **Power Consumption:** ~150mA active, ~20mA idle

### Software Specifications
- **Sampling Rate:** 100ms per reading (10 readings/second)
- **Data Retention:** Last 100 readings stored
- **Web Update Rate:** 1 second refresh
- **Calibration Accuracy:** ±5% with proper setup

### Environmental Specifications
- **Operating Temperature:** -10°C to +85°C
- **Operating Humidity:** 0-90% RH non-condensing
- **Light Wavelength:** Visible spectrum (400-700nm)
- **LDR Resistance Range:** 1kΩ (bright) to 1MΩ (dark)

## 📈 Analytics & Features

### Real-time Monitoring
- **Live Light Level:** Current percentage and condition description
- **Voltage Measurement:** Actual ADC voltage reading
- **Resistance Calculation:** Real-time LDR resistance computation
- **Auto LED Control:** Automatic lighting based on ambient conditions

### Historical Analysis
- **Light Timeline:** Continuous graphical representation
- **Pattern Recognition:** Daily light cycle analysis
- **Condition Distribution:** Time spent in bright/normal/dim/dark
- **Resistance vs Light:** Correlation analysis

### Alert System
- **Low Light Alerts:** Configurable threshold notifications
- **Audio Alerts:** Buzzer for immediate notification
- **Visual Alerts:** LED indicators for system status
- **OLED Messages:** Real-time status on display

### Data Export & Integration
- **CSV Data Export:** Complete measurement log download
- **JSON API:** Real-time data access for integration
- **Calibration Data:** Save and restore calibration settings
- **Statistical Reports:** Automated analysis summaries

## 🔮 Advanced Applications

### Home Automation Integration
- **Smart Lighting:** Automatic room lighting control
- **Curtain Control:** Automated blinds based on sunlight
- **Security Integration:** Motion sensing with light correlation
- **Energy Management:** Daylight harvesting for energy savings

### Agricultural Monitoring
- **Greenhouse Control:** Optimal plant lighting management
- **Growth Chambers:** Precise light level control
- **Outdoor Monitoring:** Natural light tracking
- **Plant Health:** Light requirement analysis

### Commercial Applications
- **Office Lighting:** Workspace light optimization
- **Retail Displays:** Product lighting analysis
- **Photography:** Light meter for professional use
- **Quality Control:** Manufacturing light standards

## 🎯 Calibration Tips

### Optimal Calibration Conditions
1. **Stable Lighting:** Avoid fluctuating light sources
2. **Representative Environment:** Calibrate in typical usage conditions
3. **Multiple Samples:** System takes 20 readings for accuracy
4. **Regular Recalibration:** Weekly calibration recommended

### Understanding Light Conditions
```
Light Level Classifications:
- Bright (80-100%): Direct sunlight, bright indoor lighting
- Normal (20-79%): Typical indoor lighting, overcast outdoors
- Dim (5-19%): Evening lighting, dim indoor areas
- Dark (0-4%): Night time, closets, covered conditions
```

### Troubleshooting Calibration
- **Unstable Readings:** Ensure stable power supply
- **Poor Range:** Check LDR quality and wiring
- **Drift Over Time:** Temperature effects, component aging
- **Environmental Changes:** Seasonal light variations

---

## 🎯 Quick Start Checklist

- [ ] All components acquired (ESP32, LDR, OLED, resistors, LEDs, buzzer, button)
- [ ] Voltage divider circuit assembled correctly
- [ ] OLED display wired to I2C pins (GPIO21, GPIO22)
- [ ] Arduino IDE installed with ESP32 support
- [ ] All required libraries installed (ArduinoJson, Adafruit GFX, Adafruit SSD1306)
- [ ] WiFi credentials updated in code
- [ ] Code uploaded successfully without errors
- [ ] Serial monitor shows IP address and light readings
- [ ] OLED display showing real-time data
- [ ] LDR responding to light changes (cover/uncover test)
- [ ] External LED automatically controlling based on light
- [ ] Web dashboard accessible and updating
- [ ] Calibration button working (manual calibration test)
- [ ] System stable and ready for deployment

**🎉 Your LDR Light Sensor Analytics system is fully operational!**

### 🌟 **Key Features Summary:**
- **Real-time OLED display** with live light readings
- **Professional web dashboard** with charts and analytics  
- **Automatic LED control** based on ambient light
- **Audio alerts** for low light conditions
- **One-button calibration** for easy setup
- **Data export capabilities** for analysis
- **Mobile-responsive interface** for remote monitoring
