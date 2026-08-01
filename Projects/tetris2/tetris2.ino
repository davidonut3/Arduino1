#include <MD_MAX72xx.h>
#include <SPI.h>
#include <tetris.h>

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

uint8_t state = 1;
uint8_t menuState = 0;
uint8_t pauseState = 0;

uint8_t prevInputs = 0;
uint8_t inputs = 0;

uint16_t previousMillis = 0;
uint16_t currentMillis = 0;
const uint16_t delayMillis = 250;

uint16_t gameSpeed = 0;
bool gameOver = false;
uint16_t score = 0;

uint8_t piece = 0;
uint8_t rotation = 0;
uint8_t xPos = 0;
uint8_t yPos = 31;

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

uint8_t buffer1[32] = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

uint8_t buffer2[32] = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

uint8_t menuBuffer[32] = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

const uint8_t gameOverScreen[32] PROGMEM = {
  0x3e, 0x22, 0x3a, 0x0, 0x3e, 0xa, 0x3e, 0x0, 0x3e, 0x4, 0x3e, 0x0, 0x3e, 0x2a, 0x2a, 0x0, 0x0, 0x3e, 0x22, 0x3e, 0x0, 0x1e, 0x20, 0x1e, 0x0, 0x3e, 0x2a, 0x2a, 0x0, 0x3e, 0xa, 0x36,
};

const uint8_t pauseScreen[32] PROGMEM = {
  0x0, 0x38, 0x44, 0x4e, 0x4e, 0x44, 0x38, 0x0, 0x0, 0x8, 0x7c, 0x7e, 0x7e, 0x7c, 0x8, 0x0, 0x0, 0x3c, 0x42, 0x42, 0xe2, 0x42, 0x1c, 0x0, 0x0, 0x0, 0x7e, 0x7e, 0x3c, 0x18, 0x0, 0x0,
};

const uint8_t menuScreen[32] PROGMEM = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc, 0xc, 0x3c, 0x3c, 0x30, 0x30, 0x0,
}

void renderBuffer12() {
  for (uint8_t i = 0; i < 32; i++) {
    mx.setColumn(i, buffer1[31 - i] | buffer2[31 - i]);
  }
}

void renderMenuBuffer() {
  for (uint8_t i = 0; i < 32; i++) {
    mx.setColumn(i, menuBuffer[31 - i]);
  }
}

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

void printBitmap(uint8_t *bitmap) {
  for (uint8_t i = 0; i < 32; i++) {
    Serial.print(bitmap[i]);
    Serial.print("\t");
  }

  Serial.println("");
}

bool applyGravity() {
  bool is_floating = true;

  for (uint8_t i = 1; i < 32; i++) {
    if ((buffer2[i] & buffer1[i - 1]) != 0) {
      return false;
    }
  }

  for (uint8_t i = 1; i < 32; i++) {
    buffer2[i - 1] = buffer2[i];

    if ((buffer2[i] & buffer1[i - 1]) != 0) {
      is_floating = false;
    }
  }

  yPos--;

  buffer2[31] = 0x0;
  if (buffer2[0] != 0) {
    return false;
  }

  return is_floating;
}

void applyLeft() {
  bool allowLeft = true;

  for (uint8_t i = 0; i < 32; i++) {
    if ((0x1 & buffer2[i]) != 0 || ((buffer2[i] >> 1) & buffer1[i]) != 0) {
      allowLeft = false;
    }
  }

  if (allowLeft) {
    xPos--;

    for (uint8_t i = 0; i < 32; i++) {
      buffer2[i] >>= 1;
    }
  }
}

void applyRight() {
  bool allowRight = true;

  for (uint8_t i = 0; i < 32; i++) {
    if ((0x80 & buffer2[i]) != 0 || ((buffer2[i] << 1) & buffer1[i]) != 0) {
      allowRight = false;
    }
  }

  if (allowRight) {
    xPos++;

    for (uint8_t i = 0; i < 32; i++) {
      buffer2[i] <<= 1;
    }
  }
}

void lineClearAnim(uint8_t index) {
  uint8_t animSpeed = 50;

  for (uint8_t i = 0; i < 8; i++) {
    renderBuffer12();
    delay(animSpeed);
    buffer1[index] >>= 1;
  }

  buffer1[index] = 0x0;
  renderBuffer12();
  delay(animSpeed);
}

void clearLines() {
  clearBitmap(buffer2);

  uint8_t tempScore = 0;

  for (uint8_t i = 0; i < 32; i++) {

    if (buffer1[i] == 0xFF) {

      tempScore++;
      
      lineClearAnim(i);

      for (uint8_t j = i + 1; j < 32; j++) {

        buffer1[j - 1] = buffer1[j];
      }

      i--;
    }
  }

  switch (tempScore) {
    case 1: score += 1; break;
    case 2: score += 3; break;
    case 3: score += 6; break;
    case 4: score += 10; break;
  }

  if (tempScore != 0) {
    Serial.println(score);
  }
}

void placePieceI() {
  switch (rotation) {
    case 0:
      xPos = min((xPos + 8) % 256, 12) - 8;
      yPos = max(yPos, 1);

      buffer2[yPos] = 15 << xPos;
      break;

    case 1:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 3);

      buffer2[yPos] = 4 << xPos;
      buffer2[yPos - 1] = 4 << xPos;
      buffer2[yPos - 2] = 4 << xPos;
      buffer2[yPos - 3] = 4 << xPos;
      break;

    case 2:
      xPos = min((xPos + 8) % 256, 12) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos - 1] = 15 << xPos;
      break;

    case 3:
      xPos = min((xPos + 8) % 256, 14) - 8;
      yPos = max(yPos, 3);

      buffer2[yPos] = 2 << xPos;
      buffer2[yPos - 1] = 2 << xPos;
      buffer2[yPos - 2] = 2 << xPos;
      buffer2[yPos - 3] = 2 << xPos;
      break;
  }
}

void placePieceS() {
  switch (rotation) {

    case 0:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 1);

      buffer2[yPos] = 6 << xPos;
      buffer2[yPos - 1] = 3 << xPos;
      break;

    case 1:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos] = 2 << xPos;
      buffer2[yPos - 1] = 6 << xPos;
      buffer2[yPos - 2] = 4 << xPos;
      break;

    case 2:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos - 1] = 6 << xPos;
      buffer2[yPos - 2] = 3 << xPos;
      break;

    case 3:
      xPos = min((xPos + 8) % 256, 14) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos] = 1 << xPos;
      buffer2[yPos - 1] = 3 << xPos;
      buffer2[yPos - 2] = 2 << xPos;
      break;
  }
}

void placePieceZ() {
  switch (rotation) {

    case 0:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 1);

      buffer2[yPos] = 3 << xPos;
      buffer2[yPos - 1] = 6 << xPos;
      break;

    case 1:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos] = 4 << xPos;
      buffer2[yPos - 1] = 6 << xPos;
      buffer2[yPos - 2] = 2 << xPos;
      break;

    case 2:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos - 1] = 3 << xPos;
      buffer2[yPos - 2] = 6 << xPos;
      break;

    case 3:
      xPos = min((xPos + 8) % 256, 14) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos] = 2 << xPos;
      buffer2[yPos - 1] = 3 << xPos;
      buffer2[yPos - 2] = 1 << xPos;
      break;
  }
}

void placePieceO() {
  buffer2[yPos] = 3 << xPos;
  buffer2[yPos - 1] = 3 << xPos;
}

void placePieceJ() {
  switch (rotation) {

    case 0:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 1);

      buffer2[yPos] = 1 << xPos;
      buffer2[yPos - 1] = 7 << xPos;
      break;

    case 1:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos] = 6 << xPos;
      buffer2[yPos - 1] = 2 << xPos;
      buffer2[yPos - 2] = 2 << xPos;
      break;

    case 2:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos - 1] = 7 << xPos;
      buffer2[yPos - 2] = 4 << xPos;
      break;

    case 3:
      xPos = min((xPos + 8) % 256, 14) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos] = 2 << xPos;
      buffer2[yPos - 1] = 2 << xPos;
      buffer2[yPos - 2] = 3 << xPos;
      break;
  }
}

void placePieceL() {
  switch (rotation) {

    case 0:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 1);

      buffer2[yPos] = 4 << xPos;
      buffer2[yPos - 1] = 7 << xPos;
      break;

    case 1:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos] = 2 << xPos;
      buffer2[yPos - 1] = 2 << xPos;
      buffer2[yPos - 2] = 6 << xPos;
      break;

    case 2:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos - 1] = 7 << xPos;
      buffer2[yPos - 2] = 1 << xPos;
      break;

    case 3:
      xPos = min((xPos + 8) % 256, 14) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos] = 3 << xPos;
      buffer2[yPos - 1] = 2 << xPos;
      buffer2[yPos - 2] = 2 << xPos;
      break;
  }
}

void placePieceT() {
  switch (rotation) {

    case 0:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 1);

      buffer2[yPos] = 2 << xPos;
      buffer2[yPos - 1] = 7 << xPos;
      break;

    case 1:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos] = 2 << xPos;
      buffer2[yPos - 1] = 6 << xPos;
      buffer2[yPos - 2] = 2 << xPos;
      break;

    case 2:
      xPos = min((xPos + 8) % 256, 13) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos - 1] = 7 << xPos;
      buffer2[yPos - 2] = 2 << xPos;
      break;

    case 3:
      xPos = min((xPos + 8) % 256, 14) - 8;
      yPos = max(yPos, 2);

      buffer2[yPos] = 2 << xPos;
      buffer2[yPos - 1] = 3 << xPos;
      buffer2[yPos - 2] = 2 << xPos;
      break;
  }
}

void placePiece() {

  clearBitmap(buffer2);

  switch (piece) {
    case 0: placePieceS(); break;
    case 1: placePieceZ(); break;
    case 2: placePieceO(); break;
    case 3: placePieceJ(); break;
    case 4: placePieceL(); break;
    case 5: placePieceT(); break;
    case 6: placePieceI(); break;
  }
}

void loadPiece() {

  piece = random(0, 7);

  if (piece == 2) {
    xPos = 3;
  } else {
    xPos = 2;
  }

  rotation = 0;
  yPos = 31;

  placePiece();

  for (uint8_t i = 0; i < 32; i++) {
    if ((buffer2[i] & buffer1[i]) != 0) {
      endGame();
    }
  }
}

void endGameAnim() {
  uint8_t animSpeed = 25;

  for (uint8_t i = 0; i < 32; i++) {
    buffer1[i] = pgm_read_byte(&gameOverScreen[i]);
    renderBuffer12();
    delay(animSpeed);
  }
}

void endGame() {
  Serial.print("Final Score: ");
  Serial.println(score);

  gameOver = true;
  clearBitmap(buffer2);
  clearBitmap(buffer1);
  
  endGameAnim();
}

void startTetris() {
  gameOver = false;
  gameSpeed = 0;
  score = 0;

  clearBitmap(buffer2);
  clearBitmap(buffer1);

  loadPiece();
}

void startBlank() {
  state = 0;

  clearBitmap(menuBuffer);
}

void startMenu() {
  state = 1;
  menuState = 3;

  for (uint8_t i = 0; i < 32; i++) {
    menuBuffer[i] = pgm_read_byte(&pauseScreen[i]);
  }
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

  startMenu();
}

void updateTetris() {

  if (isPressed(SOURCE) && !isPressed(RIGHT)) {
    applyLeft();
  }

  if (isPressed(RIGHT) && !isPressed(SOURCE)) {
    applyRight();
  }

  if (isPressed(MENU)) {
    gameSpeed = delayMillis - 25;
  }

  if (isPressed(LEFT)) {
    rotation = (rotation + 1) % 4;
    placePiece();

    // This is a bit hacky, but it ensures that we do not rotate into other blocks
    bool rotationSuccess = true;
    for (uint8_t i = 0; i < 32; i++) {
      if ((buffer2[i] & buffer1[i]) != 0) {
        rotationSuccess = false;
      }
    }

    if (!rotationSuccess) {
      rotation = (rotation + 3) % 4;
      placePiece();
    }
  }

  bool is_floating = applyGravity();
  
  if (!is_floating) {
    gameSpeed = 0;

    for (uint8_t i = 0; i < 32; i++) {
      buffer1[i] |= buffer2[i];
    }

    clearLines();

    loadPiece();
  }
}

void loopMenu() {
  // updateMenu();

  if (isPressed(SOURCE) && !isPressed(RIGHT)) {
    menuState = (menuState + 1) % 4;
  }

  if (isPressed(RIGHT) && !isPressed(SOURCE)) {
    menuState = (menuState + 3) % 4;
  }

  if (isPressed(POWER)) {
    switch (menuState) {
      case 0: startBlank(); break;
    }
  }

  renderMenuBuffer();
}

void loopBlank() {
  if (isPressed(POWER) || isPressed(MENU) || isPressed(LEFT) || isPressed(RIGHT) || isPressed(SOURCE)) {
    startMenu();
  }

  renderMenuBuffer();
}

void loop() {

  currentMillis = millis();

  if (currentMillis - previousMillis >= delayMillis - gameSpeed) {
    previousMillis = currentMillis;

    switch (state) {
      case 0: loopBlank(); break;
      case 1: loopMenu(); break;
    }

    // if (!gameOver) {
    //   updateTetris();
    // } 
    
    // if (isPressed(POWER)) {
    //   startTetris();
    // }

    // renderBuffer12();
    inputs = 0;
  }

  loadInputs();
}
