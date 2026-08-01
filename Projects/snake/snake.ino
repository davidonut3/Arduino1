#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 10

#define LED_GREEN 5
#define LED_RED 6

#define BUZZER 19
#define BUTTON_POWER 2
#define BUTTON_MENU 3
#define BUTTON_LEFT 4
#define BUTTON_RIGHT 7
#define BUTTON_SOURCE 8

#define POWER 0
#define MENU 1
#define LEFT 2
#define RIGHT 3
#define SOURCE 4

uint8_t prevInputs = 0;
uint8_t inputs = 0;

uint16_t previousMillis = 0;
uint16_t currentMillis = 0;
const uint16_t delayMillis = 350;

bool gameOver = true;
uint16_t score = 0;

int xPos = 31;
int yPos = 1;

int xApple = 0;
int yApple = 0;

int direction = 0;

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

uint8_t buffer[32] = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

const uint8_t gameOverScreen[32] PROGMEM = {
  0x3e, 0x22, 0x3a, 0x0, 0x3e, 0xa, 0x3e, 0x0, 0x3e, 0x4, 0x3e, 0x0, 0x3e, 0x2a, 0x2a, 0x0, 0x0, 0x3e, 0x22, 0x3e, 0x0, 0x1e, 0x20, 0x1e, 0x0, 0x3e, 0x2a, 0x2a, 0x0, 0x3e, 0xa, 0x36,
};

void loadInputs() {
  uint8_t tempInputs;

  bitWrite(tempInputs, POWER, !digitalRead(BUTTON_POWER));
  bitWrite(tempInputs, MENU, !digitalRead(BUTTON_MENU));
  bitWrite(tempInputs, LEFT, !digitalRead(BUTTON_LEFT));
  bitWrite(tempInputs, RIGHT, !digitalRead(BUTTON_RIGHT));
  bitWrite(tempInputs, SOURCE, !digitalRead(BUTTON_SOURCE));

  if (bitRead(tempInputs, POWER) != 0 && bitRead(prevInputs, POWER) == 0) {
    bitSet(inputs, POWER);
  }

  if (bitRead(tempInputs, MENU) != 0 && bitRead(prevInputs, MENU) == 0) {
    bitSet(inputs, MENU);
  }

  if (bitRead(tempInputs, LEFT) != 0 && bitRead(prevInputs, LEFT) == 0) {
    bitSet(inputs, LEFT);
  }

  if (bitRead(tempInputs, RIGHT) != 0 && bitRead(prevInputs, RIGHT) == 0) {
    bitSet(inputs, RIGHT);
  }

  if (bitRead(tempInputs, SOURCE) != 0 && bitRead(prevInputs, SOURCE) == 0) {
    bitSet(inputs, SOURCE);
  }

  prevInputs = tempInputs;
}

inline bool isPressed(uint8_t button) {
  return bitRead(inputs, button) == 1;
}

inline void clearBitmap(uint8_t *bitmap) {
  for (uint8_t i = 0; i < 32; i++) {
    bitmap[i] = 0x00;
  }
}

void endGameAnim() {
  uint8_t animSpeed = 25;

  for (uint8_t i = 0; i < 32; i++) {
    buffer[i] = pgm_read_byte(&gameOverScreen[i]);
    render();
    delay(animSpeed);
  }
}

void endGame() {
  Serial.print("Final Score: ");
  Serial.println(score);

  gameOver = true;
  clearBitmap(buffer);
  
  endGameAnim();
}

void render() {
  for (uint8_t i = 0; i < 32; i++) {
    mx.setColumn(i, buffer[31 - i]);
  }
}

void placeSnake() {
  buffer[xPos] |= (1 << yPos);
}

void placeApple() {
  xApple = random(0, 32);
  yApple = random(0, 8);

  buffer[xApple] |= (1 << yApple);
}

void update() {
  if (isPressed(POWER)) {
    endGame();
    return;
  }

  if (isPressed(MENU) && !(direction == 0)) {
    direction = 2;
  }

  if (isPressed(LEFT) && !(direction == 2)) {
    direction = 0;
  }

  if (isPressed(RIGHT) && !(direction == 1)) {
    direction = 3;
  }

  if (isPressed(SOURCE) && !(direction == 3)) {
    direction = 1;
  }

  buffer[xPos] &= !(1 << yPos);

  if (direction == 0) {
    xPos -= 1;

    if (xPos < 0) {
      endGame();
      return;
    }

  } else if (direction == 1) {
    yPos -= 1;

    if (yPos < 0) {
      endGame();
      return;
    }

  } else if (direction == 2) {
    xPos += 1;

    if (xPos > 31) {
      endGame();
      return;
    }

  } else if (direction == 3) {
    yPos += 1;

    if (yPos > 7) {
      endGame();
      return;
    }

  }

  if ((xPos == xApple) && (yPos == yApple)) {
    score += 1;
    Serial.print("Score: ");
    Serial.println(score);

    placeApple();
  }

  placeSnake();

  render();
}

void startGame() {
  gameOver = false;
  score = 0;

  xPos = 31;
  yPos = 1;
  direction = 0;

  clearBitmap(buffer);
  placeSnake();

  placeApple();
}

void setup() {
  pinMode(BUTTON_POWER, INPUT_PULLUP);
  pinMode(BUTTON_MENU, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_SOURCE, INPUT_PULLUP);

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 2);
  mx.clear();

  randomSeed(1);

  Serial.begin(115200);

  endGameAnim();
}

void loop() {
  currentMillis = millis();

  if (currentMillis - previousMillis >= delayMillis) {
    previousMillis = currentMillis;

    if (gameOver && isPressed(POWER)) {
      startGame();
    } else if (!gameOver) {
      update();
    }
    
    inputs = 0;
  }

  loadInputs();
}
