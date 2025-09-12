#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// WiFi credentials - CHANGE THESE
const char* ssid = "Sukrit_wifi";
const char* password = "abcde123";

// Pin Definitions
#define DHT_PIN 4
#define DHT_TYPE DHT11
#define PIR_PIN 5
#define BUZZER_PIN 18
#define RED_LED_PIN 19
#define YELLOW_LED_PIN 21
#define GREEN_LED_PIN 22
#define EMERGENCY_BTN_PIN 23
#define RELAY1_PIN 25  // Security Light
#define RELAY2_PIN 26  // Fan/Alarm
#define LIGHT_SENSOR_PIN 36
#define POTENTIOMETER_PIN 39

// OLED Display (comment out if not using OLED)
// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64
// #define OLED_RESET -1
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Sensors
DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);

// System States
bool securityArmed = false;
bool emergencyMode = false;
bool motionDetected = false;
unsigned long lastMotionTime = 0;
unsigned long emergencyStartTime = 0;

// Enhanced sensor data structure
struct SensorData {
  float temperature;
  float humidity;
  bool dht11_connected;
  bool motion_detected;
  int light_level;
  int potentiometer_value;
  bool security_armed;
  bool emergency_active;
  unsigned long timestamp;
};

SensorData currentData;
String dataLog = ""; // Store data for Excel export
String alertLog = ""; // Store security alerts

// Function declarations
void handleRoot();
void handleSensorData();
void handleDownloadExcel();
void handleClearData();
void handleToggleSecurity();
void handleEmergencyReset();
void readSensors();
void logData();
void updateDisplay();
void checkSecurity();
void emergencyAlert();
void ledControl();

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(EMERGENCY_BTN_PIN, INPUT_PULLUP);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(POTENTIOMETER_PIN, INPUT);
  
  // Initialize outputs to OFF
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, HIGH); // Green = System OK
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);
  
  // Initialize OLED display (comment out if not using OLED)
  /*
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("CYBERPUNK SECURITY");
  display.println("Initializing...");
  display.display();
  delay(2000);
  */
  
  // Initialize DHT11
  dht.begin();
  
  // Initialize sensor data
  currentData.dht11_connected = false;
  currentData.temperature = 0;
  currentData.humidity = 0;
  currentData.motion_detected = false;
  currentData.light_level = 0;
  currentData.potentiometer_value = 0;
  currentData.security_armed = false;
  currentData.emergency_active = false;
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  Serial.println("CYBERPUNK SECURITY SYSTEM");
  Serial.println("Connecting WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Access your dashboard at: http://");
  Serial.println(WiFi.localIP());
  
  // Update console with status
  Serial.println("SYSTEM ONLINE");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Security: DISARMED");
  
  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/sensor-data", HTTP_GET, handleSensorData);
  server.on("/api/download-excel", HTTP_GET, handleDownloadExcel);
  server.on("/api/clear-data", HTTP_POST, handleClearData);
  server.on("/api/toggle-security", HTTP_POST, handleToggleSecurity);
  server.on("/api/emergency-reset", HTTP_POST, handleEmergencyReset);
  
    // Enable CORS for all routes
  server.enableCORS(true);
  
  server.begin();
  Serial.println("Web server started!");
  Serial.println("CYBERPUNK SECURITY SYSTEM ONLINE!");
  
  // Initialize data log with headers
  dataLog = "Timestamp,Temperature(C),Humidity(%),DHT11_Status,Motion,Light_Level,Security_Armed,Emergency\n";
  alertLog = "Timestamp,Alert_Type,Description\n";
  
  // Startup sequence
  for(int i = 0; i < 3; i++) {
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, HIGH);
    delay(200);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    delay(200);
  }
  digitalWrite(GREEN_LED_PIN, HIGH); // System ready
}

void loop() {
  server.handleClient();
  readSensors();
  logData();
  updateDisplay();
  checkSecurity();
  ledControl();
  
  // Check emergency button
  if(digitalRead(EMERGENCY_BTN_PIN) == LOW) {
    emergencyAlert();
  }
  
  // Print IP address every 30 seconds for easy access
  static unsigned long lastIPPrint = 0;
  if (millis() - lastIPPrint > 30000) {
    lastIPPrint = millis();
    Serial.print("Dashboard URL: http://");
    Serial.println(WiFi.localIP());
  }
  
  delay(1000); // Read sensors every second for better security response
}

void readSensors() {
  // Read DHT11
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  
  // Check if readings are valid
  if (isnan(temp) || isnan(hum)) {
    currentData.dht11_connected = false;
    currentData.temperature = 0;
    currentData.humidity = 0;
    Serial.println("DHT11: Failed to read sensor!");
  } else {
    currentData.dht11_connected = true;
    currentData.temperature = temp;
    currentData.humidity = hum;
    Serial.printf("DHT11: Temp=%.1f°C, Humidity=%.1f%%\n", temp, hum);
  }
  
  // Read PIR Motion Sensor
  bool currentMotion = digitalRead(PIR_PIN);
  if(currentMotion && !currentData.motion_detected) {
    lastMotionTime = millis();
    Serial.println("MOTION DETECTED!");
    if(securityArmed) {
      alertLog += String(millis()) + ",MOTION_ALERT,Unauthorized motion detected\n";
    }
  }
  currentData.motion_detected = currentMotion;
  
  // Read Light Sensor (0-4095 range)
  currentData.light_level = analogRead(LIGHT_SENSOR_PIN);
  
  // Read Potentiometer (used for sensitivity control)
  currentData.potentiometer_value = analogRead(POTENTIOMETER_PIN);
  
  // Update system states
  currentData.security_armed = securityArmed;
  currentData.emergency_active = emergencyMode;
  currentData.timestamp = millis();
}

void logData() {
  // Add current data to log every minute
  static unsigned long lastLogTime = 0;
  if (millis() - lastLogTime > 60000) { // Log every 60 seconds
    lastLogTime = millis();
    
    String logEntry = String(currentData.timestamp) + "," + 
                     String(currentData.temperature, 1) + "," + 
                     String(currentData.humidity, 1) + "," + 
                     (currentData.dht11_connected ? "Connected" : "Disconnected") + "," +
                     (currentData.motion_detected ? "MOTION" : "Clear") + "," +
                     String(currentData.light_level) + "," +
                     (currentData.security_armed ? "ARMED" : "DISARMED") + "," +
                     (currentData.emergency_active ? "EMERGENCY" : "Normal") + "\n";
    
    dataLog += logEntry;
    
    // Prevent memory overflow - keep only last 1000 entries
    int lineCount = 0;
    for (int i = 0; i < dataLog.length(); i++) {
      if (dataLog[i] == '\n') lineCount++;
    }
    
    if (lineCount > 1000) {
      int firstNewline = dataLog.indexOf('\n');
      int secondNewline = dataLog.indexOf('\n', firstNewline + 1);
      dataLog = dataLog.substring(0, firstNewline + 1) + dataLog.substring(secondNewline + 1);
    }
  }
}

void updateDisplay() {
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 2000) { // Update every 2 seconds
    lastDisplayUpdate = millis();
    
    // Use Serial output instead of OLED for now
    Serial.println("=== CYBERPUNK SECURITY STATUS ===");
    if (emergencyMode) {
      Serial.println("*** EMERGENCY ACTIVE! ***");
    } else {
      Serial.printf("Temp: %.1fC | Humidity: %.1f%%\n", currentData.temperature, currentData.humidity);
      Serial.printf("Light: %d | Motion: %s\n", currentData.light_level, currentData.motion_detected ? "DETECTED" : "Clear");
      Serial.printf("Security: %s\n", securityArmed ? "ARMED" : "DISARMED");
      Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    }
    Serial.println("================================");
    
    /*
    // OLED Display code (uncomment when OLED is connected)
    display.clearDisplay();
    display.setCursor(0, 0);
    
    if (emergencyMode) {
      display.setTextSize(2);
      display.println("EMERGENCY");
      display.setTextSize(1);
      display.println("ACTIVE!");
    } else {
      display.setTextSize(1);
      display.println("CYBERPUNK SECURITY");
      display.println("==================");
      display.printf("Temp: %.1fC Hum: %.1f%%\n", currentData.temperature, currentData.humidity);
      display.printf("Light: %d\n", currentData.light_level);
      display.printf("Motion: %s\n", currentData.motion_detected ? "DETECTED" : "Clear");
      display.printf("Security: %s\n", securityArmed ? "ARMED" : "DISARMED");
      display.printf("IP: %s", WiFi.localIP().toString().c_str());
    }
    
    display.display();
    */
  }
}

void checkSecurity() {
  if (securityArmed && currentData.motion_detected) {
    // Security breach detected
    digitalWrite(RELAY1_PIN, HIGH); // Turn on security light
    
    // Sound alarm based on potentiometer sensitivity
    int sensitivity = map(currentData.potentiometer_value, 0, 4095, 500, 2000);
    tone(BUZZER_PIN, sensitivity, 500);
    
    Serial.println("SECURITY BREACH! Motion detected while armed!");
  } else {
    digitalWrite(RELAY1_PIN, LOW); // Turn off security light
    noTone(BUZZER_PIN);
  }
  
  // Environmental controls
  if (currentData.temperature > 30.0) {
    digitalWrite(RELAY2_PIN, HIGH); // Turn on fan/cooling
  } else if (currentData.temperature < 25.0) {
    digitalWrite(RELAY2_PIN, LOW); // Turn off fan
  }
}

void emergencyAlert() {
  emergencyMode = true;
  emergencyStartTime = millis();
  currentData.emergency_active = true;
  
  Serial.println("EMERGENCY ACTIVATED!");
  alertLog += String(millis()) + ",EMERGENCY,Manual emergency button pressed\n";
  
  // Emergency will auto-reset after 30 seconds
  if (millis() - emergencyStartTime > 30000) {
    emergencyMode = false;
    currentData.emergency_active = false;
  }
}

void ledControl() {
  if (emergencyMode) {
    // Emergency mode - flash all LEDs rapidly
    static unsigned long lastFlash = 0;
    static bool flashState = false;
    
    if (millis() - lastFlash > 200) {
      lastFlash = millis();
      flashState = !flashState;
      
      digitalWrite(RED_LED_PIN, flashState);
      digitalWrite(YELLOW_LED_PIN, flashState);
      digitalWrite(GREEN_LED_PIN, flashState);
      
      // Emergency siren
      if (flashState) {
        tone(BUZZER_PIN, 1000, 100);
      } else {
        tone(BUZZER_PIN, 500, 100);
      }
    }
  } else {
    // Normal operation
    digitalWrite(RED_LED_PIN, securityArmed && currentData.motion_detected);
    digitalWrite(YELLOW_LED_PIN, securityArmed);
    digitalWrite(GREEN_LED_PIN, !securityArmed && currentData.dht11_connected);
  }
}

void handleRoot() {
  String html = R"HTMLDOC(
<!DOCTYPE html>
<html>
<head>
    <title>CYBERPUNK Security System</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.9.1/chart.min.js"></script>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700;900&display=swap');
        
        * { 
            margin: 0; 
            padding: 0; 
            box-sizing: border-box; 
        }
        
        body {
            font-family: 'Orbitron', monospace;
            background: linear-gradient(135deg, #0f0f0f 0%, #1a0033 50%, #000 100%);
            min-height: 100vh;
            color: #00ff41;
            overflow-x: hidden;
        }
        
        .header {
            text-align: center;
            padding: 20px;
            color: #00ff41;
            text-shadow: 0 0 20px #00ff41;
            border-bottom: 2px solid #00ff41;
            margin-bottom: 20px;
        }
        
        .header h1 {
            font-size: 3rem;
            font-weight: 900;
            margin-bottom: 10px;
            text-transform: uppercase;
            letter-spacing: 3px;
            animation: neonGlow 2s ease-in-out infinite alternate;
        }
        
        @keyframes neonGlow {
            from { text-shadow: 0 0 20px #00ff41, 0 0 30px #00ff41; }
            to { text-shadow: 0 0 10px #00ff41, 0 0 20px #00ff41; }
        }
        
        .dashboard {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 20px;
            padding: 20px;
            max-width: 1600px;
            margin: 0 auto;
        }
        
        .wide-card {
            grid-column: 1 / -1;
        }
        
        .card {
            background: rgba(0, 0, 0, 0.8);
            border: 2px solid #00ff41;
            border-radius: 15px;
            padding: 25px;
            box-shadow: 0 0 30px rgba(0, 255, 65, 0.3);
            transition: all 0.3s ease;
        }
        
        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 10px 50px rgba(0, 255, 65, 0.5);
            border-color: #ff0080;
        }
        
        .card h2 {
            color: #00ff41;
            margin-bottom: 20px;
            font-size: 1.5rem;
            text-transform: uppercase;
            letter-spacing: 2px;
        }
        
        .sensor-status {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 0.8rem;
            font-weight: bold;
            text-transform: uppercase;
        }
        
        .connected { 
            background: #00ff41; 
            color: #000; 
            box-shadow: 0 0 15px #00ff41;
        }
        
        .disconnected { 
            background: #ff0080; 
            color: #fff; 
            box-shadow: 0 0 15px #ff0080;
        }
        
        .armed {
            background: #ff4500;
            color: #fff;
            box-shadow: 0 0 15px #ff4500;
        }
        
        .emergency {
            background: #ff0000;
            color: #fff;
            box-shadow: 0 0 15px #ff0000;
            animation: emergencyFlash 0.5s infinite;
        }
        
        @keyframes emergencyFlash {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.3; }
        }
        
        .metric-display {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 15px;
            margin: 20px 0;
        }
        
        .metric {
            text-align: center;
            padding: 20px;
            background: linear-gradient(135deg, rgba(0, 255, 65, 0.2), rgba(255, 0, 128, 0.2));
            border: 1px solid #00ff41;
            border-radius: 10px;
            transition: all 0.3s ease;
        }
        
        .metric-value {
            font-size: 2.5rem;
            font-weight: bold;
            color: #00ff41;
            text-shadow: 0 0 10px #00ff41;
        }
        
        .metric-label {
            font-size: 0.9rem;
            color: #ccc;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .chart-container {
            position: relative;
            height: 300px;
            margin: 20px 0;
            background: rgba(0, 0, 0, 0.5);
            border-radius: 10px;
            padding: 15px;
        }
        
        .controls {
            text-align: center;
            margin: 20px 0;
            display: flex;
            gap: 15px;
            flex-wrap: wrap;
            justify-content: center;
        }
        
        button {
            background: linear-gradient(135deg, #00ff41, #00cc33);
            color: #000;
            border: 2px solid #00ff41;
            padding: 15px 30px;
            border-radius: 25px;
            cursor: pointer;
            font-size: 1rem;
            font-family: 'Orbitron', monospace;
            font-weight: bold;
            text-transform: uppercase;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(0, 255, 65, 0.4);
        }
        
        button:hover {
            transform: translateY(-3px);
            box-shadow: 0 8px 25px rgba(0, 255, 65, 0.6);
        }
        
        .danger { 
            background: linear-gradient(135deg, #ff0080, #cc0066);
            border-color: #ff0080;
            color: #fff;
        }
        
        .warning {
            background: linear-gradient(135deg, #ff4500, #cc3300);
            border-color: #ff4500;
            color: #fff;
        }

        .motion-alert {
            background: rgba(255, 0, 0, 0.2);
            border: 2px solid #ff0000;
            padding: 10px;
            border-radius: 10px;
            margin: 10px 0;
            text-align: center;
            color: #ff0000;
            font-weight: bold;
            animation: alertPulse 1s infinite;
        }

        @keyframes alertPulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.7; }
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>◉ CYBERPUNK SECURITY ◉</h1>
        <p>NEURAL INTERFACE v2.0 | QUANTUM ENCRYPTION</p>
    </div>
    
    <div class="dashboard">
        <!-- Environmental Monitoring -->
        <div class="card">
            <h2>⚡ Environmental Scan <span id="dht11Status" class="sensor-status disconnected">INIT</span></h2>
            
            <div class="metric-display">
                <div class="metric">
                    <div id="tempValue" class="metric-value">--</div>
                    <div class="metric-label">Temperature °C</div>
                </div>
                <div class="metric">
                    <div id="humValue" class="metric-value">--</div>
                    <div class="metric-label">Humidity %</div>
                </div>
                <div class="metric">
                    <div id="lightValue" class="metric-value">--</div>
                    <div class="metric-label">Light Level</div>
                </div>
            </div>

            <div class="chart-container">
                <canvas id="environmentChart"></canvas>
            </div>
        </div>
        
        <!-- Security Control -->
        <div class="card">
            <h2>🛡️ Security Protocol <span id="securityStatus" class="sensor-status disconnected">DISARMED</span></h2>
            
            <div class="metric-display">
                <div class="metric">
                    <div id="motionValue" class="metric-value">--</div>
                    <div class="metric-label">Motion Status</div>
                </div>
                <div class="metric">
                    <div id="sensitivityValue" class="metric-value">--</div>
                    <div class="metric-label">Sensitivity</div>
                </div>
            </div>
            
            <div id="motionAlert" style="display: none;" class="motion-alert">
                🚨 MOTION DETECTED! 🚨
            </div>
            
            <div class="controls">
                <button id="securityToggle" onclick="toggleSecurity()">ARM SECURITY</button>
                <button class="danger" onclick="emergencyReset()">EMERGENCY</button>
            </div>
        </div>

        <!-- Motion Sensor Analytics - Wide Card -->
        <div class="card wide-card">
            <h2>📊 Motion Detection Analytics</h2>
            
            <div class="metric-display">
                <div class="metric">
                    <div id="motionCount" class="metric-value">0</div>
                    <div class="metric-label">Motion Events</div>
                </div>
                <div class="metric">
                    <div id="motionRate" class="metric-value">0</div>
                    <div class="metric-label">Events/Hour</div>
                </div>
                <div class="metric">
                    <div id="lastMotion" class="metric-value">--</div>
                    <div class="metric-label">Last Detection</div>
                </div>
            </div>

            <div class="chart-container">
                <canvas id="motionChart"></canvas>
            </div>
        </div>

        <!-- Light Level Analytics -->
        <div class="card wide-card">
            <h2>💡 Light Level Patterns</h2>
            <div class="chart-container">
                <canvas id="lightChart"></canvas>
            </div>
        </div>
        
        <!-- Data Controls -->
        <div class="card">
            <h2>💾 Data Management</h2>
            <div class="controls">
                <button onclick="downloadExcel()">📊 EXPORT DATA</button>
                <button class="warning" onclick="clearData()">🗑️ PURGE MEMORY</button>
            </div>
            <div id="dataStatus" style="margin-top: 15px; text-align: center; color: #00ff41;">
                <p>◉ QUANTUM STORAGE ACTIVE ◉</p>
            </div>
        </div>
    </div>
    
    <script>
        let environmentChart, motionChart, lightChart;
        let motionData = [];
        let lightData = [];
        let motionEvents = 0;
        let motionHistory = [];
        let startTime = Date.now();

        // Initialize charts
        function initCharts() {
            // Environment Chart (Temperature & Humidity)
            const envCtx = document.getElementById('environmentChart').getContext('2d');
            environmentChart = new Chart(envCtx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'Temperature (°C)',
                        data: [],
                        borderColor: '#ff0080',
                        backgroundColor: 'rgba(255, 0, 128, 0.1)',
                        borderWidth: 3,
                        fill: true,
                        tension: 0.4
                    }, {
                        label: 'Humidity (%)',
                        data: [],
                        borderColor: '#00ff41',
                        backgroundColor: 'rgba(0, 255, 65, 0.1)',
                        borderWidth: 3,
                        fill: true,
                        tension: 0.4
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {
                        legend: { 
                            labels: { color: '#00ff41' }
                        }
                    },
                    scales: {
                        y: {
                            ticks: { color: '#00ff41' },
                            grid: { color: 'rgba(0, 255, 65, 0.2)' }
                        },
                        x: {
                            ticks: { color: '#00ff41' },
                            grid: { color: 'rgba(0, 255, 65, 0.2)' }
                        }
                    }
                }
            });

            // Motion Detection Bar Chart
            const motionCtx = document.getElementById('motionChart').getContext('2d');
            motionChart = new Chart(motionCtx, {
                type: 'bar',
                data: {
                    labels: ['Last 5min', 'Last 15min', 'Last 30min', 'Last 1hr', 'Last 2hr'],
                    datasets: [{
                        label: 'Motion Events',
                        data: [0, 0, 0, 0, 0],
                        backgroundColor: [
                            'rgba(255, 0, 128, 0.8)',
                            'rgba(255, 69, 0, 0.8)',
                            'rgba(255, 215, 0, 0.8)',
                            'rgba(0, 255, 65, 0.8)',
                            'rgba(0, 191, 255, 0.8)'
                        ],
                        borderColor: [
                            '#ff0080',
                            '#ff4500',
                            '#ffd700',
                            '#00ff41',
                            '#00bfff'
                        ],
                        borderWidth: 2
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {
                        legend: { 
                            labels: { color: '#00ff41' }
                        }
                    },
                    scales: {
                        y: {
                            beginAtZero: true,
                            ticks: { 
                                color: '#00ff41',
                                stepSize: 1
                            },
                            grid: { color: 'rgba(0, 255, 65, 0.2)' }
                        },
                        x: {
                            ticks: { color: '#00ff41' },
                            grid: { color: 'rgba(0, 255, 65, 0.2)' }
                        }
                    }
                }
            });

            // Light Level Line Chart
            const lightCtx = document.getElementById('lightChart').getContext('2d');
            lightChart = new Chart(lightCtx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'Light Level',
                        data: [],
                        borderColor: '#ffd700',
                        backgroundColor: 'rgba(255, 215, 0, 0.2)',
                        borderWidth: 3,
                        fill: true,
                        tension: 0.4
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {
                        legend: { 
                            labels: { color: '#00ff41' }
                        }
                    },
                    scales: {
                        y: {
                            beginAtZero: true,
                            max: 4095,
                            ticks: { color: '#00ff41' },
                            grid: { color: 'rgba(0, 255, 65, 0.2)' }
                        },
                        x: {
                            ticks: { color: '#00ff41' },
                            grid: { color: 'rgba(0, 255, 65, 0.2)' }
                        }
                    }
                }
            });
        }

        // Update motion analytics
        function updateMotionAnalytics(motionDetected) {
            const now = Date.now();
            
            if (motionDetected) {
                motionEvents++;
                motionHistory.push(now);
                
                // Show motion alert
                document.getElementById('motionAlert').style.display = 'block';
                setTimeout(() => {
                    document.getElementById('motionAlert').style.display = 'none';
                }, 3000);
            }

            // Update motion metrics
            document.getElementById('motionCount').textContent = motionEvents;
            
            // Calculate motion rate (events per hour)
            const hourlyRate = (motionEvents / ((now - startTime) / 3600000)).toFixed(1);
            document.getElementById('motionRate').textContent = hourlyRate;
            
            // Last motion time
            if (motionHistory.length > 0) {
                const lastTime = new Date(motionHistory[motionHistory.length - 1]);
                document.getElementById('lastMotion').textContent = lastTime.toLocaleTimeString();
            }

            // Update motion bar chart data
            const intervals = [5, 15, 30, 60, 120]; // minutes
            const motionCounts = intervals.map(interval => {
                const cutoff = now - (interval * 60 * 1000);
                return motionHistory.filter(time => time > cutoff).length;
            });

            motionChart.data.datasets[0].data = motionCounts;
            motionChart.update('none');
        }

        // Fetch sensor data
        async function fetchSensorData() {
            try {
                const response = await fetch('/api/sensor-data');
                const data = await response.json();
                updateDisplay(data);
                updateCharts(data);
                updateMotionAnalytics(data.motion_detected);
            } catch (error) {
                console.error('Neural link error:', error);
            }
        }
        
        function updateDisplay(data) {
            // Environmental data
            if (data.dht11_connected) {
                document.getElementById('dht11Status').textContent = 'ACTIVE';
                document.getElementById('dht11Status').className = 'sensor-status connected';
                document.getElementById('tempValue').textContent = data.temperature.toFixed(1);
                document.getElementById('humValue').textContent = data.humidity.toFixed(1);
            } else {
                document.getElementById('dht11Status').textContent = 'OFFLINE';
                document.getElementById('dht11Status').className = 'sensor-status disconnected';
                document.getElementById('tempValue').textContent = '--';
                document.getElementById('humValue').textContent = '--';
            }
            
            document.getElementById('lightValue').textContent = data.light_level;
            
            // Security data
            const securityEl = document.getElementById('securityStatus');
            const toggleBtn = document.getElementById('securityToggle');
            
            if (data.emergency_active) {
                securityEl.textContent = 'EMERGENCY';
                securityEl.className = 'sensor-status emergency';
            } else if (data.security_armed) {
                securityEl.textContent = 'ARMED';
                securityEl.className = 'sensor-status armed';
                toggleBtn.textContent = 'DISARM SECURITY';
            } else {
                securityEl.textContent = 'DISARMED';
                securityEl.className = 'sensor-status disconnected';
                toggleBtn.textContent = 'ARM SECURITY';
            }
            
            document.getElementById('motionValue').textContent = data.motion_detected ? 'DETECTED' : 'CLEAR';
            document.getElementById('sensitivityValue').textContent = Math.round(data.potentiometer_value / 40.95) + '%';
        }

        function updateCharts(data) {
            const now = new Date().toLocaleTimeString();
            
            // Update environment chart
            if (data.dht11_connected) {
                environmentChart.data.labels.push(now);
                environmentChart.data.datasets[0].data.push(data.temperature);
                environmentChart.data.datasets[1].data.push(data.humidity);
                
                // Keep only last 20 data points
                if (environmentChart.data.labels.length > 20) {
                    environmentChart.data.labels.shift();
                    environmentChart.data.datasets[0].data.shift();
                    environmentChart.data.datasets[1].data.shift();
                }
                
                environmentChart.update('none');
            }

            // Update light chart
            lightChart.data.labels.push(now);
            lightChart.data.datasets[0].data.push(data.light_level);
            
            if (lightChart.data.labels.length > 20) {
                lightChart.data.labels.shift();
                lightChart.data.datasets[0].data.shift();
            }
            
            lightChart.update('none');
        }
        
        async function toggleSecurity() {
            try {
                await fetch('/api/toggle-security', { method: 'POST' });
                document.getElementById('dataStatus').innerHTML = '<p style="color: #00ff41;">Security toggled</p>';
            } catch (error) {
                console.error('Error:', error);
            }
        }
        
        async function emergencyReset() {
            if (confirm('ACTIVATE EMERGENCY PROTOCOL?')) {
                try {
                    await fetch('/api/emergency-reset', { method: 'POST' });
                    document.getElementById('dataStatus').innerHTML = '<p style="color: #ff0080;">Emergency activated</p>';
                } catch (error) {
                    console.error('Error:', error);
                }
            }
        }
        
        async function downloadExcel() {
            try {
                const response = await fetch('/api/download-excel');
                const blob = await response.blob();
                
                const url = window.URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = 'cyberpunk_data_' + new Date().getTime() + '.csv';
                document.body.appendChild(a);
                a.click();
                document.body.removeChild(a);
                window.URL.revokeObjectURL(url);
                
                document.getElementById('dataStatus').innerHTML = '<p style="color: #00ff41;">Data exported!</p>';
            } catch (error) {
                console.error('Export error:', error);
            }
        }
        
        async function clearData() {
            if (confirm('PURGE ALL MEMORY BANKS?')) {
                try {
                    await fetch('/api/clear-data', { method: 'POST' });
                    document.getElementById('dataStatus').innerHTML = '<p style="color: #00ff41;">Memory purged</p>';
                    
                    // Reset local analytics
                    motionEvents = 0;
                    motionHistory = [];
                    startTime = Date.now();
                } catch (error) {
                    console.error('Error:', error);
                }
            }
        }
        
        // Initialize
        window.onload = function() {
            initCharts();
            fetchSensorData();
            setInterval(fetchSensorData, 2000);
        };
    </script>
</body>
</html>
)HTMLDOC";
  
  server.send(200, "text/html", html);
}

// Enhanced handleSensorData function to include all sensor data
void handleSensorData() {
  StaticJsonDocument<400> doc;
  
  doc["temperature"] = currentData.temperature;
  doc["humidity"] = currentData.humidity;
  doc["dht11_connected"] = currentData.dht11_connected;
  doc["motion_detected"] = currentData.motion_detected;
  doc["light_level"] = currentData.light_level;
  doc["potentiometer_value"] = currentData.potentiometer_value;
  doc["security_armed"] = currentData.security_armed;
  doc["emergency_active"] = currentData.emergency_active;
  doc["timestamp"] = currentData.timestamp;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  server.send(200, "application/json", jsonString);
}

void handleDownloadExcel() {
  server.sendHeader("Content-Type", "text/csv");
  server.sendHeader("Content-Disposition", "attachment; filename=security_data.csv");
  server.send(200, "text/csv", dataLog);
}

void handleClearData() {
  dataLog = "Timestamp,Temperature(C),Humidity(%),DHT11_Status,Motion,Light_Level,Security_Armed,Emergency\n";
  alertLog = "Timestamp,Alert_Type,Description\n";
  server.send(200, "text/plain", "Data cleared");
}

void handleToggleSecurity() {
  securityArmed = !securityArmed;
  currentData.security_armed = securityArmed;
  
  String response = securityArmed ? "Security ARMED" : "Security DISARMED";
  Serial.println(response);
  
  if(securityArmed) {
    alertLog += String(millis()) + ",SECURITY_ARMED,Security system activated\n";
  } else {
    alertLog += String(millis()) + ",SECURITY_DISARMED,Security system deactivated\n";
  }
  
  server.send(200, "text/plain", response);
}

void handleEmergencyReset() {
  emergencyMode = false;
  currentData.emergency_active = false;
  
  // Turn off all emergency signals
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);
  noTone(BUZZER_PIN);
  
  alertLog += String(millis()) + ",EMERGENCY_RESET,Emergency reset activated\n";
  Serial.println("Emergency reset - system normalized");
  server.send(200, "text/plain", "Emergency reset complete");
}