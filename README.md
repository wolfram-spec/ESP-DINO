# 🦖 Chrome Dino Game for Arduino

A fully-featured recreation of Google Chrome's iconic offline dinosaur game for ESP8266/Arduino with SSD1306 OLED display!
## 📖 About This Project

> **Note:** This code was AI-generated through collaboration with Claude (Anthropic). I'm sharing it here for the community to enjoy and learn from! Feel free to use, modify, and improve it. All credit for the original Chrome Dino game goes to Google Chrome's team. 🎮

## ✨ Features

- **Smooth Jump Physics** - Natural gravity and velocity-based jumping
- **Duck Mechanic** - Avoid flying pterodactyls by ducking
- **Animated Sprites** - Running animation, wing flapping, and more
- **Multiple Obstacles** - Ground cacti and flying enemies
- **Progressive Difficulty** - Game speeds up and obstacles appear more frequently
- **Score Tracking** - Live score and high score persistence
- **Optimized Code** - Pre-calculated patterns and efficient rendering
- **Forgiving Hitboxes** - Slightly reduced collision areas for better gameplay

## 🛠️ Hardware Requirements

### Required Components
- **Microcontroller:** ESP8266 (NodeMCU, Wemos D1 Mini), ESP32, or Arduino
- **Display:** SSD1306 OLED (128x64 pixels, I2C interface)
- **Buttons:** 2x tactile push buttons
- **Resistors:** 2x 10kΩ pull-up resistors (if not using INPUT_PULLUP)
- **Wires:** Jumper wires for connections
- **Breadboard:** For prototyping (optional)

### Pin Configuration

| Component | Pin | ESP8266 | ESP32 | Arduino Uno |
|-----------|-----|---------|-------|-------------|
| OLED SDA | SDA | D2 (GPIO4) | GPIO21 | A4 |
| OLED SCL | SCL | D1 (GPIO5) | GPIO22 | A5 |
| Jump Button | Digital | D4 (GPIO2) | GPIO16 | Pin 4 |
| Duck Button | Digital | D6 (GPIO12) | GPIO17 | Pin 6 |
| OLED VCC | Power | 3.3V | 3.3V | 5V |
| OLED GND | Ground | GND | GND | GND |

> **Note for ESP32 Users:** You'll need to modify the button pin definitions in the code:
> ```cpp
> #define JUMP_BUTTON 16  // Change from D4 to 16
> #define DOWN_BUTTON 17  // Change from D6 to 17
> ```

## 📦 Software Requirements

### Arduino Libraries
Install these libraries via Arduino IDE Library Manager:

1. **Wire** (Built-in)
2. **Adafruit GFX Library** by Adafruit
3. **Adafruit SSD1306** by Adafruit

### Installation Steps

1. **Install Arduino IDE**
   - Download from [arduino.cc](https://www.arduino.cc/en/software)

2. **Install ESP8266/ESP32 Board Support** (if using ESP8266/ESP32)
   
   **For ESP8266:**
   - Go to `File` → `Preferences`
   - Add to Additional Board Manager URLs:
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - Go to `Tools` → `Board` → `Board Manager`
   - Search for "ESP8266" and install
   
   **For ESP32:**
   - Go to `File` → `Preferences`
   - Add to Additional Board Manager URLs:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to `Tools` → `Board` → `Board Manager`
   - Search for "ESP32" and install

3. **Install Required Libraries**
   - Go to `Sketch` → `Include Library` → `Manage Libraries`
   - Search and install:
     - "Adafruit GFX Library"
     - "Adafruit SSD1306"

4. **Upload the Code**
   - Open the `.ino` file in Arduino IDE
   - **Important:** If using ESP32, modify the pin definitions:
     ```cpp
     #define JUMP_BUTTON 16  // GPIO16 for ESP32
     #define DOWN_BUTTON 17  // GPIO17 for ESP32
     ```
   - Select your board: `Tools` → `Board` → Select your ESP8266/ESP32/Arduino
   - Select the port: `Tools` → `Port` → Select your device
   - Click Upload ⬆️

## 🔌 Wiring Diagram

### ESP8266 (NodeMCU) Wiring

```
OLED Display          ESP8266
------------          -------
VCC         -------→  3.3V
GND         -------→  GND
SDA         -------→  D2 (GPIO4)
SCL         -------→  D1 (GPIO5)

Jump Button          ESP8266
-----------          -------
One side    -------→  D4 (GPIO2)
Other side  -------→  GND

Duck Button          ESP8266
-----------          -------
One side    -------→  D6 (GPIO12)
Other side  -------→  GND
```

### ESP32 Wiring

```
OLED Display          ESP32
------------          -----
VCC         -------→  3.3V
GND         -------→  GND
SDA         -------→  GPIO21
SCL         -------→  GPIO22

Jump Button          ESP32
-----------          -----
One side    -------→  GPIO16
Other side  -------→  GND

Duck Button          ESP32
-----------          -----
One side    -------→  GPIO17
Other side  -------→  GND
```

### Arduino Uno Wiring

```
OLED Display          Arduino Uno
------------          -----------
VCC         -------→  5V
GND         -------→  GND
SDA         -------→  A4
SCL         -------→  A5

Buttons connected to Pin 4 and Pin 6 with GND
```

## 🎮 How to Play

1. **Start Game:** Press the Jump button
2. **Jump:** Press the Jump button (avoid ground obstacles)
3. **Duck:** Hold the Duck button (avoid flying obstacles)
4. **Goal:** Survive as long as possible and beat your high score!

### Game Mechanics

- **Score** increases automatically over time
- **Speed** increases every 100 points
- **Obstacles** appear more frequently as you progress
- **Flying obstacles** appear at mid-height - duck to avoid them
- **Ground cacti** block your path - jump over them
- **Game Over** when you collide with any obstacle

## ⚙️ Configuration & Tuning

You can customize the game by modifying constants at the top of the code:

```cpp
// Physics tuning
#define JUMP_VELOCITY -7.5f      // Jump strength
#define GRAVITY 0.7f              // Gravity force
#define MAX_FALL_SPEED 8.0f      // Terminal velocity

// Difficulty tuning
#define INITIAL_GAME_SPEED 3.0f  // Starting speed
#define MAX_GAME_SPEED 12.0f     // Maximum speed
#define SPEED_INCREMENT 0.1f     // Speed increase rate

// Obstacle tuning
#define CACTUS_SPAWN_WEIGHT 70   // 70% cactus, 30% flying
#define MIN_OBSTACLE_SPACING 70  // Minimum gap between obstacles
```

## 🐛 Troubleshooting

### Display Not Working
- Check I2C address (usually 0x3C, sometimes 0x3D)
- Verify wiring connections
- Try scanning I2C devices with an I2C scanner sketch

### Compilation Errors
- Ensure all libraries are installed correctly
- Check board selection matches your hardware
- Update Arduino IDE to latest version

### Game Too Fast/Slow
- Adjust `INITIAL_GAME_SPEED` constant
- Modify `SPEED_INCREMENT` for difficulty curve

### Buttons Not Responding
- Verify button connections to correct pins
- Check if buttons are normally open (NO)
- Test with a simple button sketch first

## 📝 Code Structure

```
├── Constants & Configuration
│   ├── Display settings
│   ├── Physics constants
│   ├── Game tuning parameters
│   └── Pin definitions
│
├── Game Variables
│   ├── Score tracking
│   ├── Physics state
│   └── Timing variables
│
├── Sprite Data (PROGMEM)
│   ├── Dino animations
│   ├── Obstacle sprites
│   └── UI elements
│
└── Functions
    ├── setup() - Initialize hardware
    ├── loop() - Main game loop
    ├── Physics System
    │   ├── updatePhysics()
    │   └── updateScore()
    ├── Game Logic
    │   ├── updateOb
