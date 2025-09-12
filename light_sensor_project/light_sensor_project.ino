#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WiFi credentials - CHANGE THESE
const char* ssid = "Sukrit_wifi";
const char* password = "abcde123";

// Pin Definitions for ESP32-DOWD-V3
#define LDR_PIN 35          // Photosensitive resistor (analog input) - ADC1_CH7
#define LED_PIN 2           // Built-in LED
#define EXTERNAL_LED_PIN 19 // External LED for visual feedback
#define BUZZER_PIN 18       // Buzzer for audio alerts
#define BUTTON_PIN 23       // Button for manual calibration

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Web Server
WebServer server(80);

// Light Sensor Data Structure
struct LightData {
  int raw_value;          // Raw ADC reading (0-4095)
  float voltage;          // Converted voltage
  float resistance;       // Calculated LDR resistance
  int light_percentage;   // Light level as percentage
  String light_condition; // Description (Bright, Normal, Dim, Dark)
  unsigned long timestamp;
};

// Light Analytics
struct LightAnalytics {
  int min_reading;
  int max_reading;
  float average_reading;
  int readings_count;
  int bright_events;      // Light level > 80%
  int normal_events;      // Light level 20-80%
  int dim_events;         // Light level 5-20%
  int dark_events;        // Light level < 5%
  unsigned long session_start;
  bool auto_led_mode;     // Automatic LED control based on light
  int alert_threshold;    // Alert when light drops below this
};

// Global Variables
LightData currentLight;
LightAnalytics analytics;
String lightLog = "";
LightData readings[100]; // Store last 100 readings for analysis
int reading_index = 0;

// Calibration values (will be updated during runtime)
int calibration_dark = 4095;   // Reading in complete darkness
int calibration_bright = 0;    // Reading in bright light

// Function Declarations
void handleRoot();
void handleLightData();
void handleAnalytics();
void handleCalibration();
void handleDownloadData();
void handleClearData();
void readLightSensor();
void updateOLED();
void updateAnalytics();
void checkAlerts();
void logLightReading();
void calibrateSensor();

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(EXTERNAL_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize outputs
  digitalWrite(LED_PIN, LOW);
  digitalWrite(EXTERNAL_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize OLED Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    // Continue without OLED if it fails
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("LDR Light Sensor");
    display.println("==================");
    display.println("Initializing...");
    display.display();
    delay(2000);
  }
  
  // Initialize analytics
  analytics.min_reading = 4095;
  analytics.max_reading = 0;
  analytics.average_reading = 0;
  analytics.readings_count = 0;
  analytics.bright_events = 0;
  analytics.normal_events = 0;
  analytics.dim_events = 0;
  analytics.dark_events = 0;
  analytics.session_start = millis();
  analytics.auto_led_mode = true;
  analytics.alert_threshold = 10; // Alert when light < 10%
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Photosensitive Resistor (LDR) Light Sensor Project");
  Serial.println("Connecting to WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Light Sensor Dashboard: http://");
  Serial.println(WiFi.localIP());
  
  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/light-data", HTTP_GET, handleLightData);
  server.on("/api/analytics", HTTP_GET, handleAnalytics);
  server.on("/api/calibration", HTTP_POST, handleCalibration);
  server.on("/api/download", HTTP_GET, handleDownloadData);
  server.on("/api/clear", HTTP_POST, handleClearData);
  
  server.enableCORS(true);
  server.begin();
  
  Serial.println("Light Sensor Web Server Started!");
  Serial.println("=================================");
  
  // Initialize data log
  lightLog = "Timestamp,Raw_Value,Voltage,Resistance,Light_Percentage,Condition\n";
  
  // Startup sequence
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(EXTERNAL_LED_PIN, HIGH);
    tone(BUZZER_PIN, 1000, 200);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(EXTERNAL_LED_PIN, LOW);
    delay(200);
  }
  
  Serial.println("LDR Sensor Ready - Monitoring Light Levels...");
  
  // Initial calibration
  calibrateSensor();
}

void loop() {
  server.handleClient();
  readLightSensor();
  updateOLED();
  updateAnalytics();
  checkAlerts();
  
  // Check calibration button
  static bool last_button_state = HIGH;
  bool current_button_state = digitalRead(BUTTON_PIN);
  
  if (last_button_state == HIGH && current_button_state == LOW) {
    Serial.println("Manual calibration started...");
    calibrateSensor();
    tone(BUZZER_PIN, 1500, 500); // Confirmation beep
  }
  last_button_state = current_button_state;
  
  // Print status every 10 seconds
  static unsigned long lastStatus = 0;
  if (millis() - lastStatus > 10000) {
    lastStatus = millis();
    Serial.printf("Light: %d%% (%s) | Raw: %d | Readings: %d\n", 
                  currentLight.light_percentage,
                  currentLight.light_condition.c_str(),
                  currentLight.raw_value,
                  analytics.readings_count);
    Serial.printf("Dashboard: http://%s\n", WiFi.localIP().toString().c_str());
  }
  
  delay(100); // Read every 100ms for smooth response
}

void readLightSensor() {
  // Read raw ADC value (0-4095 for ESP32)
  currentLight.raw_value = analogRead(LDR_PIN);
  
  // Convert to voltage (ESP32 ADC reference is 3.3V)
  currentLight.voltage = (currentLight.raw_value / 4095.0) * 3.3;
  
  // Calculate LDR resistance (assuming 10kΩ pull-up resistor)
  // Voltage divider: Vout = Vin * R2 / (R1 + R2)
  // Solving for R2 (LDR): R2 = R1 * Vout / (Vin - Vout)
  float pullup_resistance = 10000.0; // 10kΩ
  if (currentLight.voltage < 3.3) {
    currentLight.resistance = pullup_resistance * currentLight.voltage / (3.3 - currentLight.voltage);
  } else {
    currentLight.resistance = 0; // Very bright light
  }
  
  // Calculate light percentage using calibrated values
  if (calibration_dark > calibration_bright) {
    currentLight.light_percentage = map(currentLight.raw_value, 
                                       calibration_bright, calibration_dark, 
                                       100, 0);
  } else {
    currentLight.light_percentage = 50; // Default if calibration failed
  }
  
  // Constrain to 0-100%
  currentLight.light_percentage = constrain(currentLight.light_percentage, 0, 100);
  
  // Determine light condition
  if (currentLight.light_percentage >= 80) {
    currentLight.light_condition = "Bright";
  } else if (currentLight.light_percentage >= 20) {
    currentLight.light_condition = "Normal";
  } else if (currentLight.light_percentage >= 5) {
    currentLight.light_condition = "Dim";
  } else {
    currentLight.light_condition = "Dark";
  }
  
  currentLight.timestamp = millis();
  
  // Store reading for analysis
  if (reading_index < 100) {
    readings[reading_index] = currentLight;
    reading_index++;
  } else {
    // Shift array when full
    for (int i = 0; i < 99; i++) {
      readings[i] = readings[i + 1];
    }
    readings[99] = currentLight;
  }
  
  // Log reading every 5 seconds
  static unsigned long lastLog = 0;
  if (millis() - lastLog > 5000) {
    lastLog = millis();
    logLightReading();
  }
  
  // Auto LED control
  if (analytics.auto_led_mode) {
    if (currentLight.light_percentage < 30) {
      digitalWrite(EXTERNAL_LED_PIN, HIGH); // Turn on LED in low light
    } else {
      digitalWrite(EXTERNAL_LED_PIN, LOW);  // Turn off LED in bright light
    }
  }
}

void updateOLED() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate < 1000) return; // Update every second
  lastUpdate = millis();
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  // Title
  display.setTextSize(1);
  display.println("LDR Light Sensor");
  display.println("================");
  
  // Current readings
  display.printf("Light: %d%% (%s)\n", currentLight.light_percentage, currentLight.light_condition.c_str());
  display.printf("Raw: %d/4095\n", currentLight.raw_value);
  display.printf("Voltage: %.2fV\n", currentLight.voltage);
  display.printf("Resistance: %.0f ohms\n", currentLight.resistance);
  
  // Analytics
  display.printf("Min/Max: %d/%d\n", analytics.min_reading, analytics.max_reading);
  display.printf("Readings: %d\n", analytics.readings_count);
  
  // Status indicators
  if (analytics.auto_led_mode) {
    display.print("Auto LED: ON ");
  } else {
    display.print("Auto LED: OFF ");
  }
  
  if (currentLight.light_percentage < analytics.alert_threshold) {
    display.print("ALERT!");
  }
  
  display.display();
}

void updateAnalytics() {
  analytics.readings_count++;
  
  // Update min/max
  if (currentLight.raw_value < analytics.min_reading) {
    analytics.min_reading = currentLight.raw_value;
  }
  if (currentLight.raw_value > analytics.max_reading) {
    analytics.max_reading = currentLight.raw_value;
  }
  
  // Calculate running average
  static long total_readings = 0;
  total_readings += currentLight.raw_value;
  analytics.average_reading = (float)total_readings / analytics.readings_count;
  
  // Count events by condition
  if (currentLight.light_percentage >= 80) {
    analytics.bright_events++;
  } else if (currentLight.light_percentage >= 20) {
    analytics.normal_events++;
  } else if (currentLight.light_percentage >= 5) {
    analytics.dim_events++;
  } else {
    analytics.dark_events++;
  }
}

void checkAlerts() {
  static bool alert_active = false;
  static unsigned long last_alert = 0;
  
  // Check if light level is below threshold
  if (currentLight.light_percentage < analytics.alert_threshold) {
    if (!alert_active || (millis() - last_alert > 5000)) {
      // Sound alert (don't spam)
      tone(BUZZER_PIN, 800, 200);
      digitalWrite(LED_PIN, HIGH);
      Serial.printf("ALERT: Light level low (%d%%)!\n", currentLight.light_percentage);
      alert_active = true;
      last_alert = millis();
    }
  } else {
    alert_active = false;
    digitalWrite(LED_PIN, LOW);
  }
}

void logLightReading() {
  String logEntry = String(currentLight.timestamp) + "," + 
                   String(currentLight.raw_value) + "," + 
                   String(currentLight.voltage, 2) + "," + 
                   String(currentLight.resistance, 0) + "," + 
                   String(currentLight.light_percentage) + "," + 
                   currentLight.light_condition + "\n";
  
  lightLog += logEntry;
  
  // Keep log size manageable
  int lineCount = 0;
  for (int i = 0; i < lightLog.length(); i++) {
    if (lightLog[i] == '\n') lineCount++;
  }
  
  if (lineCount > 1000) {
    int firstNewline = lightLog.indexOf('\n');
    int secondNewline = lightLog.indexOf('\n', firstNewline + 1);
    lightLog = lightLog.substring(0, firstNewline + 1) + lightLog.substring(secondNewline + 1);
  }
}

void calibrateSensor() {
  Serial.println("Starting calibration...");
  Serial.println("Please ensure normal lighting conditions");
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("CALIBRATING...");
  display.println("Please wait...");
  display.display();
  
  delay(2000);
  
  // Take multiple readings for calibration
  int readings_sum = 0;
  int num_readings = 20;
  
  for (int i = 0; i < num_readings; i++) {
    readings_sum += analogRead(LDR_PIN);
    delay(100);
  }
  
  int current_reading = readings_sum / num_readings;
  
  // Auto-calibrate based on current reading
  // Assume current reading is "normal" lighting (50%)
  // Dark would be roughly 2x this value, bright would be 0.5x this value
  calibration_bright = current_reading / 2;
  calibration_dark = current_reading * 2;
  
  // Ensure values are within ADC range
  calibration_bright = constrain(calibration_bright, 0, 4095);
  calibration_dark = constrain(calibration_dark, 0, 4095);
  
  Serial.printf("Calibration complete: Bright=%d, Dark=%d\n", calibration_bright, calibration_dark);
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Calibration Done!");
  display.printf("Bright: %d\n", calibration_bright);
  display.printf("Dark: %d\n", calibration_dark);
  display.display();
  delay(2000);
}

void handleRoot() {
  String html = R"HTMLDOC(
<!DOCTYPE html>
<html>
<head>
    <title>LDR Light Sensor Analytics</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.9.1/chart.min.js"></script>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap');
        
        * { 
            margin: 0; 
            padding: 0; 
            box-sizing: border-box; 
        }
        
        body {
            font-family: 'Inter', sans-serif;
            background: linear-gradient(135deg, #ffecd2 0%, #fcb69f 100%);
            min-height: 100vh;
            color: #333;
        }
        
        .header {
            background: rgba(255, 255, 255, 0.95);
            padding: 25px;
            text-align: center;
            box-shadow: 0 4px 20px rgba(0,0,0,0.1);
            margin-bottom: 25px;
        }
        
        .header h1 {
            color: #d35400;
            font-size: 2.8rem;
            font-weight: 700;
            margin-bottom: 10px;
        }
        
        .header p {
            color: #7f8c8d;
            font-size: 1.2rem;
            font-weight: 400;
        }
        
        .status-card {
            background: linear-gradient(135deg, #f39c12, #e67e22);
            color: white;
            padding: 20px;
            margin: 0 20px 20px 20px;
            border-radius: 15px;
            text-align: center;
            box-shadow: 0 6px 20px rgba(243, 156, 18, 0.3);
        }
        
        .status-bright { background: linear-gradient(135deg, #f1c40f, #f39c12); }
        .status-normal { background: linear-gradient(135deg, #27ae60, #2ecc71); }
        .status-dim { background: linear-gradient(135deg, #e67e22, #d35400); }
        .status-dark { background: linear-gradient(135deg, #8e44ad, #9b59b6); }
        
        .dashboard {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(400px, 1fr));
            gap: 25px;
            padding: 25px;
            max-width: 1600px;
            margin: 0 auto;
        }
        
        .wide-card {
            grid-column: 1 / -1;
        }
        
        .card {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 30px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.1);
            transition: all 0.3s ease;
        }
        
        .card:hover {
            transform: translateY(-8px);
            box-shadow: 0 20px 40px rgba(0,0,0,0.15);
        }
        
        .card h2 {
            color: #2c3e50;
            margin-bottom: 25px;
            font-size: 1.6rem;
            font-weight: 600;
            border-bottom: 3px solid #f39c12;
            padding-bottom: 10px;
        }
        
        .metrics-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(160px, 1fr));
            gap: 20px;
            margin: 25px 0;
        }
        
        .metric {
            text-align: center;
            padding: 25px;
            background: linear-gradient(135deg, #fff8e1, #ffecb3);
            border-radius: 15px;
            border: 2px solid #f39c12;
            transition: all 0.3s ease;
        }
        
        .metric:hover {
            transform: scale(1.05);
            box-shadow: 0 8px 25px rgba(243, 156, 18, 0.2);
        }
        
        .metric-value {
            font-size: 2.2rem;
            font-weight: 700;
            color: #d35400;
            margin-bottom: 8px;
        }
        
        .metric-label {
            font-size: 0.95rem;
            color: #7f8c8d;
            text-transform: uppercase;
            letter-spacing: 1px;
            font-weight: 500;
        }
        
        .chart-container {
            position: relative;
            height: 400px;
            margin: 25px 0;
            background: rgba(255, 248, 225, 0.5);
            border-radius: 15px;
            padding: 20px;
        }
        
        .controls {
            display: flex;
            gap: 15px;
            flex-wrap: wrap;
            justify-content: center;
            margin: 25px 0;
        }
        
        button {
            background: linear-gradient(135deg, #f39c12, #e67e22);
            color: white;
            border: none;
            padding: 15px 30px;
            border-radius: 25px;
            cursor: pointer;
            font-size: 1.1rem;
            font-weight: 600;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(243, 156, 18, 0.4);
        }
        
        button:hover {
            transform: translateY(-3px);
            box-shadow: 0 8px 25px rgba(243, 156, 18, 0.6);
        }
        
        .danger {
            background: linear-gradient(135deg, #e74c3c, #c0392b);
        }
        
        .success {
            background: linear-gradient(135deg, #27ae60, #2ecc71);
        }
        
        .light-indicator {
            width: 60px;
            height: 60px;
            border-radius: 50%;
            margin: 0 auto 15px;
            transition: all 0.3s ease;
            border: 3px solid #fff;
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        }
        
        .light-bright { background: #f1c40f; box-shadow: 0 0 30px #f1c40f; }
        .light-normal { background: #27ae60; }
        .light-dim { background: #e67e22; }
        .light-dark { background: #8e44ad; }
    </style>
</head>
<body>
    <div class="header">
        <h1>☀️ LDR Light Sensor Analytics</h1>
        <p>Real-time Photosensitive Resistor Monitoring & Analysis</p>
    </div>
    
    <div id="statusCard" class="status-card">
        <div id="lightIndicator" class="light-indicator"></div>
        <h2 id="lightCondition">Initializing...</h2>
        <p id="lightDetails">Reading sensor data...</p>
    </div>
    
    <div class="dashboard">
        <!-- Current Readings -->
        <div class="card">
            <h2>💡 Current Light Data</h2>
            <div class="metrics-grid">
                <div class="metric">
                    <div id="lightPercentage" class="metric-value">--</div>
                    <div class="metric-label">Light Level %</div>
                </div>
                <div class="metric">
                    <div id="rawValue" class="metric-value">--</div>
                    <div class="metric-label">Raw ADC Value</div>
                </div>
                <div class="metric">
                    <div id="voltage" class="metric-value">--</div>
                    <div class="metric-label">Voltage (V)</div>
                </div>
                <div class="metric">
                    <div id="resistance" class="metric-value">--</div>
                    <div class="metric-label">Resistance (Ω)</div>
                </div>
            </div>
        </div>
        
        <!-- Analytics Summary -->
        <div class="card">
            <h2>📊 Session Analytics</h2>
            <div class="metrics-grid">
                <div class="metric">
                    <div id="totalReadings" class="metric-value">0</div>
                    <div class="metric-label">Total Readings</div>
                </div>
                <div class="metric">
                    <div id="averageLight" class="metric-value">--</div>
                    <div class="metric-label">Average Level</div>
                </div>
                <div class="metric">
                    <div id="minMax" class="metric-value">--</div>
                    <div class="metric-label">Min / Max</div>
                </div>
                <div class="metric">
                    <div id="sessionTime" class="metric-value">--</div>
                    <div class="metric-label">Session Time</div>
                </div>
            </div>
        </div>
        
        <!-- Light Level Timeline -->
        <div class="card wide-card">
            <h2>📈 Light Level Timeline</h2>
            <div class="chart-container">
                <canvas id="timelineChart"></canvas>
            </div>
        </div>
        
        <!-- Condition Distribution -->
        <div class="card">
            <h2>🌟 Light Conditions</h2>
            <div class="chart-container">
                <canvas id="conditionChart"></canvas>
            </div>
        </div>
        
        <!-- Resistance Analysis -->
        <div class="card">
            <h2>⚡ Resistance vs Light</h2>
            <div class="chart-container">
                <canvas id="resistanceChart"></canvas>
            </div>
        </div>
        
        <!-- Controls -->
        <div class="card">
            <h2>🎛️ Control Panel</h2>
            <div class="controls">
                <button onclick="calibrateSensor()">🔧 Calibrate Sensor</button>
                <button class="success" onclick="downloadData()">📊 Export Data</button>
                <button class="danger" onclick="clearData()">🗑️ Clear Data</button>
            </div>
            <div id="statusMessage" style="margin-top: 20px; text-align: center; color: #7f8c8d;">
                <p>System ready for monitoring</p>
            </div>
        </div>
    </div>
    
    <script>
        let timelineChart, conditionChart, resistanceChart;
        let lightHistory = [];
        
        // Initialize charts
        function initCharts() {
            // Timeline Chart
            const timelineCtx = document.getElementById('timelineChart').getContext('2d');
            timelineChart = new Chart(timelineCtx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'Light Level (%)',
                        data: [],
                        borderColor: '#f39c12',
                        backgroundColor: 'rgba(243, 156, 18, 0.1)',
                        borderWidth: 3,
                        fill: true,
                        tension: 0.4,
                        pointBackgroundColor: '#e67e22',
                        pointRadius: 4
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {
                        legend: { display: false }
                    },
                    scales: {
                        y: {
                            beginAtZero: true,
                            max: 100,
                            title: { display: true, text: 'Light Level (%)' }
                        },
                        x: {
                            title: { display: true, text: 'Time' }
                        }
                    }
                }
            });
            
            // Condition Distribution Chart
            const conditionCtx = document.getElementById('conditionChart').getContext('2d');
            conditionChart = new Chart(conditionCtx, {
                type: 'doughnut',
                data: {
                    labels: ['Bright', 'Normal', 'Dim', 'Dark'],
                    datasets: [{
                        data: [0, 0, 0, 0],
                        backgroundColor: ['#f1c40f', '#27ae60', '#e67e22', '#8e44ad'],
                        borderWidth: 3,
                        borderColor: '#fff'
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {
                        legend: { position: 'bottom' }
                    }
                }
            });
            
            // Resistance Chart
            const resistanceCtx = document.getElementById('resistanceChart').getContext('2d');
            resistanceChart = new Chart(resistanceCtx, {
                type: 'scatter',
                data: {
                    datasets: [{
                        label: 'Light vs Resistance',
                        data: [],
                        backgroundColor: 'rgba(243, 156, 18, 0.6)',
                        borderColor: '#f39c12',
                        borderWidth: 2
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        x: {
                            title: { display: true, text: 'Light Level (%)' },
                            min: 0,
                            max: 100
                        },
                        y: {
                            title: { display: true, text: 'Resistance (Ω)' },
                            beginAtZero: true
                        }
                    }
                }
            });
        }
        
        // Fetch sensor data
        async function fetchLightData() {
            try {
                const response = await fetch('/api/light-data');
                const data = await response.json();
                updateDisplay(data);
                updateCharts(data);
            } catch (error) {
                console.error('Error fetching data:', error);
            }
        }
        
        // Fetch analytics
        async function fetchAnalytics() {
            try {
                const response = await fetch('/api/analytics');
                const data = await response.json();
                updateAnalytics(data);
            } catch (error) {
                console.error('Error fetching analytics:', error);
            }
        }
        
        function updateDisplay(data) {
            // Update status card
            const statusCard = document.getElementById('statusCard');
            const lightIndicator = document.getElementById('lightIndicator');
            const lightCondition = document.getElementById('lightCondition');
            const lightDetails = document.getElementById('lightDetails');
            
            // Update indicator and status
            lightIndicator.className = 'light-indicator light-' + data.light_condition.toLowerCase();
            lightCondition.textContent = data.light_condition + ' Light';
            lightDetails.textContent = `${data.light_percentage}% | ${data.raw_value} ADC | ${data.voltage.toFixed(2)}V`;
            
            // Update status card color
            statusCard.className = 'status-card status-' + data.light_condition.toLowerCase();
            
            // Update metrics
            document.getElementById('lightPercentage').textContent = data.light_percentage + '%';
            document.getElementById('rawValue').textContent = data.raw_value;
            document.getElementById('voltage').textContent = data.voltage.toFixed(2) + 'V';
            document.getElementById('resistance').textContent = Math.round(data.resistance) + 'Ω';
        }
        
        function updateAnalytics(data) {
            document.getElementById('totalReadings').textContent = data.readings_count;
            document.getElementById('averageLight').textContent = Math.round(data.average_reading);
            document.getElementById('minMax').textContent = data.min_reading + ' / ' + data.max_reading;
            
            // Calculate session time
            const sessionMinutes = Math.round((Date.now() - data.session_start) / 60000);
            document.getElementById('sessionTime').textContent = sessionMinutes + ' min';
            
            // Update condition chart
            conditionChart.data.datasets[0].data = [
                data.bright_events,
                data.normal_events,
                data.dim_events,
                data.dark_events
            ];
            conditionChart.update('none');
        }
        
        function updateCharts(data) {
            // Add to history
            lightHistory.push({
                time: new Date().toLocaleTimeString(),
                light: data.light_percentage,
                resistance: data.resistance
            });
            
            // Keep only last 20 readings
            if (lightHistory.length > 20) {
                lightHistory.shift();
            }
            
            // Update timeline chart
            timelineChart.data.labels = lightHistory.map(h => h.time);
            timelineChart.data.datasets[0].data = lightHistory.map(h => h.light);
            timelineChart.update('none');
            
            // Update resistance chart
            resistanceChart.data.datasets[0].data = lightHistory.map(h => ({
                x: h.light,
                y: h.resistance
            }));
            resistanceChart.update('none');
        }
        
        async function calibrateSensor() {
            try {
                await fetch('/api/calibration', { method: 'POST' });
                document.getElementById('statusMessage').innerHTML = '<p style="color: #27ae60;">Sensor calibrated successfully!</p>';
            } catch (error) {
                console.error('Calibration error:', error);
            }
        }
        
        async function downloadData() {
            try {
                const response = await fetch('/api/download');
                const blob = await response.blob();
                
                const url = window.URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = 'light_sensor_data_' + new Date().getTime() + '.csv';
                document.body.appendChild(a);
                a.click();
                document.body.removeChild(a);
                window.URL.revokeObjectURL(url);
                
                document.getElementById('statusMessage').innerHTML = '<p style="color: #27ae60;">Data exported successfully!</p>';
            } catch (error) {
                console.error('Download error:', error);
            }
        }
        
        async function clearData() {
            if (confirm('Clear all light sensor data?')) {
                try {
                    await fetch('/api/clear', { method: 'POST' });
                    location.reload();
                } catch (error) {
                    console.error('Clear error:', error);
                }
            }
        }
        
        // Initialize everything
        window.onload = function() {
            initCharts();
            fetchLightData();
            fetchAnalytics();
            
            // Update every 1 second for real-time feel
            setInterval(() => {
                fetchLightData();
                fetchAnalytics();
            }, 1000);
        };
    </script>
</body>
</html>
)HTMLDOC";
  
  server.send(200, "text/html", html);
}

void handleLightData() {
  StaticJsonDocument<300> doc;
  
  doc["raw_value"] = currentLight.raw_value;
  doc["voltage"] = currentLight.voltage;
  doc["resistance"] = currentLight.resistance;
  doc["light_percentage"] = currentLight.light_percentage;
  doc["light_condition"] = currentLight.light_condition;
  doc["timestamp"] = currentLight.timestamp;
  
  String jsonString;
  serializeJson(doc, jsonString);
  server.send(200, "application/json", jsonString);
}

void handleAnalytics() {
  StaticJsonDocument<400> doc;
  
  doc["min_reading"] = analytics.min_reading;
  doc["max_reading"] = analytics.max_reading;
  doc["average_reading"] = analytics.average_reading;
  doc["readings_count"] = analytics.readings_count;
  doc["bright_events"] = analytics.bright_events;
  doc["normal_events"] = analytics.normal_events;
  doc["dim_events"] = analytics.dim_events;
  doc["dark_events"] = analytics.dark_events;
  doc["session_start"] = analytics.session_start;
  doc["auto_led_mode"] = analytics.auto_led_mode;
  doc["alert_threshold"] = analytics.alert_threshold;
  
  String jsonString;
  serializeJson(doc, jsonString);
  server.send(200, "application/json", jsonString);
}

void handleCalibration() {
  calibrateSensor();
  server.send(200, "text/plain", "Calibration completed");
}

void handleDownloadData() {
  server.sendHeader("Content-Type", "text/csv");
  server.sendHeader("Content-Disposition", "attachment; filename=light_sensor_data.csv");
  server.send(200, "text/csv", lightLog);
}

void handleClearData() {
  // Reset all data
  analytics.min_reading = 4095;
  analytics.max_reading = 0;
  analytics.average_reading = 0;
  analytics.readings_count = 0;
  analytics.bright_events = 0;
  analytics.normal_events = 0;
  analytics.dim_events = 0;
  analytics.dark_events = 0;
  analytics.session_start = millis();
  
  reading_index = 0;
  lightLog = "Timestamp,Raw_Value,Voltage,Resistance,Light_Percentage,Condition\n";
  
  server.send(200, "text/plain", "Data cleared successfully");
  Serial.println("All light sensor data cleared");
}
