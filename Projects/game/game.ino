// Setup and definitions

#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 10

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

#define LED_GREEN 5
#define LED_RED 6

#define BUZZER 19
#define BUTTON_UP 4
#define BUTTON_DOWN 3
#define BUTTON_FIRE 8
#define BUTTON_RESET 2

// Game variables

int playerX = 31;
int playerY = 3;

int enemyX = 0;
int enemyY = 3;

bool bulletActive = false;
int bulletX = 0;
int bulletY = 0;

bool gameOver = false;
int score = 0;

unsigned long lastUpdate = 0;
int gameSpeed = 100;

bool prevUp = HIGH;
bool prevDown = HIGH;
bool prevFire = HIGH;
bool prevReset = HIGH;

// Sound effects

void explosionSound() {
  for (int i = 0; i < 50; i++) {
    int freq = random(50, 350);
    tone(BUZZER, freq);
    delay(random(1, 4));
  }
  noTone(BUZZER);
}

void laserZap() {
  for (int freq = 2000; freq >= 100; freq -=80) {
    tone(BUZZER, freq);
    delay(3);
  }
  noTone(BUZZER);
}

void beep(int freq, int duration) {
  tone(BUZZER, freq, duration);
}

// Game functions

void spawnEnemy() {
  enemyX = 0;
  enemyY = random(0, 8);
}

void resetGame() {
  playerY = 3;
  bulletActive = false;
  gameOver = false;
  score = 0;
  Serial.println(score);
  spawnEnemy();
}

// Setup

void setup() {
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_FIRE, INPUT_PULLUP);
  pinMode(BUTTON_RESET, INPUT_PULLUP);

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 2);
  mx.clear();

  randomSeed(analogRead(0));
  spawnEnemy();

  Serial.begin(115200);
}

// Input handling

void handleInput() {
  bool up = digitalRead(BUTTON_UP);
  bool down = digitalRead(BUTTON_DOWN);
  bool fire = digitalRead(BUTTON_FIRE);

  if (prevUp == HIGH && up == LOW && playerY < 7) {
    playerY++;
  }

  if (prevDown == HIGH && down == LOW && playerY > 0) {
    playerY--;
  }

  if (prevFire == HIGH && fire == LOW && !bulletActive) {
    bulletActive = true;
    bulletX = playerX - 1;
    bulletY = playerY;
    laserZap();
  }

  prevUp = up;
  prevDown = down;
  prevFire = fire;
}

// Game update

void updateGame() {
  int prevEnemyX = enemyX;
  int prevBulletX = bulletX;

  enemyX++;
  if (enemyX > 31) {
    spawnEnemy();
    score--;
    Serial.println(score);
  }

  if (bulletActive) {
    bulletX--;
    if (bulletX < 0) bulletActive = false;
  }

  if (bulletActive && bulletY == enemyY) {
    if (prevBulletX >= prevEnemyX && bulletX <= enemyX) {
      bulletActive = false;
      spawnEnemy();
      score++;
      Serial.println(score);
      explosionSound();
    }
  }

  if (enemyX == playerX && enemyY == playerY) {
    gameOver = true;
    Serial.println("Game over");
    Serial.println("Final score:");
    Serial.println(score);
    Serial.println();
    beep(50, 500);
  }

  gameSpeed = 100 - 10 * floor(score / 10);
}

// Rendering functions

void drawGameOver() {
  for (int m = 0; m < 4; m++) {
    int offset = m * 8;
    for (int i = 0; i < 8; i++) {
      mx.setPoint(i, offset + i, true);
      mx.setPoint(i, offset + (7 - i), true);
    }
  }
}

void render() {
  mx.clear();

  if (gameOver) {
    drawGameOver();
    return;
  }

  mx.setPoint(playerY, playerX, true);

  mx.setPoint(enemyY, enemyX, true);

  if (bulletActive) {
    mx.setPoint(bulletY, bulletX, true);
  }

  analogWrite(LED_RED, score * 10);
  analogWrite(LED_GREEN, score * 10);
}

void loop() {
  if (gameOver) {
    bool reset = digitalRead(BUTTON_RESET);

    if (prevReset == HIGH && reset == LOW) {
      resetGame();
    }

    prevReset = reset;

    render();
    return;
  }

  handleInput();

  if (millis() - lastUpdate > gameSpeed) {
    lastUpdate = millis();
    updateGame();
    render();
  }
}