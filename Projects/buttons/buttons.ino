#define LED_GREEN 5
#define LED_RED 6

#define BUTTON_POWER 2
#define BUTTON_MENU 3
#define BUTTON_LEFT 4
#define BUTTON_RIGHT 7
#define BUTTON_SOURCE 8

int redValue = 0;
int greenValue = 0;
int increment = 16;

void setup() {

  // We initialise the pins and the Serial port

  Serial.begin(115200);
  pinMode(BUTTON_POWER, INPUT_PULLUP);
  pinMode(BUTTON_MENU, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_SOURCE, INPUT_PULLUP);
}

void loop() {

  // We have to check for 0 since the buttons are at 1 when not pressed

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
  
  delay(200);
}
