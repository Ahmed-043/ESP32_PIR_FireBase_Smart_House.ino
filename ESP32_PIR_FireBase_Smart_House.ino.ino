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

static bool relayInSensors(const String &list, const char *name) {
  String s = list;
  s.toLowerCase();
  s.replace(" ", "");
  s.replace("\"", "");
  s.replace("[", "");
  s.replace("]", "");
  s.replace("{", "");
  s.replace("}", "");

  String token = String(name);
  token.toLowerCase();

  int start = 0;
  while (start < s.length()) {
    int comma = s.indexOf(',', start);
    if (comma == -1) comma = s.length();
    String part = s.substring(start, comma);
    part.trim();
    if (part == token) return true;
    start = comma + 1;
  }
  return false;
}

static void writeRelay(uint8_t pin, bool on) {
  digitalWrite(pin, on ? LOW : HIGH); // active LOW
}

static void logRelayChange(int n, bool on) {
  Serial.println(String(on ? "✅ " : "❌ ") + "Relay" + String(n) + ": " + (on ? "ON" : "OFF"));
}

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

  // Connect Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, HIGH);
    Serial.print(".");
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }
  digitalWrite(LED_PIN, LOW);
  Serial.println("\n✅ Wi-Fi Connected: " + String(WiFi.localIP()));

  // Firebase setup
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Wait for Firebase
  Serial.println("Waiting for Firebase connection...");
  unsigned long start = millis();
  while (!Firebase.ready()) {
    Serial.print(".");
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    delay(250);
    if (millis() - start > 15000) {
      Serial.println("\n❌ Firebase connection failed!");
      break;
    }
  }
  if (Firebase.ready()) Serial.println("\n✅ Firebase Connected!");
  digitalWrite(LED_PIN, LOW);

  Serial.println("✅ PIR on GPIO " + String(PIR_PIN));
}

void loop() {
  // -------------------
  // 1) PIR event handling (FAST, no Firebase here)
  // -------------------
  bool pirLevel = (digitalRead(PIR_PIN) == HIGH);

  // Detect rising edge: LOW -> HIGH (motion event)
  if (pirLevel && !prevPirLevel) {
    Serial.println("✅ PIR event: HIGH");

    // Start/reset timer
    sensorActive = true;
    sensorOffAtMs = millis() + (unsigned long)timeoutSec * 1000UL;

    Serial.println("✅ Timer started/reset: " + String(timeoutSec) + " sec");
  }
  prevPirLevel = pirLevel;

  // Check timer expiry
  if (sensorActive) {
    // handle millis overflow safely
    long diff = (long)(millis() - sensorOffAtMs);
    if (diff >= 0) {
      sensorActive = false;
      Serial.println("❌ Timer expired: releasing sensor relays");
    }
  }

  // -------------------
  // 2) Apply relay outputs (FAST)
  // -------------------
  bool s1 = relayInSensors(sensorsList, "relay1");
  bool s2 = relayInSensors(sensorsList, "relay2");
  bool s3 = relayInSensors(sensorsList, "relay3");
  bool s4 = relayInSensors(sensorsList, "relay4");

  // If sensorActive, force ON only the relays in sensors list.
  // Otherwise, output follows manual Firebase state.
  bool out1 = sensorActive ? (s1 ? true : prevRelay1) : prevRelay1;
  bool out2 = sensorActive ? (s2 ? true : prevRelay2) : prevRelay2;
  bool out3 = sensorActive ? (s3 ? true : prevRelay3) : prevRelay3;
  bool out4 = sensorActive ? (s4 ? true : prevRelay4) : prevRelay4;

  if (out1 != prevOut1) { prevOut1 = out1; logRelayChange(1, out1); }
  if (out2 != prevOut2) { prevOut2 = out2; logRelayChange(2, out2); }
  if (out3 != prevOut3) { prevOut3 = out3; logRelayChange(3, out3); }
  if (out4 != prevOut4) { prevOut4 = out4; logRelayChange(4, out4); }

  writeRelay(RELAY1, out1);
  writeRelay(RELAY2, out2);
  writeRelay(RELAY3, out3);
  writeRelay(RELAY4, out4);

  // -------------------
  // 3) Poll Firebase occasionally (SLOW network stuff)
  // -------------------
  if (millis() - lastFirebasePoll >= FIREBASE_POLL_MS) {
    lastFirebasePoll = millis();

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("❌ Wi-Fi disconnected!");
      digitalWrite(LED_PIN, HIGH);
      return;
    } else {
      digitalWrite(LED_PIN, LOW);
    }

    if (!Firebase.ready()) {
      Serial.println("❌ Firebase not ready!");
      return;
    }

    // sensors list
    if (Firebase.RTDB.getString(&fbdo, "/sensors")) {
      String newList = fbdo.stringData();
      if (newList != sensorsList) {
        sensorsList = newList;
        Serial.println("✅ Sensors config: " + sensorsList);
      }
    } else {
      Serial.println("❌ Failed to read sensors: " + fbdo.errorReason());
    }

    // timeout (seconds)
    // Accept int; if not found keep existing timeoutSec
    if (Firebase.RTDB.getInt(&fbdo, "/timeout")) {
      int t = fbdo.intData();
      if (t < 1) t = 1;
      if ((uint32_t)t != timeoutSec) {
        timeoutSec = (uint32_t)t;
        Serial.println("✅ Timeout updated: " + String(timeoutSec) + " sec");
      }
    } else {
      // optional log; keep simple
      // Serial.println("❌ Failed to read timeout: " + fbdo.errorReason());
    }

    // manual relays
    if (Firebase.RTDB.getBool(&fbdo, "/relay1")) {
      bool r1 = fbdo.boolData();
      if (r1 != prevRelay1) { prevRelay1 = r1; Serial.println("✅ Relay1 manual: " + String(r1)); }
    } else Serial.println("❌ Failed to read relay1: " + fbdo.errorReason());

    if (Firebase.RTDB.getBool(&fbdo, "/relay2")) {
      bool r2 = fbdo.boolData();
      if (r2 != prevRelay2) { prevRelay2 = r2; Serial.println("✅ Relay2 manual: " + String(r2)); }
    } else Serial.println("❌ Failed to read relay2: " + fbdo.errorReason());

    if (Firebase.RTDB.getBool(&fbdo, "/relay3")) {
      bool r3 = fbdo.boolData();
      if (r3 != prevRelay3) { prevRelay3 = r3; Serial.println("✅ Relay3 manual: " + String(r3)); }
    } else Serial.println("❌ Failed to read relay3: " + fbdo.errorReason());

    if (Firebase.RTDB.getBool(&fbdo, "/relay4")) {
      bool r4 = fbdo.boolData();
      if (r4 != prevRelay4) { prevRelay4 = r4; Serial.println("✅ Relay4 manual: " + String(r4)); }
    } else Serial.println("❌ Failed to read relay4: " + fbdo.errorReason());
  }

  delay(5);
}