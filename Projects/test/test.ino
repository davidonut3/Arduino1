#include "pitches.h"

#define BUZZER 2

#define LED_GREEN 9
#define LED_RED 3

#define BUTTON_POWER 4
#define BUTTON_MENU 5
#define BUTTON_LEFT 6
#define BUTTON_RIGHT 7
#define BUTTON_SOURCE 8

#include "MD_Parola.h"
#include "MD_MAX72xx.h"
#include "SPI.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 10

MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

int redValue = 0;
int greenValue = 0;
int increment = 16;

void laserZap() {
  for (int freq = 2000; freq >= 100; freq -= 40) {
    tone(BUZZER, freq);
    delay(3);
  }
  noTone(BUZZER);
}

void powerUp() {
  int notes[] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5};
  for (int i = 0; i < 4; i++) {
    tone(BUZZER, notes[i], 150);
    delay(200);
  }
  noTone(BUZZER);
}

void coinCollect() {
  tone(BUZZER, NOTE_B5, 100);
  delay(120);
  tone(BUZZER, NOTE_E6, 500);
  delay(600);
  noTone(BUZZER);
}

void explosionSound() {
  for (int i = 0; i < 150; i++) {
    int freq = random(50, 350);
    tone(BUZZER, freq);
    delay(random(1, 10));
  }
  noTone(BUZZER);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_POWER, INPUT_PULLUP);
  pinMode(BUTTON_MENU, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_SOURCE, INPUT_PULLUP);

  coinCollect();

  myDisplay.begin();
  myDisplay.setIntensity(0);
  myDisplay.displayClear();

}

void loop() {
  
  if (digitalRead(BUTTON_POWER) == 0) {
    Serial.println("Power");
    greenValue = 0;
    redValue = 0;
  }

  if (digitalRead(BUTTON_MENU) == 0) {
    Serial.println("Menu");
    greenValue -= increment;
  }

  if (digitalRead(BUTTON_LEFT) == 0) {
    Serial.println("Left");
    greenValue += increment;
  }

  if (digitalRead(BUTTON_RIGHT) == 0) {
    Serial.println("Right");
    redValue -= increment;
  }

  if (digitalRead(BUTTON_SOURCE) == 0) {
    Serial.println("Source");
    redValue += increment;
  }

  analogWrite(LED_RED, redValue);
  analogWrite(LED_GREEN, greenValue);

  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print("Center");
  delay(2000);
  myDisplay.setTextAlignment(PA_LEFT);
  myDisplay.print("Left");
  delay(2000);
  myDisplay.setTextAlignment(PA_RIGHT);
  myDisplay.print("Right");
  delay(2000);
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.setInvert(true);
  myDisplay.print("Invert");
  delay(2000);
  myDisplay.setInvert(false);
  myDisplay.print(1234);
  delay(2000);

}
