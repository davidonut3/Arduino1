#include "pitches.h"

int buzzerPin = 19;

void playStarWars() {
  tone(buzzerPin, NOTE_A4, 500);
  delay(350);
  tone(buzzerPin, 0, 500);
  delay(350);
  tone(buzzerPin, NOTE_A4, 500);
  delay(350);
  tone(buzzerPin, 0, 500);
  delay(350);
  tone(buzzerPin, NOTE_A4, 500);
  delay(350);
  tone(buzzerPin, 0, 500);
  delay(350);
  tone(buzzerPin, NOTE_F4, 350);
  delay(255);
  tone(buzzerPin, NOTE_C5, 150);
  delay(195);
  tone(buzzerPin, NOTE_A4, 500);
  delay(350);
  tone(buzzerPin, 0, 500);
  delay(350);
  tone(buzzerPin, NOTE_F4, 350);
  delay(255);
  tone(buzzerPin, NOTE_C5, 150);
  delay(195);
  tone(buzzerPin, NOTE_A4, 650);
  delay(445);
  tone(buzzerPin, 0, 500);
  delay(350);
  tone(buzzerPin, NOTE_E5, 500);
  delay(350);
  tone(buzzerPin, 0, 500);
  delay(350);
  tone(buzzerPin, NOTE_E5, 500);
  delay(350);
  tone(buzzerPin, 0, 500);
  delay(350);
  tone(buzzerPin, NOTE_E5, 500);
  delay(350);
  tone(buzzerPin, 0, 350);
  delay(255);
  tone(buzzerPin, NOTE_F5, 150);
  delay(195);
  tone(buzzerPin, NOTE_C5, 500);
  delay(350);
  tone(buzzerPin, NOTE_GS4, 500);
  delay(350);
  tone(buzzerPin, NOTE_F4, 350);
  delay(255);
  tone(buzzerPin, NOTE_C5, 150);
  delay(195);
  tone(buzzerPin, NOTE_A4, 650);
  delay(445);
  tone(buzzerPin, 0, 500);
  delay(350);

  noTone(buzzerPin);
}

void siren(int cycles) {
  for (int c = 0; c < cycles; c++) {
    for (int freq = 800; freq <= 1200; freq += 5) {
      tone(buzzerPin, freq);
      delay(5);
    }
    for (int freq = 1200; freq >= 800; freq -= 5) {
      tone(buzzerPin, freq);
      delay(5);
    }
  }
  noTone(buzzerPin);
}

void laserZap() {
  for (int freq = 2000; freq >= 100; freq -= 40) {
    tone(buzzerPin, freq);
    delay(3);
  }
  noTone(buzzerPin);
}

void powerUp() {
  int notes[] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5};
  for (int i = 0; i < 4; i++) {
    tone(buzzerPin, notes[i], 150);
    delay(200);
  }
  noTone(buzzerPin);
}

void alarm(int times) {
  for (int i = 0; i < times; i++) {
    tone(buzzerPin, 2500, 100);
    delay(150);
    tone(buzzerPin, 2000, 100);
    delay(150);
  }
  noTone(buzzerPin);
}

void coinCollect() {
  tone(buzzerPin, NOTE_B5, 100);
  delay(120);
  tone(buzzerPin, NOTE_E6, 500);
  delay(600);
  noTone(buzzerPin);
}

void explosionSound() {
  // Rapidly sweep through random frequencies
  for (int i = 0; i < 150; i++) {
    int freq = random(50, 350); // Randomize frequencies for crackle
    tone(buzzerPin, freq);
    delay(random(1, 10)); // Varying duration to simulate erratic bass
  }
  noTone(buzzerPin);
}

void setup() {
  playStarWars();
  delay(1000);

  siren(5);
  delay(1000);

  laserZap();
  delay(1000);

  powerUp();
  delay(1000);

  alarm(5);
  delay(1000);

  coinCollect();
  delay(1000);

  explosionSound();
}

void loop() {
  // We only play our sounds once, so we do not need to loop anything
}