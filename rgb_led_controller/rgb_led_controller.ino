#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WiFi credentials - CHANGE THESE
const char* ssid = "Sukrit_wifi";
const char* password = "abcde123";

// RGB LED Pin Definitions (4-pin RGB LED)
#define RED_PIN 25      // Red channel
#define GREEN_PIN 26    // Green channel  
#define BLUE_PIN 27     // Blue channel
#define COMMON_PIN 14   // Common cathode/anode (if needed)

// PWM Configuration
#define PWM_FREQ 5000    // 5 KHz frequency
#define PWM_RESOLUTION 8 // 8-bit resolution (0-255)
#define RED_CHANNEL 0
#define GREEN_CHANNEL 1
#define BLUE_CHANNEL 2

// Web Server
WebServer server(80);

// RGB LED State
struct RGBState {
  int red;
  int green;
  int blue;
  int brightness;
  bool power;
  String mode;
  int animation_speed;
  String current_color_name;
  unsigned long last_update;
};

// Global Variables
RGBState currentState;
bool rainbow_mode = false;
bool breathing_mode = false;
bool strobe_mode = false;
int animation_step = 0;
unsigned long last_animation = 0;

// Predefined colors
struct ColorPreset {
  String name;
  int red, green, blue;
};

ColorPreset presets[] = {
  {"Red", 255, 0, 0},
  {"Green", 0, 255, 0},
  {"Blue", 0, 0, 255},
  {"Yellow", 255, 255, 0},
  {"Cyan", 0, 255, 255},
  {"Magenta", 255, 0, 255},
  {"White", 255, 255, 255},
  {"Orange", 255, 165, 0},
  {"Purple", 128, 0, 128},
  {"Pink", 255, 192, 203},
  {"Lime", 50, 205, 50},
  {"Violet", 238, 130, 238}
};

// Function Declarations
void handleRoot();
void handleSetColor();
void handleSetBrightness();
void handleSetMode();
void handleGetState();
void handlePresetColor();
void handleTogglePower();
void updateLED();
void rainbowAnimation();
void breathingAnimation();
void strobeAnimation();
void setupPWM();
String getCurrentColorName();

void setup() {
  Serial.begin(115200);
  
  // Initialize RGB LED pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(COMMON_PIN, OUTPUT);
  
  // Setup PWM channels
  setupPWM();
  
  // Initialize LED state
  currentState.red = 0;
  currentState.green = 0;
  currentState.blue = 0;
  currentState.brightness = 100;
  currentState.power = true;
  currentState.mode = "manual";
  currentState.animation_speed = 50;
  currentState.current_color_name = "Off";
  currentState.last_update = millis();
  
  // Set common pin (for common cathode, set HIGH; for common anode, set LOW)
  digitalWrite(COMMON_PIN, LOW); // Adjust based on your LED type
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("RGB LED Controller");
  Serial.println("Connecting to WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("RGB LED Controller: http://");
  Serial.println(WiFi.localIP());
  
  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/set-color", HTTP_POST, handleSetColor);
  server.on("/api/set-brightness", HTTP_POST, handleSetBrightness);
  server.on("/api/set-mode", HTTP_POST, handleSetMode);
  server.on("/api/get-state", HTTP_GET, handleGetState);
  server.on("/api/preset-color", HTTP_POST, handlePresetColor);
  server.on("/api/toggle-power", HTTP_POST, handleTogglePower);
  
  server.enableCORS(true);
  server.begin();
  
  Serial.println("RGB LED Web Server Started!");
  Serial.println("==============================");
  
  // Startup animation
  for(int i = 0; i <= 255; i += 5) {
    ledcWrite(RED_CHANNEL, i);
    delay(10);
  }
  for(int i = 255; i >= 0; i -= 5) {
    ledcWrite(RED_CHANNEL, i);
    delay(10);
  }
  
  for(int i = 0; i <= 255; i += 5) {
    ledcWrite(GREEN_CHANNEL, i);
    delay(10);
  }
  for(int i = 255; i >= 0; i -= 5) {
    ledcWrite(GREEN_CHANNEL, i);
    delay(10);
  }
  
  for(int i = 0; i <= 255; i += 5) {
    ledcWrite(BLUE_CHANNEL, i);
    delay(10);
  }
  for(int i = 255; i >= 0; i -= 5) {
    ledcWrite(BLUE_CHANNEL, i);
    delay(10);
  }
  
  Serial.println("RGB LED Ready - Access web interface for control");
  updateLED();
}

void loop() {
  server.handleClient();
  
  // Handle animations
  if (currentState.power) {
    if (currentState.mode == "rainbow") {
      rainbowAnimation();
    } else if (currentState.mode == "breathing") {
      breathingAnimation();
    } else if (currentState.mode == "strobe") {
      strobeAnimation();
    }
  }
  
  // Print status every 30 seconds
  static unsigned long lastStatus = 0;
  if (millis() - lastStatus > 30000) {
    lastStatus = millis();
    Serial.printf("RGB: R=%d G=%d B=%d | Mode: %s | Power: %s\n", 
                  currentState.red, currentState.green, currentState.blue,
                  currentState.mode.c_str(), currentState.power ? "ON" : "OFF");
    Serial.printf("Web Interface: http://%s\n", WiFi.localIP().toString().c_str());
  }
  
  delay(10);
}

void setupPWM() {
  // Configure PWM channels
  ledcSetup(RED_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(GREEN_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(BLUE_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  
  // Attach pins to channels
  ledcAttachPin(RED_PIN, RED_CHANNEL);
  ledcAttachPin(GREEN_PIN, GREEN_CHANNEL);
  ledcAttachPin(BLUE_PIN, BLUE_CHANNEL);
}

void updateLED() {
  if (currentState.power && currentState.mode == "manual") {
    // Apply brightness scaling
    int red_val = (currentState.red * currentState.brightness) / 100;
    int green_val = (currentState.green * currentState.brightness) / 100;
    int blue_val = (currentState.blue * currentState.brightness) / 100;
    
    ledcWrite(RED_CHANNEL, red_val);
    ledcWrite(GREEN_CHANNEL, green_val);
    ledcWrite(BLUE_CHANNEL, blue_val);
    
    currentState.current_color_name = getCurrentColorName();
  } else if (!currentState.power) {
    // Turn off LED
    ledcWrite(RED_CHANNEL, 0);
    ledcWrite(GREEN_CHANNEL, 0);
    ledcWrite(BLUE_CHANNEL, 0);
    currentState.current_color_name = "Off";
  }
  
  currentState.last_update = millis();
  
  Serial.printf("LED Updated: R=%d G=%d B=%d (Brightness: %d%%)\n", 
                currentState.red, currentState.green, currentState.blue, currentState.brightness);
}

String getCurrentColorName() {
  // Check if it matches any preset
  for (int i = 0; i < sizeof(presets)/sizeof(presets[0]); i++) {
    if (abs(currentState.red - presets[i].red) < 10 && 
        abs(currentState.green - presets[i].green) < 10 && 
        abs(currentState.blue - presets[i].blue) < 10) {
      return presets[i].name;
    }
  }
  
  // Generate custom color description
  if (currentState.red == 0 && currentState.green == 0 && currentState.blue == 0) {
    return "Off";
  } else {
    return "Custom (" + String(currentState.red) + "," + String(currentState.green) + "," + String(currentState.blue) + ")";
  }
}

void rainbowAnimation() {
  if (millis() - last_animation > (101 - currentState.animation_speed)) {
    last_animation = millis();
    
    float hue = (animation_step % 360) * PI / 180.0;
    
    // Convert HSV to RGB
    float r, g, b;
    int sector = floor(hue / (PI / 3.0));
    float f = (hue / (PI / 3.0)) - sector;
    float p = 0;
    float q = 1 - f;
    float t = f;
    
    switch(sector) {
      case 0: r = 1; g = t; b = p; break;
      case 1: r = q; g = 1; b = p; break;
      case 2: r = p; g = 1; b = t; break;
      case 3: r = p; g = q; b = 1; break;
      case 4: r = t; g = p; b = 1; break;
      case 5: r = 1; g = p; b = q; break;
      default: r = 1; g = 0; b = 0; break;
    }
    
    int red_val = (int)(r * 255 * currentState.brightness / 100);
    int green_val = (int)(g * 255 * currentState.brightness / 100);
    int blue_val = (int)(b * 255 * currentState.brightness / 100);
    
    ledcWrite(RED_CHANNEL, red_val);
    ledcWrite(GREEN_CHANNEL, green_val);
    ledcWrite(BLUE_CHANNEL, blue_val);
    
    animation_step += 2;
    if (animation_step >= 360) animation_step = 0;
  }
}

void breathingAnimation() {
  if (millis() - last_animation > (101 - currentState.animation_speed)) {
    last_animation = millis();
    
    float breath = (sin(animation_step * PI / 180.0) + 1.0) / 2.0; // 0 to 1
    
    int red_val = (currentState.red * breath * currentState.brightness) / 100;
    int green_val = (currentState.green * breath * currentState.brightness) / 100;
    int blue_val = (currentState.blue * breath * currentState.brightness) / 100;
    
    ledcWrite(RED_CHANNEL, red_val);
    ledcWrite(GREEN_CHANNEL, green_val);
    ledcWrite(BLUE_CHANNEL, blue_val);
    
    animation_step += 2;
    if (animation_step >= 360) animation_step = 0;
  }
}

void strobeAnimation() {
  if (millis() - last_animation > (101 - currentState.animation_speed)) {
    last_animation = millis();
    
    if (animation_step % 2 == 0) {
      // On
      int red_val = (currentState.red * currentState.brightness) / 100;
      int green_val = (currentState.green * currentState.brightness) / 100;
      int blue_val = (currentState.blue * currentState.brightness) / 100;
      
      ledcWrite(RED_CHANNEL, red_val);
      ledcWrite(GREEN_CHANNEL, green_val);
      ledcWrite(BLUE_CHANNEL, blue_val);
    } else {
      // Off
      ledcWrite(RED_CHANNEL, 0);
      ledcWrite(GREEN_CHANNEL, 0);
      ledcWrite(BLUE_CHANNEL, 0);
    }
    
    animation_step++;
    if (animation_step >= 20) animation_step = 0;
  }
}

void handleRoot() {
  String html = R"HTMLDOC(
<!DOCTYPE html>
<html>
<head>
    <title>RGB LED Controller</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;500;600;700&display=swap');
        
        * { 
            margin: 0; 
            padding: 0; 
            box-sizing: border-box; 
        }
        
        body {
            font-family: 'Poppins', sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            color: #333;
            overflow-x: hidden;
        }
        
        .header {
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(10px);
            padding: 20px;
            text-align: center;
            box-shadow: 0 4px 20px rgba(0,0,0,0.1);
            margin-bottom: 30px;
        }
        
        .header h1 {
            color: #4a5568;
            font-size: 3rem;
            font-weight: 700;
            margin-bottom: 10px;
            background: linear-gradient(135deg, #667eea, #764ba2);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        
        .led-preview {
            width: 150px;
            height: 150px;
            border-radius: 50%;
            margin: 20px auto;
            background: #000;
            box-shadow: 0 0 50px rgba(0,0,0,0.5);
            transition: all 0.3s ease;
            border: 5px solid #fff;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 30px;
        }
        
        .card {
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            padding: 30px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.1);
            transition: all 0.3s ease;
        }
        
        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 20px 40px rgba(0,0,0,0.15);
        }
        
        .card h2 {
            color: #2d3748;
            margin-bottom: 25px;
            font-size: 1.5rem;
            font-weight: 600;
            text-align: center;
        }
        
        .color-controls {
            grid-column: 1 / -1;
        }
        
        .slider-container {
            margin: 20px 0;
        }
        
        .slider-label {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 10px;
            font-weight: 500;
        }
        
        .slider {
            width: 100%;
            height: 8px;
            border-radius: 5px;
            outline: none;
            transition: all 0.3s ease;
            -webkit-appearance: none;
        }
        
        .slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 25px;
            height: 25px;
            border-radius: 50%;
            background: #fff;
            border: 3px solid #667eea;
            cursor: pointer;
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
            transition: all 0.3s ease;
        }
        
        .slider::-webkit-slider-thumb:hover {
            transform: scale(1.2);
            box-shadow: 0 6px 20px rgba(0,0,0,0.3);
        }
        
        .red-slider { background: linear-gradient(to right, #000, #ff0000); }
        .green-slider { background: linear-gradient(to right, #000, #00ff00); }
        .blue-slider { background: linear-gradient(to right, #000, #0000ff); }
        .brightness-slider { background: linear-gradient(to right, #333, #fff); }
        .speed-slider { background: linear-gradient(to right, #667eea, #764ba2); }
        
        .preset-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(80px, 1fr));
            gap: 15px;
            margin: 20px 0;
        }
        
        .preset-btn {
            width: 100%;
            height: 60px;
            border: none;
            border-radius: 15px;
            cursor: pointer;
            font-size: 0.9rem;
            font-weight: 600;
            color: #fff;
            text-shadow: 0 2px 4px rgba(0,0,0,0.3);
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        }
        
        .preset-btn:hover {
            transform: translateY(-3px);
            box-shadow: 0 8px 25px rgba(0,0,0,0.3);
        }
        
        .control-buttons {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            gap: 15px;
            margin: 25px 0;
        }
        
        .btn {
            padding: 15px 25px;
            border: none;
            border-radius: 25px;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 600;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        }
        
        .btn-primary {
            background: linear-gradient(135deg, #667eea, #764ba2);
            color: white;
        }
        
        .btn-success {
            background: linear-gradient(135deg, #56ab2f, #a8e6cf);
            color: white;
        }
        
        .btn-warning {
            background: linear-gradient(135deg, #f093fb, #f5576c);
            color: white;
        }
        
        .btn-danger {
            background: linear-gradient(135deg, #ff416c, #ff4b2b);
            color: white;
        }
        
        .btn:hover {
            transform: translateY(-3px);
            box-shadow: 0 8px 25px rgba(0,0,0,0.3);
        }
        
        .mode-buttons {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(100px, 1fr));
            gap: 10px;
            margin: 20px 0;
        }
        
        .mode-btn {
            padding: 12px 20px;
            border: 2px solid #667eea;
            border-radius: 20px;
            background: transparent;
            color: #667eea;
            cursor: pointer;
            font-weight: 500;
            transition: all 0.3s ease;
        }
        
        .mode-btn.active {
            background: linear-gradient(135deg, #667eea, #764ba2);
            color: white;
        }
        
        .mode-btn:hover {
            transform: scale(1.05);
        }
        
        .status-display {
            background: linear-gradient(135deg, #f093fb, #f5576c);
            color: white;
            padding: 20px;
            border-radius: 15px;
            text-align: center;
            margin: 20px 0;
            font-weight: 600;
        }
        
        .color-picker-container {
            margin: 20px 0;
            text-align: center;
        }
        
        .color-picker {
            width: 100px;
            height: 50px;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            margin: 0 auto;
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        }
        
        @keyframes glow {
            0%, 100% { box-shadow: 0 0 20px rgba(255,255,255,0.5); }
            50% { box-shadow: 0 0 40px rgba(255,255,255,0.8); }
        }
        
        .led-preview.glowing {
            animation: glow 2s ease-in-out infinite;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>🌈 RGB LED Controller</h1>
        <p>Professional Color Control Interface</p>
        <div id="ledPreview" class="led-preview"></div>
    </div>
    
    <div class="container">
        <!-- Color Controls -->
        <div class="card color-controls">
            <h2>🎨 Color Controls</h2>
            
            <div class="color-picker-container">
                <input type="color" id="colorPicker" class="color-picker" value="#000000">
                <p style="margin-top: 10px;">Quick Color Picker</p>
            </div>
            
            <div class="slider-container">
                <div class="slider-label">
                    <span>🔴 Red</span>
                    <span id="redValue">0</span>
                </div>
                <input type="range" id="redSlider" class="slider red-slider" min="0" max="255" value="0">
            </div>
            
            <div class="slider-container">
                <div class="slider-label">
                    <span>🟢 Green</span>
                    <span id="greenValue">0</span>
                </div>
                <input type="range" id="greenSlider" class="slider green-slider" min="0" max="255" value="0">
            </div>
            
            <div class="slider-container">
                <div class="slider-label">
                    <span>🔵 Blue</span>
                    <span id="blueValue">0</span>
                </div>
                <input type="range" id="blueSlider" class="slider blue-slider" min="0" max="255" value="0">
            </div>
            
            <div class="slider-container">
                <div class="slider-label">
                    <span>💡 Brightness</span>
                    <span id="brightnessValue">100%</span>
                </div>
                <input type="range" id="brightnessSlider" class="slider brightness-slider" min="1" max="100" value="100">
            </div>
        </div>
        
        <!-- Preset Colors -->
        <div class="card">
            <h2>🎭 Color Presets</h2>
            <div class="preset-grid">
                <button class="preset-btn" style="background: #ff0000;" onclick="setPreset('Red')">Red</button>
                <button class="preset-btn" style="background: #00ff00;" onclick="setPreset('Green')">Green</button>
                <button class="preset-btn" style="background: #0000ff;" onclick="setPreset('Blue')">Blue</button>
                <button class="preset-btn" style="background: #ffff00;" onclick="setPreset('Yellow')">Yellow</button>
                <button class="preset-btn" style="background: #00ffff;" onclick="setPreset('Cyan')">Cyan</button>
                <button class="preset-btn" style="background: #ff00ff;" onclick="setPreset('Magenta')">Magenta</button>
                <button class="preset-btn" style="background: #ffffff; color: #333;" onclick="setPreset('White')">White</button>
                <button class="preset-btn" style="background: #ffa500;" onclick="setPreset('Orange')">Orange</button>
                <button class="preset-btn" style="background: #800080;" onclick="setPreset('Purple')">Purple</button>
                <button class="preset-btn" style="background: #ffc0cb;" onclick="setPreset('Pink')">Pink</button>
                <button class="preset-btn" style="background: #32cd32;" onclick="setPreset('Lime')">Lime</button>
                <button class="preset-btn" style="background: #ee82ee;" onclick="setPreset('Violet')">Violet</button>
            </div>
        </div>
        
        <!-- Animation Modes -->
        <div class="card">
            <h2>✨ Animation Modes</h2>
            <div class="mode-buttons">
                <button class="mode-btn active" onclick="setMode('manual')">Manual</button>
                <button class="mode-btn" onclick="setMode('rainbow')">Rainbow</button>
                <button class="mode-btn" onclick="setMode('breathing')">Breathing</button>
                <button class="mode-btn" onclick="setMode('strobe')">Strobe</button>
            </div>
            
            <div class="slider-container">
                <div class="slider-label">
                    <span>⚡ Animation Speed</span>
                    <span id="speedValue">50</span>
                </div>
                <input type="range" id="speedSlider" class="slider speed-slider" min="1" max="100" value="50">
            </div>
        </div>
        
        <!-- System Controls -->
        <div class="card">
            <h2>🎛️ System Controls</h2>
            
            <div class="status-display" id="statusDisplay">
                System Ready
            </div>
            
            <div class="control-buttons">
                <button class="btn btn-primary" onclick="togglePower()">
                    <span id="powerText">💡 Turn Off</span>
                </button>
                <button class="btn btn-success" onclick="randomColor()">🎲 Random</button>
                <button class="btn btn-warning" onclick="saveCurrentColor()">💾 Save</button>
                <button class="btn btn-danger" onclick="resetToDefault()">🔄 Reset</button>
            </div>
        </div>
    </div>
    
    <script>
        let currentState = {
            red: 0,
            green: 0,
            blue: 0,
            brightness: 100,
            power: true,
            mode: 'manual',
            animation_speed: 50
        };
        
        // Get all controls
        const redSlider = document.getElementById('redSlider');
        const greenSlider = document.getElementById('greenSlider');
        const blueSlider = document.getElementById('blueSlider');
        const brightnessSlider = document.getElementById('brightnessSlider');
        const speedSlider = document.getElementById('speedSlider');
        const colorPicker = document.getElementById('colorPicker');
        const ledPreview = document.getElementById('ledPreview');
        
        // Add event listeners
        redSlider.addEventListener('input', updateColor);
        greenSlider.addEventListener('input', updateColor);
        blueSlider.addEventListener('input', updateColor);
        brightnessSlider.addEventListener('input', updateBrightness);
        speedSlider.addEventListener('input', updateSpeed);
        colorPicker.addEventListener('input', updateFromColorPicker);
        
        function updateColor() {
            currentState.red = parseInt(redSlider.value);
            currentState.green = parseInt(greenSlider.value);
            currentState.blue = parseInt(blueSlider.value);
            
            document.getElementById('redValue').textContent = currentState.red;
            document.getElementById('greenValue').textContent = currentState.green;
            document.getElementById('blueValue').textContent = currentState.blue;
            
            updatePreview();
            sendColorUpdate();
        }
        
        function updateFromColorPicker() {
            const hex = colorPicker.value;
            const r = parseInt(hex.slice(1, 3), 16);
            const g = parseInt(hex.slice(3, 5), 16);
            const b = parseInt(hex.slice(5, 7), 16);
            
            redSlider.value = r;
            greenSlider.value = g;
            blueSlider.value = b;
            
            updateColor();
        }
        
        function updateBrightness() {
            currentState.brightness = parseInt(brightnessSlider.value);
            document.getElementById('brightnessValue').textContent = currentState.brightness + '%';
            
            updatePreview();
            sendBrightnessUpdate();
        }
        
        function updateSpeed() {
            currentState.animation_speed = parseInt(speedSlider.value);
            document.getElementById('speedValue').textContent = currentState.animation_speed;
            
            sendSpeedUpdate();
        }
        
        function updatePreview() {
            if (currentState.power) {
                const r = Math.round(currentState.red * currentState.brightness / 100);
                const g = Math.round(currentState.green * currentState.brightness / 100);
                const b = Math.round(currentState.blue * currentState.brightness / 100);
                
                ledPreview.style.background = `rgb(${r}, ${g}, ${b})`;
                ledPreview.style.boxShadow = `0 0 50px rgba(${r}, ${g}, ${b}, 0.8)`;
                
                if (currentState.mode !== 'manual') {
                    ledPreview.classList.add('glowing');
                } else {
                    ledPreview.classList.remove('glowing');
                }
            } else {
                ledPreview.style.background = '#000';
                ledPreview.style.boxShadow = '0 0 50px rgba(0,0,0,0.5)';
                ledPreview.classList.remove('glowing');
            }
        }
        
        async function sendColorUpdate() {
            try {
                await fetch('/api/set-color', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({
                        red: currentState.red,
                        green: currentState.green,
                        blue: currentState.blue
                    })
                });
            } catch (error) {
                console.error('Error updating color:', error);
            }
        }
        
        async function sendBrightnessUpdate() {
            try {
                await fetch('/api/set-brightness', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ brightness: currentState.brightness })
                });
            } catch (error) {
                console.error('Error updating brightness:', error);
            }
        }
        
        async function sendSpeedUpdate() {
            try {
                await fetch('/api/set-mode', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ 
                        mode: currentState.mode,
                        speed: currentState.animation_speed 
                    })
                });
            } catch (error) {
                console.error('Error updating speed:', error);
            }
        }
        
        async function setPreset(colorName) {
            try {
                const response = await fetch('/api/preset-color', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ preset: colorName })
                });
                
                if (response.ok) {
                    getState();
                    document.getElementById('statusDisplay').textContent = `Color set to ${colorName}`;
                }
            } catch (error) {
                console.error('Error setting preset:', error);
            }
        }
        
        async function setMode(mode) {
            currentState.mode = mode;
            
            // Update button states
            document.querySelectorAll('.mode-btn').forEach(btn => {
                btn.classList.remove('active');
            });
            event.target.classList.add('active');
            
            try {
                await fetch('/api/set-mode', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ 
                        mode: mode,
                        speed: currentState.animation_speed 
                    })
                });
                
                updatePreview();
                document.getElementById('statusDisplay').textContent = `Mode: ${mode.charAt(0).toUpperCase() + mode.slice(1)}`;
            } catch (error) {
                console.error('Error setting mode:', error);
            }
        }
        
        async function togglePower() {
            try {
                const response = await fetch('/api/toggle-power', { method: 'POST' });
                if (response.ok) {
                    currentState.power = !currentState.power;
                    document.getElementById('powerText').textContent = currentState.power ? '💡 Turn Off' : '💡 Turn On';
                    updatePreview();
                    document.getElementById('statusDisplay').textContent = currentState.power ? 'LED Powered On' : 'LED Powered Off';
                }
            } catch (error) {
                console.error('Error toggling power:', error);
            }
        }
        
        function randomColor() {
            const r = Math.floor(Math.random() * 256);
            const g = Math.floor(Math.random() * 256);
            const b = Math.floor(Math.random() * 256);
            
            redSlider.value = r;
            greenSlider.value = g;
            blueSlider.value = b;
            
            updateColor();
            document.getElementById('statusDisplay').textContent = `Random color: RGB(${r}, ${g}, ${b})`;
        }
        
        function saveCurrentColor() {
            const colorData = {
                r: currentState.red,
                g: currentState.green,
                b: currentState.blue,
                brightness: currentState.brightness
            };
            localStorage.setItem('savedColor', JSON.stringify(colorData));
            document.getElementById('statusDisplay').textContent = 'Color saved to browser memory';
        }
        
        function resetToDefault() {
            redSlider.value = 0;
            greenSlider.value = 0;
            blueSlider.value = 0;
            brightnessSlider.value = 100;
            
            updateColor();
            updateBrightness();
            document.getElementById('statusDisplay').textContent = 'Reset to default values';
        }
        
        async function getState() {
            try {
                const response = await fetch('/api/get-state');
                const state = await response.json();
                
                currentState = state;
                
                redSlider.value = state.red;
                greenSlider.value = state.green;
                blueSlider.value = state.blue;
                brightnessSlider.value = state.brightness;
                speedSlider.value = state.animation_speed;
                
                document.getElementById('redValue').textContent = state.red;
                document.getElementById('greenValue').textContent = state.green;
                document.getElementById('blueValue').textContent = state.blue;
                document.getElementById('brightnessValue').textContent = state.brightness + '%';
                document.getElementById('speedValue').textContent = state.animation_speed;
                document.getElementById('powerText').textContent = state.power ? '💡 Turn Off' : '💡 Turn On';
                
                updatePreview();
            } catch (error) {
                console.error('Error getting state:', error);
            }
        }
        
        // Initialize
        window.onload = function() {
            getState();
            
            // Check for saved color
            const savedColor = localStorage.getItem('savedColor');
            if (savedColor) {
                const color = JSON.parse(savedColor);
                console.log('Saved color found:', color);
            }
            
            // Update state every 2 seconds
            setInterval(getState, 2000);
        };
    </script>
</body>
</html>
)HTMLDOC";
  
  server.send(200, "text/html", html);
}

void handleSetColor() {
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(200);
    deserializeJson(doc, server.arg("plain"));
    
    currentState.red = doc["red"];
    currentState.green = doc["green"];
    currentState.blue = doc["blue"];
    currentState.mode = "manual";
    
    updateLED();
    server.send(200, "text/plain", "Color updated");
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
}

void handleSetBrightness() {
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(100);
    deserializeJson(doc, server.arg("plain"));
    
    currentState.brightness = doc["brightness"];
    
    updateLED();
    server.send(200, "text/plain", "Brightness updated");
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
}

void handleSetMode() {
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(200);
    deserializeJson(doc, server.arg("plain"));
    
    currentState.mode = doc["mode"].as<String>();
    if (doc.containsKey("speed")) {
      currentState.animation_speed = doc["speed"];
    }
    
    animation_step = 0; // Reset animation
    
    if (currentState.mode == "manual") {
      updateLED();
    }
    
    server.send(200, "text/plain", "Mode updated");
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
}

void handleGetState() {
  StaticJsonDocument<300> doc;
  
  doc["red"] = currentState.red;
  doc["green"] = currentState.green;
  doc["blue"] = currentState.blue;
  doc["brightness"] = currentState.brightness;
  doc["power"] = currentState.power;
  doc["mode"] = currentState.mode;
  doc["animation_speed"] = currentState.animation_speed;
  doc["current_color_name"] = currentState.current_color_name;
  doc["last_update"] = currentState.last_update;
  
  String jsonString;
  serializeJson(doc, jsonString);
  server.send(200, "application/json", jsonString);
}

void handlePresetColor() {
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(100);
    deserializeJson(doc, server.arg("plain"));
    
    String preset = doc["preset"];
    
    // Find the preset color
    for (int i = 0; i < sizeof(presets)/sizeof(presets[0]); i++) {
      if (presets[i].name == preset) {
        currentState.red = presets[i].red;
        currentState.green = presets[i].green;
        currentState.blue = presets[i].blue;
        currentState.mode = "manual";
        
        updateLED();
        server.send(200, "text/plain", "Preset color set");
        return;
      }
    }
    
    server.send(404, "text/plain", "Preset not found");
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
}

void handleTogglePower() {
  currentState.power = !currentState.power;
  updateLED();
  
  String response = currentState.power ? "LED turned on" : "LED turned off";
  server.send(200, "text/plain", response);
}
