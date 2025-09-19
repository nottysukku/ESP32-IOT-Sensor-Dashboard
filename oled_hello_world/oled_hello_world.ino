/*
 * ESP32 OLED Web Gaming & Display Controller
 * 
 * Features:
 * - Web interface with 3 games (Snake, Pong, Tetris)
 * - System information display
 * - Paint tool for drawing
 * - 5-button control (Up, Down, Left, Right, Fire)
 * - Compatible with 0.96" 128x64 SSD1315 OLED Display Module
 * 
 * Connections:
 * OLED VCC  -> 3.3V, OLED GND  -> GND
 * OLED SDA  -> GPIO 21, OLED SCL -> GPIO 22
 * 
 * Game Controls (GPIO pins with pull-up buttons):
 * UP -> GPIO 32, DOWN -> GPIO 33, LEFT -> GPIO 25, RIGHT -> GPIO 26, FIRE -> GPIO 27
 * 
 * Required Libraries:
 * - Adafruit SSD1306, Adafruit GFX Library, WiFi, WebServer, ArduinoJson
 * 
 * Author: ESP32 Gaming System
 * Date: September 2025
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WiFi credentials (from your other project)
const char* ssid = "Sukrit_wifi";
const char* password = "abcde123";

// OLED display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C // 7-bit address (0x78 >> 1). Try 0x3D if this doesn't work

// Game control pins (with internal pull-up resistors)
#define PIN_UP    32
#define PIN_DOWN  33
#define PIN_LEFT  25
#define PIN_RIGHT 26
#define PIN_FIRE  27

// Display and web server objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WebServer server(80);

// Game states and variables
enum GameMode {
  MODE_MENU,
  MODE_INFO,
  MODE_SNAKE,
  MODE_PONG,
  MODE_TETRIS,
  MODE_PAINT
};

GameMode currentMode = MODE_MENU;

// Button states
struct ButtonState {
  bool up, down, left, right, fire;
  bool upPressed, downPressed, leftPressed, rightPressed, firePressed;
};
ButtonState buttons;

// Menu system
int menuSelection = 0;
const char* menuItems[] = {"System Info", "Snake Game", "Pong Game", "Tetris", "Paint Tool", "Web Portal"};
const int menuItemCount = 6;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=========================================");
  Serial.println("ESP32 OLED Web Gaming & Display System");
  Serial.println("=========================================");
  
  // Initialize button pins with internal pull-up resistors
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_FIRE, INPUT_PULLUP);
  
  // Initialize I2C and OLED
  Wire.begin();
  Serial.println("Initializing OLED Display...");
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED allocation failed!");
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println("Alternative address 0x3D also failed!");
      for(;;);
    }
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  Serial.println("OLED Display initialized!");
  
  // Initialize WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Setup web server routes
    setupWebServer();
    server.begin();
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WiFi Connected!");
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.println("Web Portal Ready");
    display.display();
    delay(2000);
  } else {
    Serial.println("\nWiFi connection failed!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WiFi Failed");
    display.println("Check credentials");
    display.display();
    delay(2000);
  }
  
  // Initialize game systems
  initializeGames();
  
  // Show main menu
  currentMode = MODE_MENU;
  drawMenu();
  
  Serial.println("System ready!");
  Serial.println("Use buttons to navigate menu");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Web interface: http://");
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  // Handle web server
  server.handleClient();
  
  // Read button states
  readButtons();
  
  // Handle current mode
  switch (currentMode) {
    case MODE_MENU:
      handleMenu();
      break;
    case MODE_INFO:
      handleInfoDisplay();
      break;
    case MODE_SNAKE:
      handleSnakeGame();
      break;
    case MODE_PONG:
      handlePongGame();
      break;
    case MODE_TETRIS:
      handleTetrisGame();
      break;
    case MODE_PAINT:
      handlePaintTool();
      break;
  }
  
  delay(50); // Small delay for stability
}

// ========== BUTTON HANDLING ==========
void readButtons() {
  // Read current button states (buttons are active LOW with pull-up)
  bool currentUp = !digitalRead(PIN_UP);
  bool currentDown = !digitalRead(PIN_DOWN);
  bool currentLeft = !digitalRead(PIN_LEFT);
  bool currentRight = !digitalRead(PIN_RIGHT);
  bool currentFire = !digitalRead(PIN_FIRE);
  
  // Detect button presses (edge detection)
  buttons.upPressed = currentUp && !buttons.up;
  buttons.downPressed = currentDown && !buttons.down;
  buttons.leftPressed = currentLeft && !buttons.left;
  buttons.rightPressed = currentRight && !buttons.right;
  buttons.firePressed = currentFire && !buttons.fire;
  
  // Update button states
  buttons.up = currentUp;
  buttons.down = currentDown;
  buttons.left = currentLeft;
  buttons.right = currentRight;
  buttons.fire = currentFire;
}

// ========== MENU SYSTEM ==========
void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.println("=== OLED GAMING ===");
  display.println();
  
  for (int i = 0; i < menuItemCount; i++) {
    if (i == menuSelection) {
      display.print("> ");
    } else {
      display.print("  ");
    }
    display.println(menuItems[i]);
  }
  
  display.display();
}

void handleMenu() {
  if (buttons.upPressed) {
    menuSelection = (menuSelection - 1 + menuItemCount) % menuItemCount;
    drawMenu();
  } else if (buttons.downPressed) {
    menuSelection = (menuSelection + 1) % menuItemCount;
    drawMenu();
  } else if (buttons.firePressed) {
    switch (menuSelection) {
      case 0: currentMode = MODE_INFO; displaySystemInfo(); break;
      case 1: currentMode = MODE_SNAKE; initSnake(); break;
      case 2: currentMode = MODE_PONG; initPong(); break;
      case 3: currentMode = MODE_TETRIS; initTetris(); break;
      case 4: currentMode = MODE_PAINT; initPaint(); break;
      case 5: showWebPortalInfo(); break;
    }
  }
}

// ========== SYSTEM INFO DISPLAY ==========
void displaySystemInfo() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.println("=== SYSTEM INFO ===");
  display.println();
  display.print("CPU: ");
  display.print(ESP.getCpuFreqMHz());
  display.println(" MHz");
  display.print("Flash: ");
  display.print(ESP.getFlashChipSize() / 1024 / 1024);
  display.println(" MB");
  display.print("Free RAM: ");
  display.print(ESP.getFreeHeap() / 1024);
  display.println(" KB");
  display.print("Uptime: ");
  display.print(millis() / 1000);
  display.println("s");
  if (WiFi.status() == WL_CONNECTED) {
    display.print("IP: ");
    display.println(WiFi.localIP());
  }
  display.println("FIRE: Back to menu");
  display.display();
}

void handleInfoDisplay() {
  displaySystemInfo();
  if (buttons.firePressed) {
    currentMode = MODE_MENU;
    drawMenu();
  }
}

void showWebPortalInfo() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.println("=== WEB PORTAL ===");
  display.println();
  if (WiFi.status() == WL_CONNECTED) {
    display.println("Portal Active!");
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.println();
    display.println("Open web browser");
    display.println("and navigate to");
    display.println("the IP address");
  } else {
    display.println("WiFi not connected");
    display.println("Check credentials");
  }
  display.println();
  display.println("FIRE: Back to menu");
  display.display();
  
  delay(100);
  if (buttons.firePressed) {
    currentMode = MODE_MENU;
    drawMenu();
  }
}

// ========== GAME INITIALIZATION ==========
void initializeGames() {
  // Initialize any game-specific variables here
  Serial.println("Games initialized");
}

// ========== SNAKE GAME ==========
struct SnakeGame {
  int snakeX[50], snakeY[50];
  int snakeLength;
  int foodX, foodY;
  int dirX, dirY;
  bool gameOver;
  unsigned long lastMove;
  int score;
} snake;

void initSnake() {
  snake.snakeLength = 3;
  snake.snakeX[0] = 64; snake.snakeY[0] = 32;
  snake.snakeX[1] = 60; snake.snakeY[1] = 32;
  snake.snakeX[2] = 56; snake.snakeY[2] = 32;
  snake.dirX = 4; snake.dirY = 0;
  snake.foodX = random(4, 124); snake.foodY = random(4, 60);
  snake.gameOver = false;
  snake.lastMove = millis();
  snake.score = 0;
  drawSnake();
}

void drawSnake() {
  display.clearDisplay();
  
  // Draw snake
  for (int i = 0; i < snake.snakeLength; i++) {
    display.fillRect(snake.snakeX[i], snake.snakeY[i], 4, 4, SSD1306_WHITE);
  }
  
  // Draw food
  display.fillRect(snake.foodX, snake.foodY, 4, 4, SSD1306_WHITE);
  
  // Draw score
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Score: ");
  display.print(snake.score);
  
  if (snake.gameOver) {
    display.setCursor(35, 25);
    display.println("GAME OVER");
    display.setCursor(30, 40);
    display.println("FIRE: Menu");
  }
  
  display.display();
}

void handleSnakeGame() {
  // Handle input
  if (buttons.upPressed && snake.dirY == 0) { snake.dirX = 0; snake.dirY = -4; }
  if (buttons.downPressed && snake.dirY == 0) { snake.dirX = 0; snake.dirY = 4; }
  if (buttons.leftPressed && snake.dirX == 0) { snake.dirX = -4; snake.dirY = 0; }
  if (buttons.rightPressed && snake.dirX == 0) { snake.dirX = 4; snake.dirY = 0; }
  
  if (buttons.firePressed && snake.gameOver) {
    currentMode = MODE_MENU;
    drawMenu();
    return;
  }
  
  // Game logic
  if (millis() - snake.lastMove > 200 && !snake.gameOver) {
    // Move snake
    for (int i = snake.snakeLength - 1; i > 0; i--) {
      snake.snakeX[i] = snake.snakeX[i - 1];
      snake.snakeY[i] = snake.snakeY[i - 1];
    }
    snake.snakeX[0] += snake.dirX;
    snake.snakeY[0] += snake.dirY;
    
    // Check walls
    if (snake.snakeX[0] < 0 || snake.snakeX[0] >= 128 || snake.snakeY[0] < 8 || snake.snakeY[0] >= 64) {
      snake.gameOver = true;
    }
    
    // Check self collision
    for (int i = 1; i < snake.snakeLength; i++) {
      if (snake.snakeX[0] == snake.snakeX[i] && snake.snakeY[0] == snake.snakeY[i]) {
        snake.gameOver = true;
      }
    }
    
    // Check food
    if (abs(snake.snakeX[0] - snake.foodX) < 4 && abs(snake.snakeY[0] - snake.foodY) < 4) {
      snake.snakeLength++;
      snake.score += 10;
      snake.foodX = random(4, 124);
      snake.foodY = random(8, 60);
    }
    
    snake.lastMove = millis();
    drawSnake();
  }
}

// ========== PONG GAME ==========
struct PongGame {
  int ballX, ballY;
  int ballDirX, ballDirY;
  int paddleY;
  int aiPaddleY;
  int playerScore, aiScore;
  unsigned long lastUpdate;
} pong;

void initPong() {
  pong.ballX = 64; pong.ballY = 32;
  pong.ballDirX = 2; pong.ballDirY = 1;
  pong.paddleY = 28; pong.aiPaddleY = 28;
  pong.playerScore = 0; pong.aiScore = 0;
  pong.lastUpdate = millis();
  drawPong();
}

void drawPong() {
  display.clearDisplay();
  
  // Draw ball
  display.fillRect(pong.ballX, pong.ballY, 2, 2, SSD1306_WHITE);
  
  // Draw paddles
  display.fillRect(2, pong.paddleY, 2, 8, SSD1306_WHITE);
  display.fillRect(124, pong.aiPaddleY, 2, 8, SSD1306_WHITE);
  
  // Draw center line
  for (int i = 0; i < 64; i += 4) {
    display.drawPixel(64, i, SSD1306_WHITE);
  }
  
  // Draw scores
  display.setTextSize(1);
  display.setCursor(30, 2);
  display.print(pong.playerScore);
  display.setCursor(90, 2);
  display.print(pong.aiScore);
  
  display.display();
}

void handlePongGame() {
  // Handle input
  if (buttons.upPressed && pong.paddleY > 0) pong.paddleY -= 4;
  if (buttons.downPressed && pong.paddleY < 56) pong.paddleY += 4;
  if (buttons.firePressed) {
    currentMode = MODE_MENU;
    drawMenu();
    return;
  }
  
  // Game logic
  if (millis() - pong.lastUpdate > 50) {
    // Move ball
    pong.ballX += pong.ballDirX;
    pong.ballY += pong.ballDirY;
    
    // Ball collision with top/bottom
    if (pong.ballY <= 0 || pong.ballY >= 62) {
      pong.ballDirY = -pong.ballDirY;
    }
    
    // Ball collision with paddles
    if (pong.ballX <= 4 && pong.ballY >= pong.paddleY && pong.ballY <= pong.paddleY + 8) {
      pong.ballDirX = -pong.ballDirX;
    }
    if (pong.ballX >= 122 && pong.ballY >= pong.aiPaddleY && pong.ballY <= pong.aiPaddleY + 8) {
      pong.ballDirX = -pong.ballDirX;
    }
    
    // AI paddle movement
    if (pong.ballY > pong.aiPaddleY + 4) pong.aiPaddleY += 1;
    else if (pong.ballY < pong.aiPaddleY + 4) pong.aiPaddleY -= 1;
    
    // Score
    if (pong.ballX < 0) {
      pong.aiScore++;
      pong.ballX = 64; pong.ballY = 32;
      pong.ballDirX = 2;
    }
    if (pong.ballX > 128) {
      pong.playerScore++;
      pong.ballX = 64; pong.ballY = 32;
      pong.ballDirX = -2;
    }
    
    pong.lastUpdate = millis();
    drawPong();
  }
}

// ========== TETRIS GAME (Simplified) ==========
void initTetris() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("TETRIS");
  display.setTextSize(1);
  display.setCursor(15, 45);
  display.println("Coming Soon!");
  display.setCursor(5, 55);
  display.println("FIRE: Back to menu");
  display.display();
}

void handleTetrisGame() {
  if (buttons.firePressed) {
    currentMode = MODE_MENU;
    drawMenu();
  }
}

// ========== PAINT TOOL ==========
struct PaintTool {
  int cursorX, cursorY;
  bool drawing;
} paint;

void initPaint() {
  paint.cursorX = 64;
  paint.cursorY = 32;
  paint.drawing = false;
  display.clearDisplay();
  drawPaintCursor();
}

void drawPaintCursor() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Paint Tool - FIRE:Draw");
  
  // Draw cursor
  display.drawRect(paint.cursorX - 1, paint.cursorY - 1, 3, 3, SSD1306_WHITE);
  display.display();
}

void handlePaintTool() {
  bool moved = false;
  
  if (buttons.upPressed && paint.cursorY > 10) {
    paint.cursorY -= 2;
    moved = true;
  }
  if (buttons.downPressed && paint.cursorY < 62) {
    paint.cursorY += 2;
    moved = true;
  }
  if (buttons.leftPressed && paint.cursorX > 1) {
    paint.cursorX -= 2;
    moved = true;
  }
  if (buttons.rightPressed && paint.cursorX < 126) {
    paint.cursorX += 2;
    moved = true;
  }
  
  if (buttons.fire) {
    display.drawPixel(paint.cursorX, paint.cursorY, SSD1306_WHITE);
    paint.drawing = true;
  }
  
  // Long press fire to exit
  static unsigned long fireHoldTime = 0;
  if (buttons.firePressed) {
    fireHoldTime = millis();
  }
  if (buttons.fire && millis() - fireHoldTime > 1000) {
    currentMode = MODE_MENU;
    drawMenu();
    return;
  }
  
  if (moved || paint.drawing) {
    drawPaintCursor();
    paint.drawing = false;
  }
}

// ========== WEB SERVER SETUP ==========
void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/display", HTTP_POST, handleDisplayAPI);
  server.on("/api/game", HTTP_POST, handleGameAPI);
  server.on("/api/status", HTTP_GET, handleStatusAPI);
  server.onNotFound(handleNotFound);
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>ESP32 OLED Gaming Portal</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial; margin: 20px; background: #1a1a1a; color: white; }";
  html += ".container { max-width: 800px; margin: 0 auto; }";
  html += ".game-btn { background: #4CAF50; color: white; border: none; padding: 15px 30px; margin: 10px; border-radius: 5px; font-size: 16px; cursor: pointer; }";
  html += ".game-btn:hover { background: #45a049; }";
  html += ".info-btn { background: #2196F3; }";
  html += ".info-btn:hover { background: #1976D2; }";
  html += ".paint-btn { background: #FF9800; }";
  html += ".paint-btn:hover { background: #F57C00; }";
  html += ".exit-btn { background: #f44336; }";
  html += ".exit-btn:hover { background: #d32f2f; }";
  html += ".status { background: #333; padding: 10px; border-radius: 5px; margin: 10px 0; }";
  html += ".controls { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; max-width: 200px; margin: 20px auto; }";
  html += ".control-btn { padding: 10px; background: #555; border: none; color: white; border-radius: 3px; }";
  html += ".control-btn:active { background: #777; }";
  html += "h1 { text-align: center; color: #4CAF50; }";
  html += "h2 { color: #2196F3; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>ESP32 OLED Gaming Portal</h1>";
  html += "<div class='status' id='status'><h3>System Status</h3><p>Loading...</p></div>";
  html += "<h2>Games</h2>";
  html += "<button class='game-btn' onclick='startGame(\"snake\")'>Snake Game</button>";
  html += "<button class='game-btn' onclick='startGame(\"pong\")'>Pong Game</button>";
  html += "<button class='game-btn' onclick='startGame(\"tetris\")'>Tetris (Soon)</button>";
  html += "<h2>Display Tools</h2>";
  html += "<button class='game-btn info-btn' onclick='showDisplay(\"info\")'>System Info</button>";
  html += "<button class='game-btn paint-btn' onclick='startGame(\"paint\")'>Paint Tool</button>";
  html += "<button class='game-btn info-btn' onclick='showDisplay(\"menu\")'>Main Menu</button>";
  html += "<h2>Virtual Controls</h2>";
  html += "<div class='controls'>";
  html += "<div></div><button class='control-btn' onmousedown='sendControl(\"up\")' onmouseup='sendControl(\"release\")'>UP</button><div></div>";
  html += "<button class='control-btn' onmousedown='sendControl(\"left\")' onmouseup='sendControl(\"release\")'>LEFT</button>";
  html += "<button class='control-btn' onmousedown='sendControl(\"fire\")' onmouseup='sendControl(\"release\")'>FIRE</button>";
  html += "<button class='control-btn' onmousedown='sendControl(\"right\")' onmouseup='sendControl(\"release\")'>RIGHT</button>";
  html += "<div></div><button class='control-btn' onmousedown='sendControl(\"down\")' onmouseup='sendControl(\"release\")'>DOWN</button><div></div>";
  html += "</div>";
  html += "<h2>Quick Actions</h2>";
  html += "<button class='game-btn exit-btn' onclick='exitToMenu()'>Exit to Menu</button>";
  html += "<button class='game-btn exit-btn' onclick='refreshStatus()'>Refresh Status</button>";
  html += "</div>";
  html += "<script>";
  html += "function startGame(game) { fetch('/api/game', { method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({action: 'start', game: game}) }).then(response => response.json()).then(data => alert('Game started: ' + game)); }";
  html += "function showDisplay(mode) { fetch('/api/display', { method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({action: 'show', mode: mode}) }).then(response => response.json()).then(data => alert('Display mode: ' + mode)); }";
  html += "function sendControl(control) { fetch('/api/game', { method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({action: 'control', control: control}) }); }";
  html += "function exitToMenu() { fetch('/api/display', { method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({action: 'show', mode: 'menu'}) }).then(response => response.json()).then(data => alert('Returned to main menu')); }";
  html += "function refreshStatus() { fetch('/api/status').then(response => response.json()).then(data => { document.getElementById('status').innerHTML = '<h3>System Status</h3><p><strong>Current Mode:</strong> ' + data.mode + '</p><p><strong>WiFi:</strong> ' + data.wifi + '</p><p><strong>Uptime:</strong> ' + data.uptime + 's</p><p><strong>Free RAM:</strong> ' + data.freeRam + ' KB</p><p><strong>CPU Freq:</strong> ' + data.cpuFreq + ' MHz</p>'; }); }";
  html += "setInterval(refreshStatus, 5000); refreshStatus();";
  html += "</script></body></html>";
  
  server.send(200, "text/html", html);
}

void handleDisplayAPI() {
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    
    String action = doc["action"];
    String mode = doc["mode"];
    
    if (action == "show") {
      if (mode == "info") {
        currentMode = MODE_INFO;
        displaySystemInfo();
      } else if (mode == "menu") {
        currentMode = MODE_MENU;
        drawMenu();
      }
      
      server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Display mode changed to " + mode + "\"}");
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid request\"}");
  }
}

void handleGameAPI() {
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));
    
    String action = doc["action"];
    
    if (action == "start") {
      String game = doc["game"];
      if (game == "snake") {
        currentMode = MODE_SNAKE;
        initSnake();
      } else if (game == "pong") {
        currentMode = MODE_PONG;
        initPong();
      } else if (game == "tetris") {
        currentMode = MODE_TETRIS;
        initTetris();
      } else if (game == "paint") {
        currentMode = MODE_PAINT;
        initPaint();
      }
      server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Game started: " + game + "\"}");
    }
    else if (action == "control") {
      String control = doc["control"];
      // Simulate button presses from web interface
      if (control == "up") buttons.upPressed = true;
      else if (control == "down") buttons.downPressed = true;
      else if (control == "left") buttons.leftPressed = true;
      else if (control == "right") buttons.rightPressed = true;
      else if (control == "fire") buttons.firePressed = true;
      
      server.send(200, "application/json", "{\"status\":\"success\"}");
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid request\"}");
  }
}

void handleStatusAPI() {
  DynamicJsonDocument doc(1024);
  
  String modeStr;
  switch (currentMode) {
    case MODE_MENU: modeStr = "Menu"; break;
    case MODE_INFO: modeStr = "System Info"; break;
    case MODE_SNAKE: modeStr = "Snake Game"; break;
    case MODE_PONG: modeStr = "Pong Game"; break;
    case MODE_TETRIS: modeStr = "Tetris"; break;
    case MODE_PAINT: modeStr = "Paint Tool"; break;
  }
  
  doc["mode"] = modeStr;
  doc["wifi"] = (WiFi.status() == WL_CONNECTED) ? "Connected" : "Disconnected";
  doc["uptime"] = millis() / 1000;
  doc["freeRam"] = ESP.getFreeHeap() / 1024;
  doc["cpuFreq"] = ESP.getCpuFreqMHz();
  doc["ip"] = WiFi.localIP().toString();
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleNotFound() {
  server.send(404, "text/plain", "Page not found");
}
