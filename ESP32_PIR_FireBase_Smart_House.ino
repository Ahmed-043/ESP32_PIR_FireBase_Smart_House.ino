#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Wi-Fi credentials
const char* ssid = "Home";
const char* password = "MughalHome";

// Firebase credentials
#define API_KEY "AIzaSyCqs6trEd2Lm0RdaZOHAmqkriAUShbi-pg"
#define DATABASE_URL "https://esp32-smart-home-5643e-default-rtdb.firebaseio.com/"
#define USER_EMAIL "mughalprince11123@gmail.com"
#define USER_PASSWORD "mughal13579"

// Relay GPIOs (active LOW)
#define RELAY1 23
#define RELAY2 19
#define RELAY3 18
#define RELAY4 5

// Onboard LED
#define LED_PIN 2

// PIR sensor pin (OUT -> GPIO 27)
#define PIR_PIN 27

// Firebase setup
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Manual states from Firebase
bool prevRelay1 = false;
bool prevRelay2 = false;
bool prevRelay3 = false;
bool prevRelay4 = false;

// Sensors config string from Firebase
String sensorsList = "";

// Timeout (seconds) from Firebase
uint32_t timeoutSec = 60;

// Output change logs
bool prevOut1 = false, prevOut2 = false, prevOut3 = false, prevOut4 = false;

// PIR edge detection
bool prevPirLevel = false;

// Sensor timer state
bool sensorActive = false;
unsigned long sensorOffAtMs = 0;

// Poll Firebase occasionally so PIR handling is responsive
unsigned long lastFirebasePoll = 0;
const unsigned long FIREBASE_POLL_MS = 500;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  digitalWrite(RELAY3, HIGH);
  digitalWrite(RELAY4, HIGH);

  pinMode(PIR_PIN, INPUT);

  connectWiFi();
  initFirebase();

  Serial.println("✅ PIR on GPIO " + String(PIR_PIN));
}

void loop() {

  handlePIR();
  applyRelayOutputs();
  pollFirebaseIfNeeded();

  delay(5);
}


