# 🌈 RGB LED Controller

Professional RGB LED color control system with real-time web interface, color mixing, animation modes, and advanced color management for ESP32.

## 📋 Components Required

### Core Components
- **ESP32-DOWD-V3 Development Board** (1x)
- **4-Pin RGB LED (Common Cathode or Common Anode)** (1x)
- **220Ω Resistors** (3x) - One for each color channel
- **Breadboard & Jumper Wires**

### Optional Components
- **External 5V power supply** for brighter LED operation
- **Heat sink** for high-power RGB LEDs
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

### 🌈 4-Pin RGB LED Pinout
```
Standard RGB LED (Common Cathode):
     ┌─────────┐
     │    R    │ ← Red Anode
     │  ┌───┐  │
     │  │ █ │  │ ← LED Die
     │  └───┘  │
     │ G  │  B │ ← Green & Blue Anodes
     │    C    │ ← Common Cathode (-)
     └─────────┘

Pin identification:
- Longest pin = Common (Cathode or Anode)
- Next to longest = Red
- Shortest = Green  
- Remaining = Blue
```

### 🔌 Detailed Wiring Connections

#### Common Cathode RGB LED (Recommended)
```
RGB LED Pin → Resistor → ESP32 Pin → Notes
Red Anode   → 220Ω    → D25 (GPIO25) → PWM capable pin
Green Anode → 220Ω    → D26 (GPIO26) → PWM capable pin  
Blue Anode  → 220Ω    → D27 (GPIO27) → PWM capable pin
Common Pin  → None    → GND          → Direct to ground
```

#### Common Anode RGB LED (Alternative)
```
RGB LED Pin → Resistor → ESP32 Pin → Notes
Red Cathode → 220Ω    → D25 (GPIO25) → PWM capable pin
Green Cathode→220Ω    → D26 (GPIO26) → PWM capable pin
Blue Cathode→ 220Ω    → D27 (GPIO27) → PWM capable pin
Common Pin  → None    → 3V3          → Direct to 3.3V
```

**⚠️ Important:** The code is configured for Common Cathode by default. For Common Anode, change line in code:
```cpp
digitalWrite(COMMON_PIN, HIGH); // Change LOW to HIGH for Common Anode
```

### 🔧 Circuit Diagram
```
ESP32 GPIO25 ──┬── 220Ω ──── Red Anode
               │
ESP32 GPIO26 ──┼── 220Ω ──── Green Anode
               │                    │
ESP32 GPIO27 ──┼── 220Ω ──── Blue Anode ─── RGB LED
               │                    │
ESP32 GND ─────┴─────────────── Common Cathode
```

## 📦 Required Libraries

Install these libraries through Arduino IDE Library Manager:

```
Tools → Manage Libraries → Search and Install:
```

1. **WiFi** (Built-in with ESP32)
2. **WebServer** (Built-in with ESP32)
3. **ArduinoJson** by Benoit Blanchon

### Library Installation Steps:
1. **Open Arduino IDE**
2. **Go to Tools → Manage Libraries**
3. **Search for "ArduinoJson"** → Install latest version

## ⚙️ Setup Instructions

### 1. Hardware Assembly

#### Step 1: Identify RGB LED Type
1. **Find the longest pin** on your RGB LED - this is the Common pin
2. **Determine type:**
   - **Common Cathode:** Connect Common to GND (most common)
   - **Common Anode:** Connect Common to 3V3 (less common)
3. **Identify color pins** using multimeter or datasheet

#### Step 2: Wiring Assembly
```
Assembly Order:
1. Place RGB LED on breadboard (ensure pins don't touch)
2. Connect three 220Ω resistors to Red, Green, Blue pins
3. Connect resistor other ends to ESP32 GPIO25, 26, 27
4. Connect Common pin to GND (cathode) or 3V3 (anode)
5. Double-check all connections before powering on
```

#### Step 3: Test Connections
```
Power-on Test:
1. Upload code to ESP32
2. RGB LED should cycle through Red → Green → Blue on startup
3. If wrong colors appear, swap GPIO connections
4. If LED doesn't light, check Common pin connection
```

### 2. Software Configuration

#### Arduino IDE Setup:
1. **Install ESP32 Board Support:**
   - File → Preferences
   - Add URL: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Tools → Board → Boards Manager → Install "ESP32"

2. **Board Configuration:**
   ```
   Tools → Board → ESP32 Arduino → "ESP32 Dev Module"
   Tools → Port → Select your ESP32 port
   Tools → Upload Speed → 921600
   Tools → Flash Frequency → 80MHz
   ```

#### Code Configuration:
1. **Update WiFi Credentials:**
   ```cpp
   const char* ssid = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

2. **Configure for Common Anode (if needed):**
   ```cpp
   digitalWrite(COMMON_PIN, HIGH); // Change from LOW to HIGH
   ```

3. **Adjust PWM Settings (optional):**
   ```cpp
   #define PWM_FREQ 5000        // PWM frequency (Hz)
   #define PWM_RESOLUTION 8     // 8-bit resolution (0-255)
   ```

### 3. Upload and Testing

#### Upload Process:
1. **Verify Code:** Click "Verify" to check for compilation errors
2. **Upload Code:** Click "Upload" (or Ctrl+U)
3. **Monitor Output:** Tools → Serial Monitor (115200 baud)

#### System Testing:
1. **Startup Animation:** RGB LED cycles through Red → Green → Blue
2. **WiFi Connection:** Serial monitor shows IP address
3. **Web Interface:** Access dashboard in browser
4. **Color Control:** Test sliders change LED colors in real-time

## 🌐 Web Dashboard Access

### Finding ESP32 IP Address:
1. **Serial Monitor Method:**
   - Tools → Serial Monitor (115200 baud)
   - Look for "IP address: 192.168.x.x"

2. **Router Admin Method:**
   - Access router settings (usually 192.168.1.1)
   - Find "ESP32" in connected devices

### Accessing the Dashboard:
1. **Open Web Browser** (Chrome, Firefox, Safari, Edge)
2. **Navigate to:** `http://[ESP32_IP_ADDRESS]`
3. **Bookmark** for easy access

### 🎨 Dashboard Features:

#### Color Controls
- **🎨 Individual RGB Sliders:** Real-time color mixing (0-255 each)
- **💡 Brightness Control:** Global brightness adjustment (1-100%)
- **🎯 Color Picker:** Click-and-select color interface
- **🌈 Live Preview:** Visual LED simulation on screen

#### Color Presets
- **12 Preset Colors:** Red, Green, Blue, Yellow, Cyan, Magenta, White, Orange, Purple, Pink, Lime, Violet
- **One-Click Application:** Instant color changes
- **Visual Buttons:** Each button shows actual color

#### Animation Modes
- **Manual Mode:** Direct color control via sliders
- **Rainbow Mode:** Smooth color spectrum cycling
- **Breathing Mode:** Gentle fade in/out effect
- **Strobe Mode:** Fast on/off flashing
- **Speed Control:** Adjustable animation speed (1-100)

#### System Controls
- **Power Toggle:** Turn RGB LED on/off
- **Random Color:** Generate random color combinations
- **Save/Load:** Browser-based color memory
- **Reset:** Return to default settings

## 🔧 Troubleshooting Guide

### ❌ RGB LED Not Lighting Up

**Symptoms:** LED completely dark, no response to controls

**Solutions:**
1. **Check Power Connections:**
   ```
   Verify: ESP32 is powered and running (built-in LED should be on)
   Verify: USB cable provides sufficient power
   Check: All breadboard connections are firm
   ```

2. **Verify LED Type:**
   ```
   Common Cathode: Common pin → GND
   Common Anode: Common pin → 3V3
   
   Code setting: digitalWrite(COMMON_PIN, LOW or HIGH)
   ```

3. **Test Individual Colors:**
   ```
   Set Red = 255, Green = 0, Blue = 0 → Should show red
   Set Red = 0, Green = 255, Blue = 0 → Should show green  
   Set Red = 0, Green = 0, Blue = 255 → Should show blue
   ```

4. **Check Resistors:**
   ```
   Measure: Each 220Ω resistor should read ~220 ohms
   Position: Between ESP32 GPIO and LED pin (not on Common)
   ```

### ❌ Wrong Colors Displayed

**Symptoms:** Setting red shows blue, or colors are mixed up

**Solutions:**
1. **Pin Mapping Issue:**
   ```cpp
   // Try swapping pin assignments in code:
   #define RED_PIN 27      // Was 25
   #define GREEN_PIN 25    // Was 26  
   #define BLUE_PIN 26     // Was 27
   ```

2. **LED Pinout Confusion:**
   - Check RGB LED datasheet for correct pin identification
   - Use multimeter to identify which pin controls which color
   - Some LEDs have non-standard pin arrangements

3. **Common Type Mismatch:**
   ```cpp
   // For Common Anode, invert the logic:
   int red_val = 255 - (currentState.red * currentState.brightness) / 100;
   ```

### ❌ LED Too Dim or Too Bright

**Symptoms:** LED barely visible or uncomfortably bright

**Solutions:**
1. **Adjust Resistor Values:**
   ```
   Too Dim: Use smaller resistors (150Ω, 100Ω)
   Too Bright: Use larger resistors (330Ω, 470Ω)
   Different colors may need different resistor values
   ```

2. **Power Supply Issues:**
   ```
   Dim LED: Check if ESP32 3V3 regulator can provide enough current
   Solution: Use external 5V supply for LED (with appropriate resistors)
   ```

3. **PWM Settings:**
   ```cpp
   // Adjust PWM resolution for finer control:
   #define PWM_RESOLUTION 10    // 10-bit (0-1023) for finer control
   #define PWM_RESOLUTION 8     // 8-bit (0-255) for standard control
   ```

### ❌ Colors Not Mixing Properly

**Symptoms:** Unable to create white, strange color combinations

**Solutions:**
1. **LED Quality Issues:**
   - Cheap RGB LEDs may have unbalanced color intensities
   - Try different resistor values per color (red: 330Ω, green: 220Ω, blue: 150Ω)

2. **Calibration Needed:**
   ```cpp
   // Add color balance factors:
   int red_val = (currentState.red * 0.8 * currentState.brightness) / 100;   // Reduce red
   int green_val = (currentState.green * 1.0 * currentState.brightness) / 100; // Normal green
   int blue_val = (currentState.blue * 1.2 * currentState.brightness) / 100;   // Boost blue
   ```

### ❌ Web Dashboard Not Responding

**Symptoms:** Sliders don't change LED colors, dashboard unresponsive

**Solutions:**
1. **Network Issues:**
   - Ensure ESP32 and device are on same WiFi network
   - Check WiFi signal strength (move closer to router)
   - Restart router if connection is unstable

2. **Browser Issues:**
   - Try different browser (Chrome recommended)
   - Clear browser cache and cookies
   - Disable ad blockers that might block local connections

3. **ESP32 Performance:**
   ```
   Check serial monitor for errors:
   - "Watchdog timeout" → Code stuck in loop
   - "WiFi disconnected" → Network issues
   - "Memory low" → Restart ESP32
   ```

### ❌ Animation Modes Not Working

**Symptoms:** Rainbow, breathing, or strobe modes don't animate

**Solutions:**
1. **Mode Selection:**
   - Ensure correct mode is selected in web interface
   - Check that mode buttons show "active" state
   - Try switching to manual mode and back

2. **Animation Speed:**
   - Increase animation speed slider
   - Check if speed is set to minimum (too slow to see)

3. **Code Issues:**
   ```cpp
   // Verify animation functions are being called:
   if (currentState.mode == "rainbow") {
       rainbowAnimation();  // This should be called
   }
   ```

## 📊 Technical Specifications

### Hardware Specifications
- **PWM Frequency:** 5kHz (adjustable)
- **PWM Resolution:** 8-bit (0-255 values per color)
- **Color Combinations:** 16,777,216 possible colors (256³)
- **Response Time:** <50ms for color changes
- **Power Consumption:** ~100mA per color at full brightness

### Software Specifications
- **Update Rate:** Real-time color changes
- **Web Interface:** HTTP server on port 80
- **Animation Modes:** 4 different modes with speed control
- **Color Presets:** 12 predefined colors
- **Memory Usage:** ~40% Flash, ~25% RAM

### Color Specifications
- **Red Wavelength:** ~620-750nm
- **Green Wavelength:** ~495-570nm  
- **Blue Wavelength:** ~450-495nm
- **White Point:** Balanced RGB (255,255,255)
- **Color Temperature:** Varies with RGB balance

## 🎨 Color Theory & Applications

### Understanding RGB Color Mixing
```
Primary Colors:
Red (255,0,0) + Green (0,255,0) = Yellow (255,255,0)
Red (255,0,0) + Blue (0,0,255) = Magenta (255,0,255)
Green (0,255,0) + Blue (0,0,255) = Cyan (0,255,255)

All Colors:
Red + Green + Blue = White (255,255,255)
No Colors = Black (0,0,0)
```

### Color Temperature Guide
```
Warm Colors (2700K-3000K):
- Red-heavy: (255,147,41) Orange
- Yellow: (255,255,0) 

Cool Colors (5000K-6500K):
- Blue-heavy: (173,216,230) Light Blue
- White: (255,255,255)

Special Effects:
- Purple: (128,0,128)
- Pink: (255,192,203)
- Lime: (50,205,50)
```

### Application Ideas
- **Mood Lighting:** Warm colors for relaxation, cool for focus
- **Party Mode:** Rainbow and strobe animations
- **Reading Light:** Warm white (255,200,150)
- **Sleep Mode:** Very dim red (10,0,0)
- **Alert System:** Flashing red for alarms
- **Status Indicators:** Green=OK, Yellow=Warning, Red=Error

## 🔮 Advanced Features & Customization

### Adding More RGB LEDs
```cpp
// Connect multiple LEDs in parallel to same GPIO pins
// Or use different GPIO pins for independent control:

#define LED1_RED_PIN 25
#define LED1_GREEN_PIN 26  
#define LED1_BLUE_PIN 27

#define LED2_RED_PIN 32
#define LED2_GREEN_PIN 33
#define LED2_BLUE_PIN 14
```

### Custom Animation Development
```cpp
void customAnimation() {
  static int step = 0;
  static unsigned long lastUpdate = 0;
  
  if (millis() - lastUpdate > animationSpeed) {
    // Your custom animation logic here
    // Example: Sine wave color cycling
    
    int red = (sin(step * 0.1) + 1) * 127;
    int green = (sin(step * 0.1 + 2) + 1) * 127;
    int blue = (sin(step * 0.1 + 4) + 1) * 127;
    
    ledcWrite(RED_CHANNEL, red);
    ledcWrite(GREEN_CHANNEL, green);
    ledcWrite(BLUE_CHANNEL, blue);
    
    step++;
    lastUpdate = millis();
  }
}
```

### Integration with Home Automation
```cpp
// MQTT Integration Example
void publishColorState() {
  StaticJsonDocument<200> doc;
  doc["red"] = currentState.red;
  doc["green"] = currentState.green;
  doc["blue"] = currentState.blue;
  doc["brightness"] = currentState.brightness;
  
  String payload;
  serializeJson(doc, payload);
  mqttClient.publish("home/rgb_led/state", payload);
}
```

## 📈 Performance Optimization

### Power Efficiency
- **Lower PWM Frequency:** Reduces power consumption slightly
- **Brightness Limiting:** Set maximum brightness to save power
- **Sleep Mode:** Turn off LED when not needed
- **Efficient Animations:** Optimize animation loops

### Response Time Optimization
- **Faster WiFi:** Use 5GHz network if possible (need newer ESP32)
- **Reduce Web Updates:** Lower dashboard refresh rate
- **Optimize Code:** Remove unnecessary delays in main loop

### Color Accuracy
- **LED Binning:** Use high-quality, color-matched LEDs
- **Resistor Precision:** Use 1% tolerance resistors
- **Power Supply:** Stable voltage for consistent colors
- **Temperature Compensation:** Account for LED temperature drift

---

## 🎯 Quick Start Checklist

- [ ] ESP32-DOWD-V3 development board acquired
- [ ] 4-pin RGB LED (Common Cathode recommended)
- [ ] Three 220Ω resistors for current limiting
- [ ] Breadboard and jumper wires ready
- [ ] RGB LED type identified (Common Cathode/Anode)
- [ ] Wiring completed according to diagram
- [ ] Arduino IDE installed with ESP32 support
- [ ] ArduinoJson library installed
- [ ] WiFi credentials updated in code
- [ ] Code uploaded successfully
- [ ] Startup animation visible (Red→Green→Blue cycle)
- [ ] Serial monitor shows WiFi IP address
- [ ] Web dashboard accessible in browser
- [ ] Color sliders change LED colors in real-time
- [ ] All preset colors working correctly
- [ ] Animation modes functional
- [ ] Brightness control working
- [ ] System stable and ready for use

**🎉 Your RGB LED Controller is ready for professional color control!**

### 🌟 **Key Features Summary:**
- **Real-time Color Control:** Individual RGB sliders with live preview
- **Professional Web Interface:** Modern, responsive design
- **12 Color Presets:** One-click color selection
- **4 Animation Modes:** Rainbow, breathing, strobe, and manual
- **Brightness Control:** Global intensity adjustment
- **Mobile Friendly:** Works perfectly on smartphones
- **Save/Load Colors:** Browser-based color memory
- **Professional Features:** Speed control, random colors, power toggle
