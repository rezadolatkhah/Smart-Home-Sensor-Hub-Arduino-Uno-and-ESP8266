// ===============================
//   ESP8266 WiFi + MQTT Gateway (Optimized)
//   Reads serial data from Arduino
//   Sends sensor data and alarms to MQTT broker using JSON
// ===============================

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>   // For building JSON payloads

// ----------- Wi-Fi Settings -----------
const char* ssid = "Home";          
const char* password = "13781374";  

// ----------- MQTT Settings -----------
const char* mqtt_server = "broker.hivemq.com";   
const int   mqtt_port = 1883;                    
const char* mqtt_data_topic  = "myhome/data";     
const char* mqtt_alarm_topic = "myhome/alarm";     
const char* mqtt_client_id   = "ESP8266_Client_1"; 

// WiFi and MQTT client objects
WiFiClient espClient;
PubSubClient client(espClient);

// ----------- Data Variables -----------
String lastData = "";       
String lastAlarm = "";      
unsigned long lastSendTime = 0;              
const unsigned long sendInterval = 30000;    

// ===============================
//   Wi-Fi Connection Setup
// ===============================
void setup_wifi() {
  Serial.print("🔌 Connecting to Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Wait until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Connected to Wi-Fi!");
  Serial.print("📡 Local IP: ");
  Serial.println(WiFi.localIP());
}

// ===============================
//   Reconnect to MQTT Broker if disconnected
// ===============================
void reconnect() {
  while (!client.connected()) {
    Serial.print("🔄 Attempting MQTT connection...");

    if (client.connect(mqtt_client_id)) {
      Serial.println("✅ connected!");
      client.publish(mqtt_data_topic, "Connected"); // Publish initial message
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
// ===============================
void setup() {
  Serial.begin(9600); 
  setup_wifi();       
  client.setServer(mqtt_server, mqtt_port); 
}

// ===============================
//   Main Loop
// ===============================
void loop() {
  // --- Ensure Wi-Fi is connected ---
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  // --- Ensure MQTT connection is alive ---
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // --- Read incoming serial data from Arduino ---
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n'); 
    msg.trim();                                

    // Handle incoming "DATA" message
    if (msg.startsWith("DATA:")) {  // Use simple tag without emoji
      lastData = msg.substring(5); 
      Serial.println("📥 Data Received: " + lastData);
    }
    // Handle incoming "ALARM" message
    else if (msg.startsWith("ALARM:")) {  
      lastAlarm = msg.substring(6); 
      Serial.println("🚨 Alarm Received: " + lastAlarm);

      // Immediately send alarm via MQTT (JSON)
      StaticJsonDocument<200> doc;
      doc["type"] = "ALARM";
      doc["alarm"] = lastAlarm;
      doc["data"] = lastData;
      char buffer[200];
      serializeJson(doc, buffer);
      sendMQTT(mqtt_alarm_topic, String(buffer));

      lastSendTime = millis(); // Reset timer to avoid duplicate send
    }
  }

  // --- Periodic data sending ---
  if (millis() - lastSendTime >= sendInterval && lastData != "") {
    StaticJsonDocument<200> doc;
    doc["type"] = "DATA";
    doc["data"] = lastData;
    char buffer[200];
    serializeJson(doc, buffer);
    sendMQTT(mqtt_data_topic, String(buffer));

    lastSendTime += sendInterval; // Prevent drift
  }
}
