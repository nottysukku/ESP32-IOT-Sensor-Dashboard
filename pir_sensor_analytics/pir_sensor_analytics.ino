#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WiFi credentials - CHANGE THESE
const char* ssid = "Sukrit_wifi";
const char* password = "abcde123";

// HC-SR501 PIR Motion Sensor Pin Configuration
#define PIR_PIN 5
#define LED_PIN 2          // Built-in LED for visual indication
#define BUZZER_PIN 18      // Optional buzzer for audio feedback
#define SENSITIVITY_POT 39 // Potentiometer for sensitivity adjustment

// Web Server
WebServer server(80);

// Motion Detection Data Structure
struct MotionEvent {
  unsigned long timestamp;
  int duration;
  int sensitivity_level;
  bool is_repeated;
};

// Motion Analytics Data
struct MotionAnalytics {
  int total_events;
  int events_last_hour;
  int events_last_day;
  float average_duration;
  int longest_duration;
  int shortest_duration;
  float events_per_hour;
  int current_sensitivity;
  bool motion_active;
  unsigned long last_motion_time;
  unsigned long motion_start_time;
  unsigned long session_start;
};

// Global Variables
MotionAnalytics analytics;
MotionEvent recent_events[100]; // Store last 100 events
int event_index = 0;
String motion_log = "";

// Real-time data for charts
String hourly_data = "[";
String daily_pattern = "[";
String sensitivity_data = "[";

// Function Declarations
void handleRoot();
void handleMotionData();
void handleAnalytics();
void handleEvents();
void handleDownloadData();
void handleClearData();
void readMotionSensor();
void updateAnalytics();
void logMotionEvent(int duration, int sensitivity);
void generateHourlyData();
void generateDailyPattern();

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SENSITIVITY_POT, INPUT);
  
  // Initialize outputs
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize analytics
  analytics.total_events = 0;
  analytics.events_last_hour = 0;
  analytics.events_last_day = 0;
  analytics.average_duration = 0;
  analytics.longest_duration = 0;
  analytics.shortest_duration = 999999;
  analytics.events_per_hour = 0;
  analytics.current_sensitivity = 50;
  analytics.motion_active = false;
  analytics.last_motion_time = 0;
  analytics.motion_start_time = 0;
  analytics.session_start = millis();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("HC-SR501 PIR Motion Sensor Analytics");
  Serial.println("Connecting to WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Motion Analytics Dashboard: http://");
  Serial.println(WiFi.localIP());
  
  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/motion-data", HTTP_GET, handleMotionData);
  server.on("/api/analytics", HTTP_GET, handleAnalytics);
  server.on("/api/events", HTTP_GET, handleEvents);
  server.on("/api/download", HTTP_GET, handleDownloadData);
  server.on("/api/clear", HTTP_POST, handleClearData);
  
  server.enableCORS(true);
  server.begin();
  
  Serial.println("PIR Motion Sensor Web Server Started!");
  Serial.println("=====================================");
  
  // Initialize data log
  motion_log = "Timestamp,Duration(ms),Sensitivity(%),Event_Type\n";
  
  // Startup indication
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  
  Serial.println("PIR Sensor Ready - Monitoring Motion...");
}

void loop() {
  server.handleClient();
  readMotionSensor();
  updateAnalytics();
  
  // Print status every 10 seconds
  static unsigned long lastStatus = 0;
  if (millis() - lastStatus > 10000) {
    lastStatus = millis();
    Serial.printf("Motion Events: %d | Active: %s | Sensitivity: %d%%\n", 
                  analytics.total_events, 
                  analytics.motion_active ? "YES" : "NO",
                  analytics.current_sensitivity);
    Serial.printf("Dashboard: http://%s\n", WiFi.localIP().toString().c_str());
  }
  
  delay(10); // Much faster polling - 10ms delay for better responsiveness
}

void readMotionSensor() {
  static bool last_motion_state = false;
  static unsigned long motion_start = 0;
  static unsigned long motion_end_time = 0;
  static unsigned long last_reading_time = 0;
  static int motion_confirm_count = 0;
  static int no_motion_confirm_count = 0;
  
  // Faster polling - read every 25ms for better responsiveness
  if (millis() - last_reading_time < 25) {
    return;
  }
  last_reading_time = millis();
  
  // Read PIR sensor multiple times for stability
  bool current_motion = digitalRead(PIR_PIN);
  
  // Read sensitivity from potentiometer (0-100%)
  analytics.current_sensitivity = map(analogRead(SENSITIVITY_POT), 0, 4095, 0, 100);
  
  // Debounce logic - require multiple consistent readings
  if (current_motion) {
    motion_confirm_count++;
    no_motion_confirm_count = 0;
    
    // Require 2 consecutive readings for motion detection (faster response)
    if (motion_confirm_count >= 2 && !last_motion_state) {
      motion_start = millis();
      analytics.motion_active = true;
      analytics.motion_start_time = motion_start;
      digitalWrite(LED_PIN, HIGH);
      
      Serial.println(">>> MOTION DETECTED! <<<");
      
      // Optional buzzer feedback based on sensitivity
      if (analytics.current_sensitivity > 70) {
        tone(BUZZER_PIN, 1000, 100);
      }
      
      last_motion_state = true;
    }
  } else {
    no_motion_confirm_count++;
    motion_confirm_count = 0;
    
    // Quick reset - only need 1 reading for no motion (faster reset)
    if (no_motion_confirm_count >= 1 && last_motion_state) {
      motion_end_time = millis();
      unsigned long motion_duration = motion_end_time - motion_start;
      
      // Only log if motion was active for at least 100ms (filter noise)
      if (motion_duration >= 100) {
        analytics.motion_active = false;
        analytics.last_motion_time = motion_end_time;
        digitalWrite(LED_PIN, LOW);
        
        // Log this motion event
        logMotionEvent(motion_duration, analytics.current_sensitivity);
        
        Serial.printf("Motion ended - Duration: %lu ms\n", motion_duration);
      }
      
      last_motion_state = false;
      analytics.motion_active = false;
      digitalWrite(LED_PIN, LOW);
    }
  }
  
  // Force reset if motion has been "active" too long (HC-SR501 timeout handling)
  if (analytics.motion_active && (millis() - motion_start) > 10000) {
    Serial.println("Force reset - Motion timeout");
    analytics.motion_active = false;
    digitalWrite(LED_PIN, LOW);
    last_motion_state = false;
    motion_confirm_count = 0;
    no_motion_confirm_count = 0;
  }
}

void logMotionEvent(int duration, int sensitivity) {
  analytics.total_events++;
  
  // Update duration statistics
  if (duration > analytics.longest_duration) {
    analytics.longest_duration = duration;
  }
  if (duration < analytics.shortest_duration) {
    analytics.shortest_duration = duration;
  }
  
  // Calculate average duration
  static long total_duration = 0;
  total_duration += duration;
  analytics.average_duration = (float)total_duration / analytics.total_events;
  
  // Store event in array
  if (event_index < 100) {
    recent_events[event_index].timestamp = millis();
    recent_events[event_index].duration = duration;
    recent_events[event_index].sensitivity_level = sensitivity;
    recent_events[event_index].is_repeated = (millis() - analytics.last_motion_time) < 5000;
    event_index++;
  } else {
    // Shift array when full
    for (int i = 0; i < 99; i++) {
      recent_events[i] = recent_events[i + 1];
    }
    recent_events[99].timestamp = millis();
    recent_events[99].duration = duration;
    recent_events[99].sensitivity_level = sensitivity;
    recent_events[99].is_repeated = (millis() - analytics.last_motion_time) < 5000;
  }
  
  // Add to CSV log
  motion_log += String(millis()) + "," + String(duration) + "," + 
               String(sensitivity) + "," + 
               (recent_events[event_index-1].is_repeated ? "Repeated" : "New") + "\n";
}

void updateAnalytics() {
  unsigned long now = millis();
  unsigned long hour_ago = now - 3600000; // 1 hour
  unsigned long day_ago = now - 86400000; // 24 hours
  
  // Count events in last hour and day
  analytics.events_last_hour = 0;
  analytics.events_last_day = 0;
  
  int total_events = (event_index < 100) ? event_index : 100;
  
  for (int i = 0; i < total_events; i++) {
    if (recent_events[i].timestamp > hour_ago) {
      analytics.events_last_hour++;
    }
    if (recent_events[i].timestamp > day_ago) {
      analytics.events_last_day++;
    }
  }
  
  // Calculate events per hour
  unsigned long session_hours = (now - analytics.session_start) / 3600000;
  if (session_hours > 0) {
    analytics.events_per_hour = (float)analytics.total_events / session_hours;
  }
}

void handleRoot() {
  String html = R"HTMLDOC(
<!DOCTYPE html>
<html>
<head>
    <title>HC-SR501 PIR Motion Sensor Analytics</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.9.1/chart.min.js"></script>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;500;700&display=swap');
        
        * { 
            margin: 0; 
            padding: 0; 
            box-sizing: border-box; 
        }
        
        body {
            font-family: 'Roboto', sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            color: #333;
        }
        
        .header {
            background: rgba(255, 255, 255, 0.95);
            padding: 20px;
            text-align: center;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            margin-bottom: 20px;
        }
        
        .header h1 {
            color: #4a5568;
            font-size: 2.5rem;
            font-weight: 700;
            margin-bottom: 10px;
        }
        
        .header p {
            color: #718096;
            font-size: 1.1rem;
        }
        
        .status-bar {
            background: linear-gradient(90deg, #48bb78, #38a169);
            color: white;
            padding: 15px;
            text-align: center;
            margin: 0 20px 20px 20px;
            border-radius: 10px;
            box-shadow: 0 4px 15px rgba(72, 187, 120, 0.3);
        }
        
        .motion-active {
            background: linear-gradient(90deg, #f56565, #e53e3e);
            animation: pulse 1s infinite;
        }
        
        @keyframes pulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.02); }
        }
        
        .dashboard {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(400px, 1fr));
            gap: 20px;
            padding: 20px;
            max-width: 1600px;
            margin: 0 auto;
        }
        
        .wide-card {
            grid-column: 1 / -1;
        }
        
        .card {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 15px;
            padding: 25px;
            box-shadow: 0 8px 25px rgba(0,0,0,0.1);
            transition: all 0.3s ease;
        }
        
        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 15px 35px rgba(0,0,0,0.15);
        }
        
        .card h2 {
            color: #4a5568;
            margin-bottom: 20px;
            font-size: 1.5rem;
            font-weight: 500;
            border-bottom: 2px solid #e2e8f0;
            padding-bottom: 10px;
        }
        
        .metrics-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 15px;
            margin: 20px 0;
        }
        
        .metric {
            text-align: center;
            padding: 20px;
            background: linear-gradient(135deg, #f7fafc, #edf2f7);
            border-radius: 10px;
            border: 1px solid #e2e8f0;
        }
        
        .metric-value {
            font-size: 2rem;
            font-weight: 700;
            color: #2d3748;
            margin-bottom: 5px;
        }
        
        .metric-label {
            font-size: 0.9rem;
            color: #718096;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .chart-container {
            position: relative;
            height: 350px;
            margin: 20px 0;
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
            background: linear-gradient(135deg, #667eea, #764ba2);
            color: white;
            border: none;
            padding: 12px 25px;
            border-radius: 25px;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(102, 126, 234, 0.4);
        }
        
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(102, 126, 234, 0.6);
        }
        
        .danger {
            background: linear-gradient(135deg, #f56565, #e53e3e);
        }
        
        .events-list {
            max-height: 300px;
            overflow-y: auto;
            border: 1px solid #e2e8f0;
            border-radius: 8px;
            padding: 10px;
        }
        
        .event-item {
            padding: 10px;
            border-bottom: 1px solid #f1f5f9;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .event-time {
            font-weight: 500;
            color: #4a5568;
        }
        
        .event-duration {
            color: #667eea;
            font-weight: 500;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>🔍 HC-SR501 PIR Motion Sensor</h1>
        <p>Real-time Motion Detection Analytics & Visualization</p>
    </div>
    
    <div id="statusBar" class="status-bar">
        <span id="statusText">System Monitoring - No Motion Detected</span>
    </div>
    
    <div class="dashboard">
        <!-- Real-time Metrics -->
        <div class="card">
            <h2>📊 Real-time Metrics</h2>
            <div class="metrics-grid">
                <div class="metric">
                    <div id="totalEvents" class="metric-value">0</div>
                    <div class="metric-label">Total Events</div>
                </div>
                <div class="metric">
                    <div id="eventsPerHour" class="metric-value">0.0</div>
                    <div class="metric-label">Events/Hour</div>
                </div>
                <div class="metric">
                    <div id="avgDuration" class="metric-value">0</div>
                    <div class="metric-label">Avg Duration (ms)</div>
                </div>
                <div class="metric">
                    <div id="sensitivity" class="metric-value">50</div>
                    <div class="metric-label">Sensitivity %</div>
                </div>
            </div>
        </div>
        
        <!-- Motion Timeline Chart -->
        <div class="card wide-card">
            <h2>📈 Motion Detection Timeline</h2>
            <div class="chart-container">
                <canvas id="timelineChart"></canvas>
            </div>
        </div>
        
        <!-- Duration Analysis -->
        <div class="card">
            <h2>⏱️ Duration Analysis</h2>
            <div class="chart-container">
                <canvas id="durationChart"></canvas>
            </div>
        </div>
        
        <!-- Sensitivity Impact -->
        <div class="card">
            <h2>🎚️ Sensitivity Impact</h2>
            <div class="chart-container">
                <canvas id="sensitivityChart"></canvas>
            </div>
        </div>
        
        <!-- Hourly Pattern -->
        <div class="card wide-card">
            <h2>🕐 Hourly Motion Pattern</h2>
            <div class="chart-container">
                <canvas id="hourlyChart"></canvas>
            </div>
        </div>
        
        <!-- Recent Events -->
        <div class="card">
            <h2>📋 Recent Events</h2>
            <div id="eventsList" class="events-list">
                <div class="event-item">
                    <span class="event-time">No events yet</span>
                    <span class="event-duration">-</span>
                </div>
            </div>
        </div>
        
        <!-- Controls -->
        <div class="card">
            <h2>🎛️ Controls</h2>
            <div class="controls">
                <button onclick="downloadData()">📊 Export Data</button>
                <button class="danger" onclick="clearData()">🗑️ Clear Data</button>
            </div>
        </div>
    </div>
    
    <script>
        let timelineChart, durationChart, sensitivityChart, hourlyChart;
        let motionHistory = [];
        let startTime = Date.now();
        
        // Initialize all charts
        function initCharts() {
            // Timeline Chart
            const timelineCtx = document.getElementById('timelineChart').getContext('2d');
            timelineChart = new Chart(timelineCtx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'Motion Events',
                        data: [],
                        borderColor: '#667eea',
                        backgroundColor: 'rgba(102, 126, 234, 0.1)',
                        borderWidth: 3,
                        fill: true,
                        tension: 0.4,
                        pointBackgroundColor: '#f56565',
                        pointRadius: 5
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
                            title: { display: true, text: 'Motion Duration (ms)' }
                        },
                        x: {
                            title: { display: true, text: 'Time' }
                        }
                    }
                }
            });
            
            // Duration Distribution Chart
            const durationCtx = document.getElementById('durationChart').getContext('2d');
            durationChart = new Chart(durationCtx, {
                type: 'doughnut',
                data: {
                    labels: ['Short (0-1s)', 'Medium (1-5s)', 'Long (5s+)'],
                    datasets: [{
                        data: [0, 0, 0],
                        backgroundColor: ['#48bb78', '#ed8936', '#f56565'],
                        borderWidth: 2,
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
            
            // Sensitivity Impact Chart
            const sensitivityCtx = document.getElementById('sensitivityChart').getContext('2d');
            sensitivityChart = new Chart(sensitivityCtx, {
                type: 'scatter',
                data: {
                    datasets: [{
                        label: 'Events by Sensitivity',
                        data: [],
                        backgroundColor: 'rgba(102, 126, 234, 0.6)',
                        borderColor: '#667eea',
                        borderWidth: 2
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        x: {
                            title: { display: true, text: 'Sensitivity Level (%)' },
                            min: 0,
                            max: 100
                        },
                        y: {
                            title: { display: true, text: 'Duration (ms)' },
                            beginAtZero: true
                        }
                    }
                }
            });
            
            // Hourly Pattern Chart
            const hourlyCtx = document.getElementById('hourlyChart').getContext('2d');
            hourlyChart = new Chart(hourlyCtx, {
                type: 'bar',
                data: {
                    labels: Array.from({length: 24}, (_, i) => i + ':00'),
                    datasets: [{
                        label: 'Motion Events',
                        data: new Array(24).fill(0),
                        backgroundColor: 'rgba(102, 126, 234, 0.6)',
                        borderColor: '#667eea',
                        borderWidth: 2
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        y: {
                            beginAtZero: true,
                            title: { display: true, text: 'Number of Events' }
                        },
                        x: {
                            title: { display: true, text: 'Hour of Day' }
                        }
                    }
                }
            });
        }
        
        // Fetch motion data
        async function fetchMotionData() {
            try {
                const response = await fetch('/api/motion-data');
                const data = await response.json();
                updateDisplay(data);
            } catch (error) {
                console.error('Error fetching data:', error);
            }
        }
        
        // Fetch analytics data
        async function fetchAnalytics() {
            try {
                const response = await fetch('/api/analytics');
                const data = await response.json();
                updateAnalytics(data);
            } catch (error) {
                console.error('Error fetching analytics:', error);
            }
        }
        
        // Fetch recent events
        async function fetchEvents() {
            try {
                const response = await fetch('/api/events');
                const data = await response.json();
                updateEventsList(data);
                updateCharts(data);
            } catch (error) {
                console.error('Error fetching events:', error);
            }
        }
        
        function updateDisplay(data) {
            // Update status bar
            const statusBar = document.getElementById('statusBar');
            const statusText = document.getElementById('statusText');
            
            if (data.motion_active) {
                statusBar.className = 'status-bar motion-active';
                statusText.textContent = '🚨 MOTION DETECTED! - Active Movement Detected';
            } else {
                statusBar.className = 'status-bar';
                // Show time since last motion for better feedback
                if (data.last_motion_time > 0) {
                    const timeSince = Math.round((Date.now() - data.last_motion_time) / 1000);
                    statusText.textContent = `✅ No Motion - Last detected ${timeSince}s ago`;
                } else {
                    statusText.textContent = '✅ System Monitoring - No Motion Detected';
                }
            }
            
            // Update sensitivity
            document.getElementById('sensitivity').textContent = data.current_sensitivity;
        }
        
        function updateAnalytics(data) {
            document.getElementById('totalEvents').textContent = data.total_events;
            document.getElementById('eventsPerHour').textContent = data.events_per_hour.toFixed(1);
            document.getElementById('avgDuration').textContent = Math.round(data.average_duration);
        }
        
        function updateEventsList(events) {
            const eventsList = document.getElementById('eventsList');
            eventsList.innerHTML = '';
            
            events.slice(-10).reverse().forEach(event => {
                const eventItem = document.createElement('div');
                eventItem.className = 'event-item';
                
                const time = new Date(event.timestamp).toLocaleTimeString();
                eventItem.innerHTML = `
                    <span class="event-time">${time}</span>
                    <span class="event-duration">${event.duration}ms</span>
                `;
                
                eventsList.appendChild(eventItem);
            });
        }
        
        function updateCharts(events) {
            // Update timeline chart
            const recentEvents = events.slice(-20);
            timelineChart.data.labels = recentEvents.map(e => new Date(e.timestamp).toLocaleTimeString());
            timelineChart.data.datasets[0].data = recentEvents.map(e => e.duration);
            timelineChart.update('none');
            
            // Update duration distribution
            let short = 0, medium = 0, long = 0;
            events.forEach(event => {
                if (event.duration < 1000) short++;
                else if (event.duration < 5000) medium++;
                else long++;
            });
            durationChart.data.datasets[0].data = [short, medium, long];
            durationChart.update('none');
            
            // Update sensitivity chart
            sensitivityChart.data.datasets[0].data = events.map(e => ({
                x: e.sensitivity_level,
                y: e.duration
            }));
            sensitivityChart.update('none');
            
            // Update hourly pattern
            const hourlyData = new Array(24).fill(0);
            events.forEach(event => {
                const hour = new Date(event.timestamp).getHours();
                hourlyData[hour]++;
            });
            hourlyChart.data.datasets[0].data = hourlyData;
            hourlyChart.update('none');
        }
        
        async function downloadData() {
            try {
                const response = await fetch('/api/download');
                const blob = await response.blob();
                
                const url = window.URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = 'pir_motion_data_' + new Date().getTime() + '.csv';
                document.body.appendChild(a);
                a.click();
                document.body.removeChild(a);
                window.URL.revokeObjectURL(url);
            } catch (error) {
                console.error('Download error:', error);
            }
        }
        
        async function clearData() {
            if (confirm('Clear all motion detection data?')) {
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
            fetchMotionData();
            fetchAnalytics();
            fetchEvents();
            
            // Update every 500ms for faster response
            setInterval(() => {
                fetchMotionData();
            }, 500);
            
            // Update analytics and events every 2 seconds
            setInterval(() => {
                fetchAnalytics();
                fetchEvents();
            }, 2000);
        };
    </script>
</body>
</html>
)HTMLDOC";
  
  server.send(200, "text/html", html);
}

void handleMotionData() {
  StaticJsonDocument<200> doc;
  
  doc["motion_active"] = analytics.motion_active;
  doc["current_sensitivity"] = analytics.current_sensitivity;
  doc["last_motion_time"] = analytics.last_motion_time;
  doc["session_uptime"] = millis() - analytics.session_start;
  
  String jsonString;
  serializeJson(doc, jsonString);
  server.send(200, "application/json", jsonString);
}

void handleAnalytics() {
  StaticJsonDocument<300> doc;
  
  doc["total_events"] = analytics.total_events;
  doc["events_last_hour"] = analytics.events_last_hour;
  doc["events_last_day"] = analytics.events_last_day;
  doc["average_duration"] = analytics.average_duration;
  doc["longest_duration"] = analytics.longest_duration;
  doc["shortest_duration"] = (analytics.shortest_duration == 999999) ? 0 : analytics.shortest_duration;
  doc["events_per_hour"] = analytics.events_per_hour;
  
  String jsonString;
  serializeJson(doc, jsonString);
  server.send(200, "application/json", jsonString);
}

void handleEvents() {
  DynamicJsonDocument doc(4096);
  JsonArray events = doc.to<JsonArray>();
  
  int total_events = (event_index < 100) ? event_index : 100;
  
  for (int i = 0; i < total_events; i++) {
    JsonObject event = events.createNestedObject();
    event["timestamp"] = recent_events[i].timestamp;
    event["duration"] = recent_events[i].duration;
    event["sensitivity_level"] = recent_events[i].sensitivity_level;
    event["is_repeated"] = recent_events[i].is_repeated;
  }
  
  String jsonString;
  serializeJson(doc, jsonString);
  server.send(200, "application/json", jsonString);
}

void handleDownloadData() {
  server.sendHeader("Content-Type", "text/csv");
  server.sendHeader("Content-Disposition", "attachment; filename=pir_motion_data.csv");
  server.send(200, "text/csv", motion_log);
}

void handleClearData() {
  // Reset all data
  analytics.total_events = 0;
  analytics.events_last_hour = 0;
  analytics.events_last_day = 0;
  analytics.average_duration = 0;
  analytics.longest_duration = 0;
  analytics.shortest_duration = 999999;
  analytics.events_per_hour = 0;
  analytics.session_start = millis();
  
  event_index = 0;
  motion_log = "Timestamp,Duration(ms),Sensitivity(%),Event_Type\n";
  
  server.send(200, "text/plain", "Data cleared successfully");
  Serial.println("All motion data cleared");
}
