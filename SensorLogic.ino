void handlePIR() {
  bool pirLevel = (digitalRead(PIR_PIN) == HIGH);

  if (pirLevel && !prevPirLevel) {
    Serial.println("✅ PIR event: HIGH");
    sensorActive = true;
    sensorOffAtMs = millis() + (unsigned long)timeoutSec * 1000UL;
    Serial.println("✅ Timer started/reset: " + String(timeoutSec) + " sec");
  }
  prevPirLevel = pirLevel;

  if (sensorActive) {
    long diff = (long)(millis() - sensorOffAtMs);
    if (diff >= 0) {
      sensorActive = false;
      Serial.println("❌ Timer expired: releasing sensor relays");
    }
  }
}

