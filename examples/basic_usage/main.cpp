#include <Arduino.h>
#include <WiFi.h>
#include "MqttClient.h"

// WiFi credentials
const char *ssid = "your_wifi_ssid";
const char *password = "your_wifi_password";

// MQTT broker settings
const char *mqtt_broker = "mqtt://test.mosquitto.org:1883";
const char *mqtt_username = "";
const char *mqtt_password = "";
const char *client_id = "esp32_client";

MqttClient mqttClient;

void onMqttMessage(const char *topic, const char *payload)
{
  Serial.printf("Received on %s: %s\n", topic, payload);
}

void onConnected()
{
  Serial.println("MQTT Connected");
  mqttClient.subscribe("test/esp32/+", 0);
}

void onDisconnected()
{
  Serial.println("MQTT Disconnected");
}

void connectToWiFi()
{
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.printf("Connected to WiFi. IP address: %s\n", WiFi.localIP().toString().c_str());
}

void setup()
{
  Serial.begin(115200);
  Serial.println("MQTT Client Example (MQTT 3.1.1)");
  connectToWiFi();

  mqttClient.onMessage(onMqttMessage);
  mqttClient.onConnect(onConnected);
  mqttClient.onDisconnect(onDisconnected);

  mqttClient.begin(mqtt_broker, mqtt_username, mqtt_password, 60);
  mqttClient.connect(client_id);
}

void loop()
{
  static unsigned long lastPublish = 0;
  static int counter = 0;

  if (mqttClient.isConnected() && millis() - lastPublish > 5000)
  {
    lastPublish = millis();
    char payload[64];
    snprintf(payload, sizeof(payload), "Hello MQTT %d", counter++);
    mqttClient.publish("test/esp32/data", payload, false);
  }

  delay(100);
}
