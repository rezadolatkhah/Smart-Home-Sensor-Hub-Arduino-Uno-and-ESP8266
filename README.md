# 🏠 Smart Home Sensor Hub (Arduino Uno & ESP8266)
Click on the image to watch the video.
[![Watch the video](Photos/Thumbnail.jpg)](https://www.youtube.com/watch?v=cwjAtl-GNTM)
     
This project combines an Arduino Uno-based Sensor Hub and an ESP8266-based MQTT Gateway on a single custom board for compact and integrated monitoring.  
For users who do not have a combined board, a separate Arduino Uno and ESP8266 can also be connected following the step-by-step instructions provided in the setup section.
 
It monitors temperature, gas levels(LPG, i-butane, propane, **methane**, alcohol, Hydrogen, **smoke**), and vibrations(earthquakes), and triggers alarms when thresholds are exceeded.  

The system is designed for **safety monitoring in homes, labs, or workplaces** where gas leaks, high temperatures, or vibrations (e.g., earthquakes) could pose a risk.

---

## ✨ Features

- 🌡️ **Temperature Monitoring**: DS18B20 sensor with 11-bit resolution.  
- 🧪 **Gas Detection**: MQ2 (smoke/flammable gases) & MQ7 (carbon monoxide).  
- 📳 **Vibration Detection**: Detects sudden vibrations (e.g., earthquakes).  
- 🔔 **Alarm System**: LCD + buzzer alerts when thresholds are exceeded.  
- 🖥️ **Live Data Output**:  
  - Serial output in JSON format every second from Arduino.  
  - ESP8266 publishes JSON messages to the MQTT broker.  
  - Separate MQTT topics for **normal data** (`myhome/data`) and **alarms** (`myhome/alarm`).  
- 🌐 **Wi-Fi Resilience**: ESP8266 automatically reconnects to Wi-Fi and MQTT without blocking.  

---

## ℹ️ Why MQTT?

This project was originally planned to use Arduino IoT Cloud for data monitoring.
However, in Iran, Arduino Cloud services are not accessible — even with DNS, VPN, or other workarounds.
To overcome this limitation, the system was designed with MQTT using a public broker, which is globally accessible and integrates easily with dashboards or mobile apps.

---

## 🔌 Hardware Requirements

![Project Overview](Photos/ProjectOverview.jpg)   

**Arduino Uno Sensor Hub:**
- Arduino Uno  
- DS18B20 Temperature Sensor (with 4.7kΩ pull-up)  
- MQ2 Gas Sensor  
- MQ7 Gas Sensor  
- SW-420 Vibration Sensor (digital)  
- 16x2 LCD Display
- 10kΩ potentiometer for LCD contrast  
- Buzzer  
- Mini Breadboard + jumper wires  

**ESP8266 MQTT Gateway:**
- ESP8266 (NodeMCU, Wemos D1 Mini, etc.)  
- USB connection to Arduino Uno  
- Wi-Fi network  

Note: The system is implemented on a single combined board integrating the Arduino Uno sensor hub and ESP8266 MQTT gateway, simplifying wiring and assembly.

---

## 💻 Software Requirements

- **Arduino IDE** (or compatible platform)  
- **Libraries**:  
  - DallasTemperature (for DS18B20)  
  - OneWire (for DS18B20 communication)  
  - LiquidCrystal (for LCD control)  
  - PubSubClient (for MQTT on ESP8266)  
  - ArduinoJson (for JSON formatting on ESP8266)  

---

## ⚙️ Installation

**1. Arduino Uno Sensor Hub**
1. Connect sensors, LCD, and buzzer to Arduino Uno according to the pin table.  
2. Add 4.7kΩ pull-up resistor for DS18B20.  
3. Install libraries via Arduino Library Manager.  
4. Upload the Arduino sketch.  
5. Open the Serial Monitor to verify data output.

**2. ESP8266 MQTT Gateway**
1. Connect the ESP8266 to the Arduino Uno via the Serial (TX/RX).  
2. Configure Wi-Fi SSID/password and MQTT broker info in the ESP8266 sketch.  
3. Install required libraries (`ESP8266WiFi`, `PubSubClient`, `ArduinoJson`).  
4. Upload the ESP8266 sketch.  
5. Verify MQTT messages on your broker (separate topics for data and alarms).  

---

## 📍 Pin Connections

![Pin Connections](Photos/Pin_Connections.jpg)

| Component          | Arduino Pin / Connection |
|--------------------|--------------------------|
| DS18B20            | D11                      |
| MQ2 Gas Sensor     | A0                       |
| MQ7 Gas Sensor     | A1                       |
| Vibration Sensor   | D2                       |
| Buzzer             | D8                       |
| LED Alarm          | D9                       |
| LCD RS             | D7                       |
| LCD E (Enable)     | D6                       |
| LCD D4             | D5                       |
| LCD D5             | D4                       |
| LCD D6             | D3                       |
| LCD D7             | D2                       |
| LCD VCC            | +5V                      |
| LCD GND            | GND                      |
| LCD V0 (Contrast)  | Middle pin of 10k pot    |
| Potentiometer Left | +5V                      |
| Potentiometer Right| GND                      |
| LCD A (Backlight+) | +5V (via resistor)       |
| LCD K (Backlight-) | GND                      |
| ESP8266 RX         | Arduino TX (D1/D0)*      |
| ESP8266 TX         | Arduino RX (D0/D1)*      |

- Connect to Wi-Fi and the MQTT broker

Note: The system is implemented on a single combined board integrating the Arduino Uno sensor hub and ESP8266 MQTT gateway, simplifying wiring and assembly.

---

## 📋 How It Works

1. **Arduino Uno** continuously reads:
   - Temperature (DS18B20)  
   - Gas levels (MQ2 & MQ7)  
   - Vibration sensor  
2. Displays temperature or alarm messages on **LCD**.  
3. Sends live **JSON-formatted data** to **Serial** every second.  
4. When an alarm triggers, Arduino also sends a simple `"ALARM:<code>"` message (e.g., `ALARM:1` for vibration, `ALARM:2` for MQ2, `ALARM:3` for MQ7).  
5. **ESP8266** reads Serial data from Arduino:
   - Parses JSON sensor data  
   - Detects `"ALARM:<code>"` messages  
   - Publishes structured JSON payloads to the MQTT broker  
   - Data goes to `myhome/data` and alarms to `myhome/alarm`  
6. Alarms last for 5 seconds on Arduino; ESP8266 immediately forwards alarms.  

---

## 🧩 Code Overview

### Arduino Uno (Sensor Hub)
- Reads DS18B20, MQ2, MQ7, and vibration sensor.  
- Displays values and alarms on 16x2 LCD.  
- Sends data to ESP8266 via Serial as **JSON-formatted output** every second.  
- Additionally sends `"ALARM:<code>"` messages when an alarm is triggered.  


### ESP8266 (Wi-Fi + MQTT Gateway)
- Connects to Wi-Fi and MQTT broker.  
- Reads serial messages from Arduino.  
- Publishes **JSON payloads**:  
  - Data messages: `{"type":"DATA","data":"..."}`
  - Alarm messages: `{"type":"ALARM","alarm":"...","data":"..."}`  
  - Connection message: `{"status":"connected"}`  
- Uses **separate topics** for alarms (`myhome/alarm`) and normal data (`myhome/data`).  
- Implements **non-blocking Wi-Fi reconnect** to avoid freezing the main loop.  
- Prevents timing drift by synchronizing the data send interval.  

---

## 📷 Example Outputs

![Screenshot.jpg](Photos/Screenshot1.jpg)   

**Arduino Serial (Normal DATA):**
{"TEMP":25.5,"MQ2":200,"MQ7":150,"VIB":0}

**Arduino Serial (ALARM triggered):**
ALARM:2
{"TEMP":26.0,"MQ2":400,"MQ7":180,"VIB":0}

**ESP8266 MQTT JSON Payload:**
```json
{
  "type": "ALARM",
  "alarm": "MQ2 Gas High",
  "TEMP": 26.0,
  "MQ2": 400,
  "MQ7": 180,
  "VIB": 0
}

or for normal data:
{
  "type": "DATA",
  "TEMP": 25.5,
  "MQ2": 200,
  "MQ7": 150,
  "VIB": 0
}
```

## 🔧 Customization

- ⚡ **MQ Sensors Thresholds**: Change MQ2/MQ7 trigger values in Arduino sketch.  
- ⏱️ **Alarm Duration**: Adjust `alarmDuration` in Arduino sketch.  
- 🖥️ **LCD Messages**: Customize the messages displayed in the Arduino sketch.  
- 📡 **Data Send Interval**: Modify Arduino Serial output frequency or ESP8266 MQTT publish interval (`sendInterval`).  
- 🌐 **Wi-Fi & MQTT Settings**: Configure SSID, password, broker address, and topics in the ESP8266 sketch.  
- 📝 **JSON Payload Structure**: Extend or modify JSON keys/values in the ESP8266 sketch (e.g., add timestamp, device ID, etc.).   

---

## ⚠️ Limitations

- 🌡️ Supports only **one DS18B20 sensor** on Arduino.  
- 💨 MQ2 and MQ7 sensors may need calibration for accurate readings in your environment.  
- 🌍 Vibration sensor is **digital only** (on/off), limiting sensitivity.  
- 🔔 Arduino uses a simple buzzer for alarms; complex sound patterns are not supported.  

---

## 🚀 Future Improvements

- 🌡️ Add multiple DS18B20 sensors for multi-point temperature monitoring.  
- 💾 Implement SD card or cloud logging for long-term data storage.  
- 🌍 Upgrade to an analog vibration sensor for higher sensitivity.  
- 🏠 Integrate with Home Assistant or Node-RED dashboards for visualization.  
- 🔄 Add a manual alarm reset button on Arduino.  

---

## 📄 License

This project is licensed under the [MIT License](https://github.com/rezadolatkhah/Smart-Home-Sensor-Hub-Arduino-Uno-and-ESP8266/blob/main/LICENSE.txt).  

---

## 🤝 Contributing

Contributions are welcome!  

- Submit a **pull request** for bug fixes, new features, or enhancements.  
- Open an **issue** if you encounter problems or have suggestions.  
- Feedback and improvements help make the project better for everyone!  
