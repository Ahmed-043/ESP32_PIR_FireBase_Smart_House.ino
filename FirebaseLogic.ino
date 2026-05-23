void connectWiFi() {
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
}

void initFirebase() {
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

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
}

void pollFirebaseIfNeeded() {
  if (millis() - lastFirebasePoll < FIREBASE_POLL_MS) return;
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

  if (Firebase.RTDB.getString(&fbdo, "/sensors")) {
    String newList = fbdo.stringData();
    if (newList != sensorsList) {
      sensorsList = newList;
      Serial.println("✅ Sensors config: " + sensorsList);
    }
  } else {
    Serial.println("❌ Failed to read sensors: " + fbdo.errorReason());
  }

  if (Firebase.RTDB.getInt(&fbdo, "/timeout")) {
    int t = fbdo.intData();
    if (t < 1) t = 1;
    if ((uint32_t)t != timeoutSec) {
      timeoutSec = (uint32_t)t;
      Serial.println("✅ Timeout updated: " + String(timeoutSec) + " sec");
    }
  }

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

