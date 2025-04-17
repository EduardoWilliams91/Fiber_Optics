#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Buttons
#define RESET_BUTTON D3     // Reset corruption count
#define BAUD_BUTTON  D4     // Cycle baud rate

// Debounce for D3 (reset)
unsigned long lastDebounceTimeD3 = 0;
bool lastButtonStateD3 = HIGH;
bool buttonPressedD3 = false;

// Debounce for D4 (baud cycle)
unsigned long lastDebounceTimeD4 = 0;
bool lastButtonStateD4 = HIGH;
bool buttonPressedD4 = false;

const unsigned long debounceDelay = 50;

// Baud rate options
const long baudRates[] = {9600, 19200, 38400, 57600, 115200, 230400};
const uint8_t numRates = sizeof(baudRates) / sizeof(baudRates[0]);
uint8_t baudIndex = 0;

// Corruption counter
unsigned int corruptionCount = 0;

// === Validate if message is a number from 1 to 50 ===
bool isValidNumberMessage(String msg) {
  msg.trim();
  for (unsigned int i = 0; i < msg.length(); i++) {
    if (!isDigit(msg[i])) return false;
  }
  int num = msg.toInt();
  return num >= 1 && num <= 50;
}

void setup() {
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  pinMode(BAUD_BUTTON, INPUT_PULLUP);

  Serial.begin(baudRates[baudIndex]);
  delay(100);
  Serial.println("Receiver Ready");

  Wire.begin(D2, D1); // OLED I2C pins

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (1);  // OLED init failed
  }

  // Initial OLED screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Receiver ");
  display.println(baudRates[baudIndex]);
  display.setCursor(0, 16);
  display.println("Waiting...");
  display.display();
}

void loop() {
  // === D3: Reset corruption counter ===
  bool readingD3 = digitalRead(RESET_BUTTON);
  if (readingD3 != lastButtonStateD3) {
    lastDebounceTimeD3 = millis();
  }
  if ((millis() - lastDebounceTimeD3) > debounceDelay) {
    if (readingD3 == LOW && !buttonPressedD3) {
      buttonPressedD3 = true;
      corruptionCount = 0;
      Serial.println("Corruption count reset!");
    }
  }
  if (readingD3 == HIGH) {
    buttonPressedD3 = false;
  }
  lastButtonStateD3 = readingD3;

  // === D4: Cycle and apply new baud rate ===
  bool readingD4 = digitalRead(BAUD_BUTTON);
  if (readingD4 != lastButtonStateD4) {
    lastDebounceTimeD4 = millis();
  }
  if ((millis() - lastDebounceTimeD4) > debounceDelay) {
    if (readingD4 == LOW && !buttonPressedD4) {
      buttonPressedD4 = true;

      // Increment baud index
      baudIndex = (baudIndex + 1) % numRates;

      // OLED feedback before Serial restart
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Receiver ");
      display.println(baudRates[baudIndex]);
      display.setCursor(0, 20);
      display.println("Changing baud...");
      display.display();

      // Restart Serial quickly
      Serial.end();
      delay(10);
      Serial.begin(baudRates[baudIndex]);
      delay(10);
      Serial.println("Baud rate changed.");
    }
  }
  if (readingD4 == HIGH) {
    buttonPressedD4 = false;
  }
  lastButtonStateD4 = readingD4;

  // === Serial message handling ===
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    message.trim();

    bool isValid = (message == "HELLO WORLD") || isValidNumberMessage(message);
    if (!isValid) corruptionCount++;

    Serial.print("Received: ");
    Serial.println(message);
    if (!isValid) {
      Serial.print("Corrupt count: ");
      Serial.println(corruptionCount);
    }

    // === OLED update ===
    display.clearDisplay();

    // Header with baud rate
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Receiver ");
    display.println(baudRates[baudIndex]);

    // Message display
    display.fillRect(0, 20, SCREEN_WIDTH, 28, BLACK);
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println(message);

    // Corruption counter
    display.setTextSize(1);
    display.setCursor(0, 54);
    display.print("Corrupt Count: ");
    display.println(corruptionCount);

    display.display();
  }
}
