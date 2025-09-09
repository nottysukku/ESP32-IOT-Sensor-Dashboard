#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// WiFi credentials - CHANGE THESE
const char* ssid = "Sukrit_wifi";
const char* password = "abcde123";

// DHT11 sensor setup
#define DHT_PIN 4
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// Web server on port 80
WebServer server(80);

// Sensor data structure
struct SensorData {
  float temperature;
  float humidity;
  bool dht11_connected;
  unsigned long timestamp;
};

SensorData currentData;
String dataLog = ""; // Store data for Excel export

// Function declarations
void handleRoot();
void handleSensorData();
void handleDownloadExcel();
void handleClearData();
void readSensors();
void logData();

void setup() {
  Serial.begin(115200);
  
  // Initialize DHT11
  dht.begin();
  
  // Initialize sensor data
  currentData.dht11_connected = false;
  currentData.temperature = 0;
  currentData.humidity = 0;
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
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
  
  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/sensor-data", HTTP_GET, handleSensorData);
  server.on("/api/download-excel", HTTP_GET, handleDownloadExcel);
  server.on("/api/clear-data", HTTP_POST, handleClearData);
  
  // Enable CORS for all routes
  server.enableCORS(true);
  
  server.begin();
  Serial.println("Web server started!");
  
  // Initialize data log with headers
  dataLog = "Timestamp,Temperature(C),Humidity(%),DHT11_Status\n";
}

void loop() {
  server.handleClient();
  readSensors();
  logData();
  
  // Print IP address every 30 seconds for easy access
  static unsigned long lastIPPrint = 0;
  if (millis() - lastIPPrint > 30000) {
    lastIPPrint = millis();
    Serial.print("Dashboard URL: http://");
    Serial.println(WiFi.localIP());
  }
  
  delay(500); // Read sensors every .5 seconds
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
                     (currentData.dht11_connected ? "Connected" : "Disconnected") + "\n";
    
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

void handleRoot() {
  String html = R"HTMLDOC(
<!DOCTYPE html>
<html>
<head>
    <title>IoT Sensor Dashboard</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.9.1/chart.min.js"></script>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            color: #333;
        }
        
        .dashboard {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(400px, 1fr));
            gap: 20px;
            padding: 20px;
            max-width: 1400px;
            margin: 0 auto;
        }
        
        .card {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 25px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255,255,255,0.3);
            transition: transform 0.3s ease, box-shadow 0.3s ease;
        }
        
        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 20px 40px rgba(0,0,0,0.3);
        }
        
        .card h2 {
            color: #4a5568;
            margin-bottom: 20px;
            font-size: 1.5rem;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .sensor-status {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 0.8rem;
            font-weight: bold;
        }
        
        .connected { background: #48bb78; color: white; }
        .disconnected { background: #f56565; color: white; }
        
        .chart-container {
            position: relative;
            height: 300px;
            margin: 20px 0;
        }
        
        .metric-display {
            display: flex;
            justify-content: space-between;
            margin: 15px 0;
        }
        
        .metric {
            text-align: center;
            padding: 15px;
            background: linear-gradient(135deg, #4299e1, #3182ce);
            color: white;
            border-radius: 15px;
            flex: 1;
            margin: 0 5px;
        }
        
        .metric-value {
            font-size: 2rem;
            font-weight: bold;
        }
        
        .metric-label {
            font-size: 0.9rem;
            opacity: 0.9;
        }
        
        .controls {
            text-align: center;
            margin: 20px 0;
        }
        
        button {
            background: linear-gradient(135deg, #48bb78, #38a169);
            color: white;
            border: none;
            padding: 12px 25px;
            border-radius: 25px;
            cursor: pointer;
            font-size: 1rem;
            margin: 5px;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(72, 187, 120, 0.4);
        }
        
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(72, 187, 120, 0.6);
        }
        
        .danger { 
            background: linear-gradient(135deg, #f56565, #e53e3e);
            box-shadow: 0 4px 15px rgba(245, 101, 101, 0.4);
        }
        
        .header {
            text-align: center;
            padding: 20px;
            color: white;
        }
        
        .header h1 {
            font-size: 3rem;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        
        .loading {
            display: none;
            text-align: center;
            color: #666;
        }
        
        .spinner {
            border: 4px solid #f3f3f3;
            border-top: 4px solid #3498db;
            border-radius: 50%;
            width: 30px;
            height: 30px;
            animation: spin 1s linear infinite;
            margin: 20px auto;
        }
        
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        
        .null-sensor {
            text-align: center;
            color: #666;
            font-style: italic;
            padding: 40px;
        }
        
        @media (max-width: 768px) {
            .dashboard {
                grid-template-columns: 1fr;
                padding: 10px;
            }
            
            .header h1 {
                font-size: 2rem;
            }
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>IoT Sensor Dashboard</h1>
        <p>Real-time monitoring with ESP32</p>
    </div>
    
    <div class="dashboard">
        <!-- DHT11 Temperature & Humidity Card -->
        <div class="card">
            <h2>
                DHT11 Temperature & Humidity
                <span id="dht11Status" class="sensor-status disconnected">Checking...</span>
            </h2>
            
            <div id="dht11Metrics" class="metric-display">
                <div class="metric">
                    <div id="tempValue" class="metric-value">--</div>
                    <div class="metric-label">Temperature (°C)</div>
                </div>
                <div class="metric">
                    <div id="humValue" class="metric-value">--</div>
                    <div class="metric-label">Humidity (%)</div>
                </div>
            </div>
            
            <div class="chart-container">
                <canvas id="dht11Chart"></canvas>
            </div>
        </div>
        
        <!-- Motion Sensor Card (Placeholder) -->
        <div class="card">
            <h2>
                Motion Sensor
                <span class="sensor-status disconnected">Not Connected</span>
            </h2>
            <div class="null-sensor">
                <p>Please connect PIR motion sensor</p>
                <div class="spinner" style="display: none;"></div>
            </div>
        </div>
        
        <!-- Light Sensor Card (Placeholder) -->
        <div class="card">
            <h2>
                Light Sensor
                <span class="sensor-status disconnected">Not Connected</span>
            </h2>
            <div class="null-sensor">
                <p>Please connect photoresistor sensor</p>
            </div>
        </div>
        
        <!-- Distance Sensor Card (Placeholder) -->
        <div class="card">
            <h2>
                Distance Sensor
                <span class="sensor-status disconnected">Not Connected</span>
            </h2>
            <div class="null-sensor">
                <p>Please connect ultrasonic sensor</p>
            </div>
        </div>
        
        <!-- Controls Card -->
        <div class="card">
            <h2>Data Controls</h2>
            <div class="controls">
                <button onclick="downloadExcel()">Download Excel Report</button>
                <button onclick="clearData()" class="danger">Clear Data Log</button>
            </div>
            <div id="dataStatus" style="margin-top: 15px; text-align: center; color: #666;">
                <p>Data logging active - updates every minute</p>
            </div>
        </div>
    </div>
    
    <script>
        let dht11Chart;
        let sensorData = {
            temperature: [],
            humidity: [],
            timestamps: []
        };
        
        // Initialize charts
        function initCharts() {
            const ctx = document.getElementById('dht11Chart').getContext('2d');
            
            dht11Chart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'Temperature (°C)',
                        data: [],
                        borderColor: '#f56565',
                        backgroundColor: 'rgba(245, 101, 101, 0.1)',
                        borderWidth: 3,
                        fill: true,
                        tension: 0.4
                    }, {
                        label: 'Humidity (%)',
                        data: [],
                        borderColor: '#4299e1',
                        backgroundColor: 'rgba(66, 153, 225, 0.1)',
                        borderWidth: 3,
                        fill: true,
                        tension: 0.4
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    animation: {
                        duration: 1000,
                        easing: 'easeInOutQuart'
                    },
                    scales: {
                        y: {
                            beginAtZero: true,
                            grid: {
                                color: 'rgba(0,0,0,0.1)'
                            }
                        },
                        x: {
                            grid: {
                                color: 'rgba(0,0,0,0.1)'
                            }
                        }
                    },
                    plugins: {
                        legend: {
                            position: 'top',
                        }
                    }
                }
            });
        }
        
        // Fetch sensor data
        async function fetchSensorData() {
            try {
                const response = await fetch('/api/sensor-data');
                const data = await response.json();
                
                updateDHT11Display(data);
                updateChart(data);
                
            } catch (error) {
                console.error('Error fetching sensor data:', error);
                document.getElementById('dht11Status').textContent = 'Connection Error';
                document.getElementById('dht11Status').className = 'sensor-status disconnected';
            }
        }
        
        function updateDHT11Display(data) {
            const statusEl = document.getElementById('dht11Status');
            const tempEl = document.getElementById('tempValue');
            const humEl = document.getElementById('humValue');
            
            if (data.dht11_connected) {
                statusEl.textContent = 'Connected';
                statusEl.className = 'sensor-status connected';
                tempEl.textContent = data.temperature.toFixed(1);
                humEl.textContent = data.humidity.toFixed(1);
            } else {
                statusEl.textContent = 'Disconnected';
                statusEl.className = 'sensor-status disconnected';
                tempEl.textContent = '--';
                humEl.textContent = '--';
            }
        }
        
        function updateChart(data) {
            if (!data.dht11_connected) return;
            
            const now = new Date().toLocaleTimeString();
            
            sensorData.timestamps.push(now);
            sensorData.temperature.push(data.temperature);
            sensorData.humidity.push(data.humidity);
            
            // Keep only last 20 data points
            if (sensorData.timestamps.length > 20) {
                sensorData.timestamps.shift();
                sensorData.temperature.shift();
                sensorData.humidity.shift();
            }
            
            dht11Chart.data.labels = sensorData.timestamps;
            dht11Chart.data.datasets[0].data = sensorData.temperature;
            dht11Chart.data.datasets[1].data = sensorData.humidity;
            
            dht11Chart.update('none');
        }
        
        async function downloadExcel() {
            try {
                const response = await fetch('/api/download-excel');
                const blob = await response.blob();
                
                const url = window.URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = 'sensor_data_' + new Date().getTime() + '.csv';
                document.body.appendChild(a);
                a.click();
                document.body.removeChild(a);
                window.URL.revokeObjectURL(url);
                
                document.getElementById('dataStatus').innerHTML = 
                    '<p style="color: #48bb78;">Excel file downloaded successfully!</p>';
                
            } catch (error) {
                console.error('Error downloading file:', error);
                document.getElementById('dataStatus').innerHTML = 
                    '<p style="color: #f56565;">Error downloading file</p>';
            }
        }
        
        async function clearData() {
            if (confirm('Are you sure you want to clear all logged data?')) {
                try {
                    await fetch('/api/clear-data', { method: 'POST' });
                    document.getElementById('dataStatus').innerHTML = 
                        '<p style="color: #48bb78;">Data log cleared successfully!</p>';
                } catch (error) {
                    console.error('Error clearing data:', error);
                }
            }
        }
        
        // Initialize everything when page loads
        window.onload = function() {
            initCharts();
            fetchSensorData();
            
            // Update data every .5 seconds
            setInterval(fetchSensorData, 500);
        };
    </script>
</body>
</html>
)HTMLDOC";
  
  server.send(200, "text/html", html);
}

void handleSensorData() {
  StaticJsonDocument<200> doc;
  
  doc["temperature"] = currentData.temperature;
  doc["humidity"] = currentData.humidity;
  doc["dht11_connected"] = currentData.dht11_connected;
  doc["timestamp"] = currentData.timestamp;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  server.send(200, "application/json", jsonString);
}

void handleDownloadExcel() {
  server.sendHeader("Content-Type", "text/csv");
  server.sendHeader("Content-Disposition", "attachment; filename=sensor_data.csv");
  server.send(200, "text/csv", dataLog);
}

void handleClearData() {
  dataLog = "Timestamp,Temperature(C),Humidity(%),DHT11_Status\n";
  server.send(200, "text/plain", "Data cleared");
}