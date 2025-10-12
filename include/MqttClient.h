#pragma once

#include <functional>

#if defined(ESP32) || defined(ESP8266)
#include <WiFi.h>
#include <PubSubClient.h>
#elif defined(ARDUINO)
#include <Arduino.h>
#else
typedef unsigned char byte;
#endif

class MqttClient
{
public:
  using MessageCallback = std::function<void(const char *topic, const char *payload)>;
  using SimpleCallback = std::function<void(void)>;

  MqttClient();
  ~MqttClient();

  void begin(const char *brokerUri);
  void begin(const char *brokerUri, const char *username, const char *password, uint16_t keepalive = 60);

  bool connect(const char *clientId = nullptr);
  void disconnect();

  int publish(const char *topic, const char *payload, bool retain = false);
  int subscribe(const char *topic, int qos = 0);
  int unsubscribe(const char *topic);

  void onMessage(MessageCallback cb);
  void onConnect(SimpleCallback cb);
  void onDisconnect(SimpleCallback cb);

  bool isConnected() const;

private:
  char _brokerUri[128];
  char _brokerHost[64];
  int _brokerPort;
  char _username[64];
  char _password[64];
  uint16_t _keepalive;

#if defined(ESP32) || defined(ESP8266)
  WiFiClient _wifiClient;
  PubSubClient _pubSubClient;
#else
  void *_pubSubClient;
#endif

  bool _isConnected;

  MessageCallback _messageCb;
  SimpleCallback _legacyConnectCb;
  SimpleCallback _legacyDisconnectCb;

  static void onMessageReceived(char *topic, byte *payload, unsigned int length);
  void handleMessage(const char *topic, const char *payload);

  void parseUriComponents(const char *uri);
  static MqttClient *_instance;

  MqttClient(const MqttClient &) = delete;
  MqttClient &operator=(const MqttClient &) = delete;
};
