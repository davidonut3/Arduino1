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

bool prevPower = HIGH;
bool prevMenu = HIGH;
bool prevLeft = HIGH;
bool prevRight = HIGH;
bool prevSource = HIGH;

bool power = false;
bool menu = false;
bool left = false;
bool right = false;
bool source = false;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const long delayMillis = 250;

long gameSpeed = 0;
bool gameOver = false;

int score = 0;
int piece = 0;
int rotation = 0;
int xPos = 0;
int yPos = 31;

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

uint8_t screen[32] = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

uint8_t fixed[32] = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

uint8_t floating[32] = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

uint8_t gameOverScreen[32] = {
  0x3e, 0x22, 0x32, 0x0, 0x3c, 0xa, 0x3c, 0x0, 0x3e, 0x4, 0x3e, 0x0, 0x3e, 0x2a, 0x2a, 0x0, 0x0, 0x3e, 0x22, 0x3e, 0x0, 0x1e, 0x20, 0x1e, 0x0, 0x3e, 0x2a, 0x2a, 0x0, 0x3e, 0xa, 0x34,
};

void render() {
  mx.setBuffer(31, 32, screen);
}

void updateScreen() {
  for (int i = 0; i < 32; i++ ) {
    screen[i] = fixed[i] | floating[i];
  }
}

void loadInputs() {
  bool newPower = digitalRead(BUTTON_POWER);
  bool newMenu = digitalRead(BUTTON_MENU);
  bool newLeft = digitalRead(BUTTON_LEFT);
  bool newRight = digitalRead(BUTTON_RIGHT);
  bool newSource = digitalRead(BUTTON_SOURCE);

  if (prevPower == HIGH && newPower == LOW) {
    power = true;
  }

  if (prevMenu == HIGH && newMenu == LOW) {
    menu = true;
  }

  if (prevLeft == HIGH && newLeft == LOW) {
    left = true;
  }

  if (prevRight == HIGH && newRight == LOW) {
    right = true;
  }

  if (prevSource == HIGH && newSource == LOW) {
    source = true;
  }

  prevPower = newPower;
  prevMenu = newMenu;
  prevLeft = newLeft;
  prevRight = newRight;
  prevSource = newSource;
}

void resetInputs() {
  power = false;
  menu = false;
  left = false;
  right = false;
  source = false;
}

void clearBitmap(uint8_t *bitmap) {
  for (int i = 0; i < 32; i++) {
    bitmap[i] = 0x00;
  }
}

void printBitmap(uint8_t *bitmap) {
  for (int i = 0; i < 32; i++) {
    Serial.print(bitmap[i]);
    Serial.print("\t");
  }

  Serial.println("");
}

bool applyGravity() {
  bool is_floating = true;

  for (int i = 1; i < 32; i++) {
    for (int j = 0; j < 8; j++) {

      uint8_t mask = 1 << j;

      if ((mask & floating[i]) != 0 && (mask & fixed[i - 1]) != 0) {
        return false;
      } else if ((mask & floating[i]) != 0 && (mask & fixed[i - 1]) == 0) {

        floating[i] ^= mask;
        floating[i - 1] ^= mask;

        if (i == 1) {
          is_floating = false;
        } else if ((mask & fixed[i - 2]) != 0) {
          is_floating = false;
        }
      }
    }
  }

  yPos--;

  return is_floating;
}

void applyLeft() {
  bool allowLeft = true;

  for (int i = 0; i < 32; i++) {
    if ((0x1 & floating[i]) != 0 || ((floating[i] >> 1) & fixed[i]) != 0) {
      allowLeft = false;
    }
  }

  if (allowLeft) {
    xPos--;

    for (int i = 0; i < 32; i++) {
      floating[i] >>= 1;
    }
  }
}

void applyRight() {
  bool allowRight = true;

  for (int i = 0; i < 32; i++) {
    if ((0x80 & floating[i]) != 0 || ((floating[i] << 1) & fixed[i]) != 0) {
      allowRight = false;
    }
  }

  if (allowRight) {
    xPos++;

    for (int i = 0; i < 32; i++) {
      floating[i] <<= 1;
    }
  }
}

void lineClearAnim(int index) {
  int animSpeed = 50;

  for (int i = 0; i < 8; i++) {
    updateScreen();
    render();
    delay(animSpeed);
    fixed[index] >>= 1;
  }

  fixed[index] = 0x0;
  updateScreen();
  render();
  delay(animSpeed);
}

void clearLines() {
  clearBitmap(floating);

  int tempScore = 0;

  for (int i = 0; i < 32; i++) {

    if (fixed[i] == 0xFF) {

      tempScore++;
      
      lineClearAnim(i);

      for (int j = i + 1; j < 32; j++) {

        fixed[j - 1] = fixed[j];
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
      xPos = min(xPos, 4);
      yPos = max(yPos, 1);

      floating[yPos] = 15 << xPos;
      break;

    case 1:
      xPos = min(xPos, 5);
      yPos = max(yPos, 3);

      floating[yPos] = 4 << xPos;
      floating[yPos - 1] = 4 << xPos;
      floating[yPos - 2] = 4 << xPos;
      floating[yPos - 3] = 4 << xPos;
      break;

    case 2:
      xPos = min(xPos, 4);
      yPos = max(yPos, 2);

      floating[yPos - 1] = 15 << xPos;
      break;

    case 3:
      xPos = min(xPos, 6);
      yPos = max(yPos, 3);

      floating[yPos] = 2 << xPos;
      floating[yPos - 1] = 2 << xPos;
      floating[yPos - 2] = 2 << xPos;
      floating[yPos - 3] = 2 << xPos;
      break;
  }
}

void placePieceS() {
  switch (rotation) {

    case 0:
      xPos = min(xPos, 5);
      yPos = max(yPos, 1);

      floating[yPos] = 6 << xPos;
      floating[yPos - 1] = 3 << xPos;
      break;

    case 1:
      xPos = min(xPos, 5);
      yPos = max(yPos, 2);

      floating[yPos] = 2 << xPos;
      floating[yPos - 1] = 6 << xPos;
      floating[yPos - 2] = 4 << xPos;
      break;

    case 2:
      xPos = min(xPos, 5);
      yPos = max(yPos, 2);

      floating[yPos - 1] = 6 << xPos;
      floating[yPos - 2] = 3 << xPos;
      break;

    case 3:
      xPos = min(xPos, 6);
      yPos = max(yPos, 2);

      floating[yPos] = 1 << xPos;
      floating[yPos - 1] = 3 << xPos;
      floating[yPos - 2] = 2 << xPos;
      break;
  }
}

void placePieceZ() {
  switch (rotation) {

    case 0:
      xPos = min(xPos, 5);
      yPos = max(yPos, 1);

      floating[yPos] = 3 << xPos;
      floating[yPos - 1] = 6 << xPos;
      break;

    case 1:
      xPos = min(xPos, 5);
      yPos = max(yPos, 2);

      floating[yPos] = 4 << xPos;
      floating[yPos - 1] = 6 << xPos;
      floating[yPos - 2] = 2 << xPos;
      break;

    case 2:
      xPos = min(xPos, 5);
      yPos = max(yPos, 2);

      floating[yPos - 1] = 3 << xPos;
      floating[yPos - 2] = 6 << xPos;
      break;

    case 3:
      xPos = min(xPos, 6);
      yPos = max(yPos, 2);

      floating[yPos] = 2 << xPos;
      floating[yPos - 1] = 3 << xPos;
      floating[yPos - 2] = 1 << xPos;
      break;
  }
}

void placePieceO() {
  floating[yPos] = 3 << xPos;
  floating[yPos - 1] = 3 << xPos;
}

void placePieceJ() {
  switch (rotation) {

    case 0:
      xPos = min(xPos, 5);
      yPos = max(yPos, 1);

      floating[yPos] = 1 << xPos;
      floating[yPos - 1] = 7 << xPos;
      break;

    case 1:
      xPos = min(xPos, 5);
      yPos = max(yPos, 2);

      floating[yPos] = 6 << xPos;
      floating[yPos - 1] = 2 << xPos;
      floating[yPos - 2] = 2 << xPos;
      break;

    case 2:
      xPos = min(xPos, 5);
      yPos = max(yPos, 2);

      floating[yPos - 1] = 7 << xPos;
      floating[yPos - 2] = 4 << xPos;
      break;

    case 3:
      xPos = min(xPos, 6);
      yPos = max(yPos, 2);

      floating[yPos] = 2 << xPos;
      floating[yPos - 1] = 2 << xPos;
      floating[yPos - 2] = 3 << xPos;
      break;
  }
}

void placePieceL() {
  switch (rotation) {

    case 0:
      xPos = min(xPos, 5);
      yPos = max(yPos, 1);

      floating[yPos] = 4 << xPos;
      floating[yPos - 1] = 7 << xPos;
      break;

    case 1:
      xPos = min(xPos, 5);
      yPos = max(yPos, 2);

      floating[yPos] = 2 << xPos;
      floating[yPos - 1] = 2 << xPos;
      floating[yPos - 2] = 6 << xPos;
      break;

    case 2:
      xPos = min(xPos, 5);
      yPos = max(yPos, 2);

      floating[yPos - 1] = 7 << xPos;
      floating[yPos - 2] = 1 << xPos;
      break;

    case 3:
      xPos = min(xPos, 6);
      yPos = max(yPos, 2);

      floating[yPos] = 3 << xPos;
      floating[yPos - 1] = 2 << xPos;
      floating[yPos - 2] = 2 << xPos;
      break;
  }
}

void placePieceT() {
  switch (rotation) {

    case 0:
      xPos = min(xPos, 5);
      yPos = max(yPos, 1);

      floating[yPos] = 2 << xPos;
      floating[yPos - 1] = 7 << xPos;
      break;

    case 1:
      xPos = min(xPos, 5);
      yPos = max(yPos, 2);

      floating[yPos] = 2 << xPos;
      floating[yPos - 1] = 6 << xPos;
      floating[yPos - 2] = 2 << xPos;
      break;

    case 2:
      xPos = min(xPos, 5);
      yPos = max(yPos, 2);

      floating[yPos - 1] = 7 << xPos;
      floating[yPos - 2] = 2 << xPos;
      break;

    case 3:
      xPos = min(xPos, 6);
      yPos = max(yPos, 2);

      floating[yPos] = 2 << xPos;
      floating[yPos - 1] = 3 << xPos;
      floating[yPos - 2] = 2 << xPos;
      break;
  }
}

void placePiece() {

  clearBitmap(floating);

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

int loadPiece() {

  piece = random(0, 7);

  if (piece == 2) {
    xPos = 3;
  } else {
    xPos = 2;
  }

  rotation = 0;
  yPos = 31;

  placePiece();

  for (int i = 0; i < 32; i++) {
    if ((floating[i] & fixed[i]) != 0) {
      endGame();
    }
  }

  return piece;
}

void endGameAnim() {
  int animSpeed = 25;

  for (int i = 0; i < 32; i++) {
    fixed[i] = gameOverScreen[i];
    updateScreen();
    render();
    delay(animSpeed);
  }
}

void endGame() {
  Serial.print("Final Score: ");
  Serial.println(score);

  gameOver = true;
  clearBitmap(floating);
  clearBitmap(fixed);
  
  endGameAnim();
}

void startGame() {
  gameOver = false;
  gameSpeed = 0;
  score = 0;

  clearBitmap(floating);
  clearBitmap(fixed);

  piece = loadPiece();
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

  randomSeed(analogRead(0));

  Serial.begin(115200);

  startGame();
}

void update() {

  if (source && !right) {
    applyLeft();
  }

  if (right && !source) {
    applyRight();
  }

  if (menu) {
    gameSpeed = delayMillis - 25;
  }

  if (left) {
    rotation = (rotation + 1) % 4;
    placePiece();

    // This is a bit hacky, but it ensures that we do not rotate into other blocks
    bool rotationSuccess = true;
    for (int i = 0; i < 32; i++) {
      if ((floating[i] & fixed[i]) != 0) {
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

    for (int i = 0; i < 32; i++) {
      fixed[i] |= floating[i];
    }

    clearLines();

    piece = loadPiece();
  }

  updateScreen();
}

void loop() {

  currentMillis = millis();

  if (currentMillis - previousMillis >= delayMillis - gameSpeed) {
    previousMillis = currentMillis;

    if (!gameOver) {
      update();
    } 
    
    if (power) {
      startGame();
    }

    render();
    resetInputs();
  }

  loadInputs();
}
