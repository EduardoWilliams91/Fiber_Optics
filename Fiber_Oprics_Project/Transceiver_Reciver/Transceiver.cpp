#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Buttons
#define MODE_BUTTON D3  // Mode switch
#define BAUD_BUTTON D5  // Baud rate cycle

// Debounce config
const unsigned long debounceDelay = 50;
bool lastStateD3 = HIGH, buttonPressedD3 = false;
bool lastStateD5 = HIGH, buttonPressedD5 = false;
unsigned long lastDebounceD3 = 0, lastDebounceD5 = 0;

// Modes and counters
uint8_t mode = 0;
uint8_t countdownValue = 1;

// Baud rate options
const long baudRates[] = {9600, 19200, 38400, 57600, 115200, 230400};
const uint8_t numRates = sizeof(baudRates) / sizeof(baudRates[0]);
uint8_t baudIndex = 0;

void setup() {
  pinMode(MODE_BUTTON, INPUT_PULLUP);
  pinMode(BAUD_BUTTON, INPUT_PULLUP);

  Serial.begin(baudRates[baudIndex]);
  delay(100);

  Wire.begin(D2, D1);  // SDA, SCL

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (1); // OLED failed
  }

  // Initial display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Transceiver ");
  display.println(baudRates[baudIndex]);
  display.setCursor(0, 20);
  display.println("HELLO WORLD");
  display.display();
}

void loop() {
  // === Debounce D3 (Mode Change) ===
  bool readingD3 = digitalRead(MODE_BUTTON);
  if (readingD3 != lastStateD3) lastDebounceD3 = millis();
  bool clickD3 = false;
  if ((millis() - lastDebounceD3) > debounceDelay) {
    if (readingD3 == LOW && !buttonPressedD3) {
      buttonPressedD3 = true;
      clickD3 = true;
    }
  }
  if (readingD3 == HIGH) buttonPressedD3 = false;
  lastStateD3 = readingD3;

  // === Debounce D5 (Baud Change) ===
  bool readingD5 = digitalRead(BAUD_BUTTON);
  if (readingD5 != lastStateD5) lastDebounceD5 = millis();
  bool clickD5 = false;
  if ((millis() - lastDebounceD5) > debounceDelay) {
    if (readingD5 == LOW && !buttonPressedD5) {
      buttonPressedD5 = true;
      clickD5 = true;
    }
  }
  if (readingD5 == HIGH) buttonPressedD5 = false;
  lastStateD5 = readingD5;

  // === Handle Baud Change ===
  if (clickD5) {
    baudIndex = (baudIndex + 1) % numRates;

    // Show new rate immediately
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Transceiver ");
    display.println(baudRates[baudIndex]);
    display.setCursor(0, 20);
    display.println("Changing baud...");
    display.display();

    Serial.end();
    delay(10);
    Serial.begin(baudRates[baudIndex]);
    delay(10);
    Serial.println("Baud changed.");
  }

  // === Mode 0: HELLO WORLD ===
  if (mode == 0) {
    String message = "HELLO WORLD";
    Serial.println(message);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Transceiver ");
    display.println(baudRates[baudIndex]);
    display.setCursor(0, 20);
    display.println("HELLO WORLD");
    display.display();

    if (clickD3) {
      mode = 1;
      countdownValue = 1;
    }

    delay(1000);
  }

  // === Mode 1: Countdown ===
  else if (mode == 1) {
    if (clickD3) {
      mode = 0;
      countdownValue = 1;
      return;
    }

    String message = String(countdownValue);
    Serial.println(message);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Transceiver ");
    display.println(baudRates[baudIndex]);
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.println(message);
    display.display();

    countdownValue++;
    if (countdownValue > 50) countdownValue = 1;

    delay(1000);
  }
}
