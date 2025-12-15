#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  // Reset pin not used on most I2C displays
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button pins
#define JUMP_BUTTON D4//-(For ESP 8266) & 16 -(For ESP32)
#define DOWN_BUTTON D6//-(For ESP 8266) & 17 -(For ESP32)

// Game constants
#define DINO_WIDTH 18
#define DINO_HEIGHT 21
#define DINO_DUCK_WIDTH 23
#define DINO_DUCK_HEIGHT 13
#define DINO_START_X 20
#define GROUND_Y 63
#define JUMP_HEIGHT 35  // Increased from 27 to 35 for higher jumps
#define MIN_JUMP_Y 16   // Force Y to stay in 16-63 range as requested
#define DINO_GROUND_Y (GROUND_Y - DINO_HEIGHT)
#define DINO_DUCK_Y (GROUND_Y - DINO_DUCK_HEIGHT)  // Y position when ducking
#define ANIMATION_DELAY 200
#define CACTUS_WIDTH 18
#define CACTUS_HEIGHT 21
#define FLYING_OBSTACLE_WIDTH 28
#define FLYING_OBSTACLE_HEIGHT 21
// Flying obstacles need to be at a height where ducking can avoid them
// Make sure the ducking dino can go under the flying obstacles
#define FLYING_MIN_Y 25  // Ensures flying obstacles appear in the correct range
#define FLYING_MAX_Y 30  // Lowered to ensure obstacles are positioned where they can be ducked under
#define SCORE_UPDATE_RATE 100  // milliseconds between score increments
#define SURFACE_HEIGHT 1

// Game variables
int score = 0;
int highScore = 0;
int dinoY;
float jumpVelocity = 0;  // Changed to float for more precise control
bool isJumping = false;
bool isDucking = false;
bool gameOver = true;
bool gameStarted = false;
bool legPosition = false;  // Alternates between front and back leg up
bool duckPosition = false; // Alternates between left and right duck frames
unsigned long lastLegSwitch = 0;
unsigned long lastScoreUpdate = 0;
unsigned long lastObstacleGeneration = 0;
unsigned long lastFlyingAnimation = 0;
bool flyingObstacleFrame = false;  // Alternates between up and down wings
unsigned long jumpStartTime = 0;  // Track when jump started for duration control
float jumpGravity = 0.5;  // Lower value for gentler gravity (was 1)

// Obstacle struct
struct Obstacle {
  int x;
  int y;
  int width;
  int height;
  byte type;  // 0 = none, 1 = cactus_1, 2 = cactus_2, 3 = flying
  bool active;
};

// ' back_leg_up', 18x21px
const unsigned char epd_bitmap__back_leg_up [] PROGMEM = {
	0x00, 0x3f, 0x80, 0x00, 0x7f, 0xc0, 0x00, 0x6f, 0xc0, 0x00, 0x7f, 0xc0, 0x00, 0x7f, 0xc0, 0x00, 
	0x7c, 0x00, 0x00, 0x7f, 0x80, 0x00, 0xf8, 0x00, 0x83, 0xf8, 0x00, 0x87, 0xfe, 0x00, 0xcf, 0xfa, 
	0x00, 0xff, 0xf8, 0x00, 0xff, 0xf8, 0x00, 0xff, 0xf0, 0x00, 0x7f, 0xf0, 0x00, 0x3f, 0xe0, 0x00, 
	0x1f, 0xc0, 0x00, 0x0e, 0xc0, 0x00, 0x0c, 0x40, 0x00, 0x06, 0x40, 0x00, 0x00, 0x60, 0x00
};
// ' cactus_1', 18x21px
const unsigned char epd_bitmap__cactus_1 [] PROGMEM = {
	0x06, 0x07, 0xc0, 0x0f, 0x07, 0xc0, 0x0f, 0x07, 0xc0, 0x0f, 0x07, 0xc0, 0x0f, 0x17, 0xc0, 0x0f, 
	0x3f, 0xc0, 0x0f, 0x3f, 0xc0, 0xcf, 0x3f, 0xc0, 0x4f, 0x3f, 0xc0, 0x6f, 0x3f, 0xc0, 0x3f, 0x3f, 
	0xc0, 0x1f, 0x3f, 0xc0, 0x0f, 0x3f, 0xc0, 0x0f, 0x3f, 0xc0, 0x0f, 0xff, 0xc0, 0x0f, 0xf7, 0xc0, 
	0x0f, 0xe7, 0xc0, 0x0f, 0x07, 0xc0, 0x0f, 0x07, 0xc0, 0x0f, 0x07, 0xc0, 0x0f, 0x07, 0xc0
};
// ' cactus_2', 18x21px
const unsigned char epd_bitmap__cactus_2 [] PROGMEM = {
	0x06, 0x00, 0x00, 0x4f, 0x00, 0x00, 0xef, 0x00, 0x00, 0xef, 0x20, 0x00, 0xef, 0x70, 0x00, 0xef, 
	0x70, 0x00, 0xef, 0x70, 0x00, 0xef, 0x70, 0x00, 0xef, 0x70, 0x00, 0xef, 0x70, 0x00, 0xef, 0x70, 
	0x00, 0xef, 0x70, 0x00, 0xef, 0xe0, 0x00, 0xef, 0xc2, 0x00, 0xef, 0x07, 0x00, 0xef, 0x07, 0x00, 
	0x7f, 0x17, 0x00, 0x3f, 0x17, 0x00, 0x0f, 0x17, 0x00, 0x0f, 0x0f, 0x40, 0x0f, 0x07, 0x80
};
// ' front_leg_up', 18x21px
const unsigned char epd_bitmap__front_leg_up [] PROGMEM = {
	0x00, 0x3f, 0x80, 0x00, 0x7f, 0xc0, 0x00, 0x6f, 0xc0, 0x00, 0x7f, 0xc0, 0x00, 0x7f, 0xc0, 0x00, 
	0x7c, 0x00, 0x00, 0x7f, 0x80, 0x00, 0xf8, 0x00, 0x83, 0xf8, 0x00, 0x87, 0xfe, 0x00, 0xcf, 0xfa, 
	0x00, 0xff, 0xf8, 0x00, 0xff, 0xf8, 0x00, 0xff, 0xf0, 0x00, 0x7f, 0xf0, 0x00, 0x3f, 0xe0, 0x00, 
	0x1f, 0xc0, 0x00, 0x0e, 0xc0, 0x00, 0x0c, 0x60, 0x00, 0x08, 0x00, 0x00, 0x0c, 0x00, 0x00
};
// 'line', 18x21px
const unsigned char epd_bitmap_line [] PROGMEM = {
	0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0xff, 
	0xff, 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 
	0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 
	0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xc0
};
// ' still_jump', 18x21px
const unsigned char epd_bitmap__still_jump [] PROGMEM = {
	0x00, 0x3f, 0x80, 0x00, 0x7f, 0xc0, 0x00, 0x6f, 0xc0, 0x00, 0x7f, 0xc0, 0x00, 0x7f, 0xc0, 0x00, 
	0x7c, 0x00, 0x00, 0x7f, 0x80, 0x00, 0xf8, 0x00, 0x83, 0xf8, 0x00, 0x87, 0xfe, 0x00, 0xcf, 0xfa, 
	0x00, 0xff, 0xf8, 0x00, 0xff, 0xf8, 0x00, 0xff, 0xf0, 0x00, 0x7f, 0xf0, 0x00, 0x3f, 0xe0, 0x00, 
	0x1f, 0xc0, 0x00, 0x0e, 0xc0, 0x00, 0x0c, 0x40, 0x00, 0x08, 0x40, 0x00, 0x0c, 0x60, 0x00
};
// ' flying_obstc_down', 28x21px
const unsigned char epd_bitmap__flying_obstc_down [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 
	0x07, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x1d, 0x80, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 
	0x7f, 0xff, 0xc0, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x00, 0xff, 0xff, 0xf0, 0x00, 0x7f, 0xff, 0x00, 
	0x00, 0x3f, 0xff, 0xc0, 0x00, 0x1f, 0xff, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 
	0x00, 0x1e, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 
	0x00, 0x10, 0x00, 0x00
};
// ' flying_obstc_up', 28x21px
const unsigned char epd_bitmap__flying_obstc_up [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 
	0x03, 0x3c, 0x00, 0x00, 0x07, 0x3e, 0x00, 0x00, 0x0f, 0x3f, 0x00, 0x00, 0x1d, 0xbf, 0x80, 0x00, 
	0x3f, 0xbf, 0xc0, 0x00, 0x7f, 0xff, 0xc0, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x00, 0xff, 0xff, 0xf0, 
	0x00, 0x7f, 0xff, 0x00, 0x00, 0x3f, 0xff, 0xc0, 0x00, 0x1f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00
};
// ' dino_left', 23x13px
const unsigned char epd_bitmap__dino_left [] PROGMEM = {
	0x00, 0x01, 0xfc, 0x80, 0x03, 0xfe, 0xc0, 0x07, 0x7e, 0xe3, 0xcf, 0xfe, 0xff, 0xff, 0xfe, 0x7f, 
	0xff, 0xc0, 0x3f, 0xff, 0xf8, 0x1f, 0xfe, 0x00, 0x0f, 0xe4, 0x00, 0x0f, 0xc6, 0x00, 0x08, 0x80, 
	0x00, 0x0c, 0x80, 0x00, 0x00, 0xc0, 0x00
};
// ' dino_right', 23x13px
const unsigned char epd_bitmap__dino_right [] PROGMEM = {
	0x00, 0x01, 0xfc, 0x80, 0x03, 0xfe, 0xc0, 0x07, 0x7e, 0xe7, 0xcf, 0xfe, 0xff, 0xff, 0xfe, 0x7f, 
	0xff, 0xc0, 0x3f, 0xff, 0xf8, 0x1f, 0xfe, 0x00, 0x0f, 0xe4, 0x00, 0x0f, 0xc6, 0x00, 0x08, 0x80, 
	0x00, 0x08, 0xc0, 0x00, 0x0c, 0x00, 0x00
};

// ' dino_genda', 53x7px
const unsigned char epd_bitmap__dino_genda [] PROGMEM = {
	0xf3, 0xa2, 0x70, 0x7b, 0xe8, 0xbc, 0x70, 0x89, 0x32, 0x88, 0x82, 0x0c, 0xa2, 0x88, 0x89, 0x2a, 
	0x88, 0x9b, 0x8a, 0xa2, 0xf8, 0x89, 0x26, 0x88, 0x8a, 0x09, 0xa2, 0x88, 0x89, 0x22, 0x88, 0x8a, 
	0x08, 0xa2, 0x88, 0x89, 0x22, 0x88, 0x8a, 0x08, 0xa2, 0x88, 0xf3, 0xa2, 0x70, 0x73, 0xe8, 0xbc, 
	0x88
};
// ' game_over', 49x7px
const unsigned char epd_bitmap__game_over [] PROGMEM = {
	0x79, 0xc8, 0xbe, 0x1c, 0x8b, 0xef, 0x00, 0x82, 0x2d, 0xa0, 0x22, 0x8a, 0x08, 0x80, 0x9b, 0xea, 
	0xb8, 0x22, 0x8b, 0x8f, 0x00, 0x8a, 0x28, 0xa0, 0x22, 0x8a, 0x08, 0x80, 0x8a, 0x28, 0xa0, 0x22, 
	0x52, 0x08, 0x80, 0x8a, 0x28, 0xa0, 0x22, 0x52, 0x08, 0x80, 0x72, 0x28, 0xbe, 0x1c, 0x23, 0xe8, 
	0x80
};

#define MAX_OBSTACLES 3
Obstacle obstacles[MAX_OBSTACLES];

// Game speed
int gameSpeed = 3;
unsigned long obstacleGenerationTime = 2000;  // Start with 2 seconds between obstacles

void setup() {
  Serial.begin(115200);
  
  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);  // Don't proceed, loop forever
  }
  
  // Initialize display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();
  
  // Initialize buttons
  pinMode(JUMP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);
  
  // Initialize game state
  resetGame();
}

void loop() {
  // Check for button press with debounce
  static unsigned long lastJumpTime = 0;
  unsigned long currentTime = millis();
  
  if (digitalRead(JUMP_BUTTON) == LOW && currentTime - lastJumpTime > 50) {
    lastJumpTime = currentTime;
    
    if (gameOver) {
      resetGame();
      gameOver = false;
      gameStarted = true;
    } else if (!isJumping && !isDucking) {
      startJump();
    }
  }
  
  // Check for down button
  if (!gameOver && gameStarted) {
    if (digitalRead(DOWN_BUTTON) == LOW) {
      isDucking = true;
    } else {
      isDucking = false;
    }
  }
  
  // Update game state
  if (!gameOver && gameStarted) {
    unsigned long currentTime = millis();
    
    // Increase score
    if (currentTime - lastScoreUpdate > SCORE_UPDATE_RATE) {
      score++;
      lastScoreUpdate = currentTime;
      
      // Increase game speed based on score
      if (score % 100 == 0) {
        gameSpeed += 0.5;
      }
    }
    
    // Update dino position if jumping
    if (isJumping) {
      updateJump();
    }
    
    // Update leg animation
    if (currentTime - lastLegSwitch > ANIMATION_DELAY) {
      legPosition = !legPosition;
      duckPosition = !duckPosition;
      lastLegSwitch = currentTime;
    }
    
    // Update flying obstacle animation
    if (currentTime - lastFlyingAnimation > ANIMATION_DELAY) {
      flyingObstacleFrame = !flyingObstacleFrame;
      lastFlyingAnimation = currentTime;
    }
    
    // Generate new obstacles
    if (currentTime - lastObstacleGeneration > obstacleGenerationTime) {
      generateObstacle();
      lastObstacleGeneration = currentTime;
      
      // Gradually increase game speed and decrease obstacle generation time
      if (gameSpeed < 12) {  // Increased max speed
        gameSpeed += 0.1;
      }
      if (obstacleGenerationTime > 800) {  // Lower minimum time between obstacles
        obstacleGenerationTime -= 50;
      }
    }
    
    // Move and check obstacles
    moveAndCheckObstacles();
  }
  
  // Draw everything
  drawGame();
}

void resetGame() {
  gameOver = true;
  gameStarted = false;
  isJumping = false;
  isDucking = false;
  dinoY = DINO_GROUND_Y;
  score = 0;
  gameSpeed = 3;
  obstacleGenerationTime = 2000;
  
  // Clear all obstacles
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    obstacles[i].active = false;
  }
}

void startJump() {
  if (!isJumping) {
    isJumping = true;
    jumpVelocity = -9.0;  // Increased from -8.5 to -9.0 for stronger initial jump
    jumpStartTime = millis();  // Record when the jump started
  }
}

void updateJump() {
  // Apply gravity (decreased for more hang time)
  jumpVelocity += jumpGravity;
  
  // Update position
  dinoY += jumpVelocity;
  
  // Check if we've reached the ground
  if (dinoY >= DINO_GROUND_Y) {
    dinoY = DINO_GROUND_Y;
    isJumping = false;
    jumpVelocity = 0;
  }
  
  // Check if we've reached the highest point allowed (16 as requested)
  if (dinoY < MIN_JUMP_Y) {
    dinoY = MIN_JUMP_Y;
    jumpVelocity = jumpVelocity * 0.5;  // Reduce velocity but don't stop it entirely for smoother peak
  }
  
  // Add jump duration limit to prevent "floating" too long
  if (millis() - jumpStartTime > 1000) {  // Limit jump to 1 second max
    if (jumpVelocity < 2) {  // If we're going up or barely coming down
      jumpVelocity = 2;  // Force faster descent
    }
  }
}

void generateObstacle() {
  // Find a free obstacle slot
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (!obstacles[i].active) {
      obstacles[i].active = true;
      obstacles[i].x = SCREEN_WIDTH;
      
      // Add minimum spacing between obstacles based on game speed
      // Check if any other obstacle is too close
      bool tooClose = false;
      int minSpacing = 60 + (gameSpeed * 5); // Dynamic spacing based on speed
      
      for (int j = 0; j < MAX_OBSTACLES; j++) {
        if (j != i && obstacles[j].active) {
          if (obstacles[j].x > SCREEN_WIDTH - minSpacing) {
            tooClose = true;
            break;
          }
        }
      }
      
      if (tooClose) {
        obstacles[i].active = false; // Cancel this obstacle generation
        return;
      }
      
      // Decide obstacle type (70% cactus, 30% flying)
      int obstacleType = random(10);
      
      if (obstacleType < 7) {
        // Cactus
        obstacles[i].type = random(1, 3);  // 1 or 2 (cactus_1 or cactus_2)
        obstacles[i].width = CACTUS_WIDTH;
        obstacles[i].height = CACTUS_HEIGHT;
        // Position cactus on the ground, making sure it can be jumped over
        obstacles[i].y = GROUND_Y - CACTUS_HEIGHT;
      } else {
        // Flying obstacle
        obstacles[i].type = 3;  // 3 = flying
        obstacles[i].width = FLYING_OBSTACLE_WIDTH;
        obstacles[i].height = FLYING_OBSTACLE_HEIGHT;
        
      // Position flying obstacle at a height where it can be ducked under
        obstacles[i].y = random(FLYING_MIN_Y, FLYING_MAX_Y);
        
        // Make sure the flying obstacle can actually be ducked under
        // The distance between the bottom of the flying obstacle and the ground
        // must be greater than the height of the ducking dino plus some clearance
        int bottomOfObstacle = obstacles[i].y + FLYING_OBSTACLE_HEIGHT;
        int requiredClearance = DINO_DUCK_HEIGHT + 3; // 3 pixels of extra clearance
        
        if (GROUND_Y - bottomOfObstacle < requiredClearance) {
          // Adjust the obstacle position upward to ensure it can be ducked under
          obstacles[i].y = GROUND_Y - requiredClearance - FLYING_OBSTACLE_HEIGHT;
        }
      }
      
      break;
    }
  }
}

void moveAndCheckObstacles() {
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obstacles[i].active) {
      // Move obstacle
      obstacles[i].x -= gameSpeed;
      
      // Check if obstacle is off-screen
      if (obstacles[i].x + obstacles[i].width < 0) {
        obstacles[i].active = false;
      }
      
      // Check for collision with dino
      if (checkCollision(i)) {
        gameOver = true;
        if (score > highScore) {
          highScore = score;
        }
      }
    }
  }
}

bool checkCollision(int obstacleIndex) {
  // Dino hitbox (reduce size slightly for more forgiving gameplay)
  int dinoHitboxX = DINO_START_X + 3;  // Moved hitbox right by 1 more pixel
  int dinoHitboxY = dinoY + 3;         // Moved hitbox down by 1 more pixel
  int dinoHitboxWidth, dinoHitboxHeight;
  
  if (isDucking) {
    dinoHitboxWidth = DINO_DUCK_WIDTH - 6;  // Reduced width by 2 more pixels
    dinoHitboxHeight = DINO_DUCK_HEIGHT - 6; // Reduced height by 2 more pixels
  } else {
    dinoHitboxWidth = DINO_WIDTH - 6;      // Reduced width by 2 more pixels
    dinoHitboxHeight = DINO_HEIGHT - 6;    // Reduced height by 2 more pixels
  }
  
  // Obstacle hitbox - also made more forgiving
  int obstacleHitboxX = obstacles[obstacleIndex].x + 3;
  int obstacleHitboxY = obstacles[obstacleIndex].y + 3;
  int obstacleHitboxWidth = obstacles[obstacleIndex].width - 6;
  int obstacleHitboxHeight = obstacles[obstacleIndex].height - 6;
  
  // Check for overlap
  return (dinoHitboxX < obstacleHitboxX + obstacleHitboxWidth &&
          dinoHitboxX + dinoHitboxWidth > obstacleHitboxX &&
          dinoHitboxY < obstacleHitboxY + obstacleHitboxHeight &&
          dinoHitboxY + dinoHitboxHeight > obstacleHitboxY);
}

void drawGame() {
  display.clearDisplay();
  
  // Draw score and high score
  display.setCursor(0, 0);
  display.print("HI: ");
  display.print(highScore);
  
  display.setCursor(60, 0);
  display.print("SCORE: ");
  display.print(score);
  
  // Draw enhanced ground line - add some variation
  for (int x = 0; x < SCREEN_WIDTH; x++) {
    // Main ground line
    display.drawPixel(x, GROUND_Y, SSD1306_WHITE);
    
    // Add some random variation to make it look more like terrain
    if (x % 20 == 0 || x % 17 == 0) {
      display.drawPixel(x, GROUND_Y - 1, SSD1306_WHITE);
    }
    if (x % 43 == 0 || x % 31 == 0) {
      display.drawPixel(x, GROUND_Y - 2, SSD1306_WHITE);
    }
  }
  
  // Draw dino
  if (gameStarted && !gameOver) {
    if (isDucking) {
      // Draw ducking dino
      int duckY = DINO_DUCK_Y;
      if (duckPosition) {
        display.drawBitmap(DINO_START_X, duckY, epd_bitmap__dino_left, DINO_DUCK_WIDTH, DINO_DUCK_HEIGHT, SSD1306_WHITE);
      } else {
        display.drawBitmap(DINO_START_X, duckY, epd_bitmap__dino_right, DINO_DUCK_WIDTH, DINO_DUCK_HEIGHT, SSD1306_WHITE);
      }
    } else if (isJumping) {
      // Draw jumping dino
      display.drawBitmap(DINO_START_X, dinoY, epd_bitmap__still_jump, DINO_WIDTH, DINO_HEIGHT, SSD1306_WHITE);
    } else {
      // Draw running dino
      if (legPosition) {
        display.drawBitmap(DINO_START_X, dinoY, epd_bitmap__back_leg_up, DINO_WIDTH, DINO_HEIGHT, SSD1306_WHITE);
      } else {
        display.drawBitmap(DINO_START_X, dinoY, epd_bitmap__front_leg_up, DINO_WIDTH, DINO_HEIGHT, SSD1306_WHITE);
      }
    }
  } else {
    // Show still dino at the beginning
    display.drawBitmap(DINO_START_X, DINO_GROUND_Y, epd_bitmap__still_jump, DINO_WIDTH, DINO_HEIGHT, SSD1306_WHITE);
  }
  
  // Draw obstacles
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obstacles[i].active) {
      switch (obstacles[i].type) {
        case 1:
          display.drawBitmap(obstacles[i].x, obstacles[i].y, epd_bitmap__cactus_1, CACTUS_WIDTH, CACTUS_HEIGHT, SSD1306_WHITE);
          break;
        case 2:
          display.drawBitmap(obstacles[i].x, obstacles[i].y, epd_bitmap__cactus_2, CACTUS_WIDTH, CACTUS_HEIGHT, SSD1306_WHITE);
          break;
        case 3:
          if (flyingObstacleFrame) {
            display.drawBitmap(obstacles[i].x, obstacles[i].y, epd_bitmap__flying_obstc_up, FLYING_OBSTACLE_WIDTH, FLYING_OBSTACLE_HEIGHT, SSD1306_WHITE);
          } else {
            display.drawBitmap(obstacles[i].x, obstacles[i].y, epd_bitmap__flying_obstc_down, FLYING_OBSTACLE_WIDTH, FLYING_OBSTACLE_HEIGHT, SSD1306_WHITE);
          }
          break;
      }
    }
  }
  
  // Show game over or start message
  if (gameOver) {
    if (gameStarted) {
      display.drawBitmap(38, 17, epd_bitmap__game_over, 49, 7, WHITE);
    } else {
      display.drawBitmap(35, 17, epd_bitmap__dino_genda, 53, 7, WHITE);
    }
  }
  
  display.display();
}