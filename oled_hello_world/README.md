# ESP32 OLED Web Gaming & Display Controller

A comprehensive Arduino project that combines an ESP32 with an SSD1315 OLED display to create an interactive gaming and display system with web interface control.

## 🎮 Overview

This project transforms your ESP32 and OLED display into a complete gaming and information system featuring:
- **3 Interactive Games**: Snake, Pong, and Tetris (coming soon)
- **Web Interface**: Complete control via modern web portal
- **5-Button Control**: Physical button interface for games
- **System Information Display**: Real-time ESP32 stats
- **Paint Tool**: Creative drawing application
- **WiFi Connectivity**: Remote control and monitoring

## 🔧 Hardware Requirements

- **ESP32 Development Board** (ESP32-WROOM-32 or similar)
- **SSD1315 OLED Display** (0.96" 128x64 pixels, I2C interface)
- **5 Push Buttons** (for game controls)
- **Breadboard and Jumper Wires**
- **Pull-up Resistors** (10kΩ, or use internal pull-ups)
- **USB Cable** for programming

## 📊 Pin Connections

### OLED Display
| OLED Display | ESP32 Pin | Description |
|--------------|-----------|-------------|
| VCC          | 3.3V      | Power supply |
| GND          | GND       | Ground |
| SDA          | GPIO 21   | I2C Data line |
| SCL          | GPIO 22   | I2C Clock line |

### Game Control Buttons
| Button | ESP32 Pin | Function |
|--------|-----------|----------|
| UP     | GPIO 32   | Move up/Menu up |
| DOWN   | GPIO 33   | Move down/Menu down |
| LEFT   | GPIO 25   | Move left |
| RIGHT  | GPIO 26   | Move right |
| FIRE   | GPIO 27   | Action/Select/Fire |

**Note**: All buttons use internal pull-up resistors. Connect one side to the GPIO pin and other side to GND.

## 📚 Required Libraries

Install these libraries through the Arduino IDE Library Manager:

1. **Adafruit SSD1306** by Adafruit
2. **Adafruit GFX Library** by Adafruit
3. **Wire** (usually pre-installed with ESP32 board package)

### Installation Steps:
1. Open Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries**
3. Search for "Adafruit SSD1306" and install
4. Search for "Adafruit GFX" and install
5. Restart Arduino IDE

## 🎯 Features

### 🎮 Games
- **Snake Game**: Classic snake with food collection and scoring
- **Pong Game**: Player vs AI paddle game
- **Tetris**: Coming soon with full block mechanics
- **Paint Tool**: Pixel-perfect drawing application

### 🌐 Web Interface
- **Modern Web Portal**: Responsive design for all devices
- **Virtual Controls**: Play games directly from web browser
- **Real-time Status**: Live system monitoring and statistics
- **Remote Game Launch**: Start any game from web interface
- **Quick Actions**: Exit games, refresh status, return to menu

### 🖥️ Display Features
- **Interactive Menu System**: Navigate with physical buttons
- **System Information**: CPU frequency, RAM usage, WiFi status
- **Real-time Updates**: Live data refresh and game states
- **Multiple Display Modes**: Seamless switching between functions

## 💻 Setup Instructions

### 1. Hardware Assembly

**OLED Display Connection:**
- Connect according to pin connections table above
- Ensure stable 3.3V power supply

**Button Setup:**
- Connect 5 push buttons to GPIO pins as specified
- Each button: One side to GPIO pin, other side to GND
- No external resistors needed (uses internal pull-ups)

**Breadboard Layout:**
```
ESP32          Breadboard          OLED
GPIO32  -----> Button UP    -----> [Button]
GPIO33  -----> Button DOWN  -----> [Button]
GPIO25  -----> Button LEFT  -----> [Button]
GPIO26  -----> Button RIGHT -----> [Button]
GPIO27  -----> Button FIRE  -----> [Button]
GPIO21  -----> SDA Line     -----> SDA
GPIO22  -----> SCL Line     -----> SCL
3.3V    -----> Power Rail   -----> VCC
GND     -----> Ground Rail  -----> GND
```

### 2. Software Setup
- Install required libraries (see above)
- Update WiFi credentials in code if different from "Sukrit_wifi"
- Open `oled_hello_world.ino` in Arduino IDE
- Select ESP32 board and COM port
- Upload the code

### 3. Operation

**Physical Controls:**
- Use buttons to navigate menu and play games
- UP/DOWN: Navigate menu items
- LEFT/RIGHT: Game movement
- FIRE: Select/Action button

**Web Interface:**
- Connect to same WiFi network as ESP32
- Open web browser and navigate to ESP32's IP address
- Use virtual controls or launch games remotely
- Monitor system status in real-time

**Game Controls:**
- **Snake**: Arrow buttons to change direction, avoid walls and self
- **Pong**: UP/DOWN to move paddle, FIRE to exit
- **Paint Tool**: Arrow buttons to move cursor, FIRE to draw
- **All Games**: FIRE button to return to main menu

## 🔍 Code Structure

- `setup()`: Initializes serial communication and OLED display
- `loop()`: Cycles through different display modes
- `displayHelloWorld()`: Shows the classic Hello World message
- `displayDateTime()`: Shows current date and runtime
- `displayESP32Info()`: Displays ESP32 system information
- `displayAnimatedText()`: Shows scrolling and blinking text animations

## 🛠️ Troubleshooting

### Display Not Working:
- Check all wiring connections
- Verify OLED display address (0x3C or 0x3D)
- Ensure sufficient power supply (3.3V)

### Compilation Errors:
- Make sure all required libraries are installed
- Check ESP32 board package is installed
- Verify correct board selection in Arduino IDE

### Display Address Issues:
```cpp
// Try different addresses if display doesn't work
#define SCREEN_ADDRESS 0x3C  // Most common for 128x64
// or
#define SCREEN_ADDRESS 0x3D  // Alternative address
```

## 📈 Possible Extensions

- **Temperature Sensor**: Add DS18B20 or DHT22 for temperature display
- **WiFi Status**: Show WiFi connection status and IP address
- **Clock**: Implement real-time clock with NTP synchronization
- **Menu System**: Create interactive menu with button navigation
- **Graphics**: Add bitmap images or custom graphics
- **Data Logging**: Display sensor data from other projects

## 🔗 Related Projects

This project complements other sensors in the ESP32-IOT-Sensor-Dashboard:
- Light sensor data visualization
- PIR sensor status display
- RGB LED status indication

## 📝 Serial Output

The project also outputs status information to the serial monitor:
- Initialization status
- Error messages (if any)
- Debug information

**Serial Monitor Settings**: 115200 baud rate

## 🎯 Learning Outcomes

After completing this project, you'll understand:
- I2C communication with ESP32
- OLED display programming
- Text rendering and positioning
- Simple animations and effects
- ESP32 system information access

## 📄 License

This project is open source and available under the MIT License.

## 🤝 Contributing

Feel free to fork this project and submit pull requests for improvements or additional features.

---

**Happy Coding! 🚀**
