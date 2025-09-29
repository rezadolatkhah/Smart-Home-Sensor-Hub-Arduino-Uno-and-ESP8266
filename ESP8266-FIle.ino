// ===============================
//   ESP8266 WiFi + MQTT Gateway (Optimized)
//   Reads serial data from Arduino
//   Sends sensor data and alarms to the MQTT broker using JSON
// ===============================

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ----------- Wi-Fi Settings -----------
// Replace with your Wi-Fi credentials
const char* ssid = "Your_Wi-FI_SSID";          
const char* password = "Your_Wi-Fi_Password";  

// ----------- MQTT Settings -----------
// MQTT broker configuration and topics
const char* mqtt_server = "your_mqtt_server";   
const int   mqtt_port = 1883;                    
const char* mqtt_data_topic  = "your/topic1";     
const char* mqtt_alarm_topic = "your/topic2";     
const char* mqtt_client_id   = "your_client_id"; 

// WiFi and MQTT client objects
WiFiClient espClient;
PubSubClient client(espClient);

// ----------- Data Variables -----------
// lastData: latest JSON string read from Arduino (e.g. {"TEMP":...})
// lastAlarm: latest alarm code/string read from Arduino
String lastData = "";       
String lastAlarm = "";      
unsigned long lastSendTime = 0;               // timestamp of last periodic publish
const unsigned long sendInterval = 5000;    // how often to publish regular DATA (ms)

// Flag to ensure "Connected" message is published only once after first MQTT connect
bool firstConnect = true;   // برای اینکه "Connected" فقط یکبار ارسال بشه

// ===============================
//   Wi-Fi Connection Setup
//   Connect to local Wi-Fi network (blocking until connected)
// ===============================
void setup_wifi() {
  Serial.print("🔌 Connecting to Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // This loop blocks until Wi-Fi is connected — acceptable at startup
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Connected to Wi-Fi!");
  Serial.print("📡 Local IP: ");
  Serial.println(WiFi.localIP());
}

// ===============================
//   Reconnect to the MQTT Broker if disconnected
//   Tries to connect repeatedly until success
// ===============================
void reconnect() {
  while (!client.connected()) {
    Serial.print("🔄 Attempting MQTT connection...");

    if (client.connect(mqtt_client_id)) {
      Serial.println("✅ connected!");
      // Publish a simple "Connected" message only once on the first successful connect
      if (firstConnect) {  
        client.publish(mqtt_data_topic, "Connected");  
        firstConnect = false; 
      }
    } else {
      Serial.print("❌ failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000); 
    }
  }
}

// ===============================
//   Send MQTT Message (JSON)
//   Helper to publish and print status on Serial
// ===============================
void sendMQTT(const char* topic, const String& payload) {
  if (client.publish(topic, payload.c_str())) {
    Serial.println("📤 MQTT Sent: " + payload);
  } else {
    Serial.println("⚠️ Failed to send MQTT message!");
  }
}

// ===============================
//   Setup Function
//   Initialize Serial, Wi-Fi, and MQTT server
// ===============================
void setup() {
  Serial.begin(9600); 
  setup_wifi();       
  client.setServer(mqtt_server, mqtt_port); 
}

// ===============================
//   Main Loop
//   - maintain Wi-Fi & MQTT connections
//   - read Serial from Arduino (JSON data or ALARM lines)
//   - publish ALARM immediately with human-readable sensor name
//   - publish DATA periodically (sendInterval)
// ===============================
void loop() {
  // If Wi-Fi disconnected, try to (re)connect
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  // Ensure MQTT is connected; reconnect() will publish "Connected" only the first time
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read one line from Serial if available (Arduino should send one JSON or ALARM per line)
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n'); 
    msg.trim();  // remove whitespace/newline

    // If message starts with '{', treat it as raw JSON from Arduino (normal sensor data)
    if (msg.startsWith("{")) {  
      lastData = msg; 
      Serial.println("📥 Data Received: " + lastData);
    }
    // If message starts with "ALARM:" treat it as an alarm code sent by Arduino
    else if (msg.startsWith("ALARM:")) {  
      // Extract the code after "ALARM:"
      String alarmCode = msg.substring(6); 
      String alarmText = "";

      // Map numeric codes (sent by Arduino) to human-friendly alarm names
      if (alarmCode == "1") alarmText = "VIBRATION";
      else if (alarmCode == "2") alarmText = "MQ2 Gas";
      else if (alarmCode == "3") alarmText = "MQ7 CO Gas";
      else alarmText = "UNKNOWN";

      Serial.println("🚨 Alarm Received: " + alarmText);

      // Build JSON payload for the alarm containing:
      // { "type":"ALARM", "alarm":"<alarmText>", "data":"<lastData>" }
      // lastData may be empty if no JSON was received yet
      StaticJsonDocument<200> doc;
      doc["type"] = "ALARM";
      doc["alarm"] = alarmText;
      doc["data"] = lastData;
      char buffer[200];
      serializeJson(doc, buffer);
      sendMQTT(mqtt_alarm_topic, String(buffer));

      // Reset periodic timer so a DATA message isn't immediately sent again
      lastSendTime = millis();
    }
  }

  // Periodic sending of DATA: if enough time has passed and we have lastData
  if (millis() - lastSendTime >= sendInterval && lastData != "") {
    StaticJsonDocument<200> doc;
    doc["type"] = "DATA";
    doc["data"] = lastData;
    char buffer[200];
    serializeJson(doc, buffer);
    sendMQTT(mqtt_data_topic, String(buffer));

    // Increase lastSendTime by interval to prevent drift
    lastSendTime += sendInterval; 
  }
}
