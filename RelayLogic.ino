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

void applyRelayOutputs() {
  bool s1 = relayInSensors(sensorsList, "relay1");
  bool s2 = relayInSensors(sensorsList, "relay2");
  bool s3 = relayInSensors(sensorsList, "relay3");
  bool s4 = relayInSensors(sensorsList, "relay4");

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
}

