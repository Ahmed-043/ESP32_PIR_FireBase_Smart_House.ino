# ESP32 Smart Home Automation

A simple **ESP32 Smart Home** project that controls 4 relays using **Firebase** and a **PIR motion sensor**.

## Features

* Control 4 relays from Firebase
* PIR motion detection
* Automatically turn selected relays ON when motion is detected
* Set the automatic ON time from Firebase
* Manual relay control

## Hardware

* ESP32
* 4-Channel Relay Module
* PIR Motion Sensor

## Pin Connections

| Component  | GPIO |
| ---------- | ---- |
| Relay 1    | 23   |
| Relay 2    | 19   |
| Relay 3    | 18   |
| Relay 4    | 5    |
| PIR Sensor | 27   |
| LED        | 2    |

> Relays are **Active LOW**.

## Firebase Data

```json
{
  "relay1": false,
  "relay2": false,
  "relay3": false,
  "relay4": false,
  "sensors": "relay1,relay2",
  "timeout": 60
}
```

## How It Works

* Firebase controls the manual ON/OFF state of the relays.
* The PIR sensor detects motion.
* When motion is detected, the relays listed in `sensors` turn ON.
* They stay ON for the number of seconds set in `timeout`.
* After the timeout, the relays return to their manual Firebase states.

## Required Libraries

```cpp
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
```

## Setup

1. Connect the ESP32, relays, and PIR sensor.
2. Install the required libraries in Arduino IDE.
3. Add your Wi-Fi and Firebase credentials.
4. Create the Firebase database values shown above.
5. Upload the code to the ESP32.

## Security

Do not upload your real Wi-Fi password, Firebase API key, email, or password to a public GitHub repository. Use a separate `secrets.h` file and add it to `.gitignore`.
