#include "MqttClient.h"
#include <cstring>
#include <cstdio>

#if defined(ESP32) || defined(ESP8266)
// Use ESP-IDF style macros provided by esp32-hal-log.h (ESP_LOGI/ESP_LOGE/ESP_LOGW)
#define LOG_TAG "MqttClient"
#else
#define LOG_TAG "MqttClient"
#define ESP_LOGI(tag, format, ...) printf("[INFO] %s: " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGE(tag, format, ...) printf("[ERROR] %s: " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...) printf("[WARN] %s: " format "\n", tag, ##__VA_ARGS__)
#endif

MqttClient *MqttClient::_instance = nullptr;

MqttClient::MqttClient()
    : _brokerPort(1883), _username{0}, _password{0}, _keepalive(60),
      _isConnected(false), _messageCb(nullptr), _legacyConnectCb(nullptr), _legacyDisconnectCb(nullptr)
#if defined(ESP32) || defined(ESP8266)
      ,
      _pubSubClient(_wifiClient)
#else
      ,
      _pubSubClient(nullptr)
#endif
{
  _brokerUri[0] = 0;
  _brokerHost[0] = 0;
  _instance = this;
#if defined(ESP32) || defined(ESP8266)
  _pubSubClient.setCallback(onMessageReceived);
#endif
}

MqttClient::~MqttClient()
{
  if (_instance == this)
    _instance = nullptr;
#if defined(ESP32) || defined(ESP8266)
  if (_pubSubClient.connected())
    _pubSubClient.disconnect();
#endif
}

void MqttClient::parseUriComponents(const char *uri)
{
  const char *p = uri;
  if (std::strncmp(p, "mqtt://", 7) == 0)
    p += 7;
  else if (std::strncmp(p, "mqtts://", 8) == 0)
    p += 8;

  char buf[128];
  std::strncpy(buf, p, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = 0;

  char *colon = std::strchr(buf, ':');
  if (colon)
  {
    *colon = 0;
    std::strncpy(_brokerHost, buf, sizeof(_brokerHost) - 1);
    _brokerHost[sizeof(_brokerHost) - 1] = 0;
    _brokerPort = std::atoi(colon + 1);
  }
  else
  {
    std::strncpy(_brokerHost, buf, sizeof(_brokerHost) - 1);
    _brokerHost[sizeof(_brokerHost) - 1] = 0;
    _brokerPort = 1883;
  }
}

void MqttClient::begin(const char *brokerUri)
{
  std::strncpy(_brokerUri, brokerUri, sizeof(_brokerUri) - 1);
  _brokerUri[sizeof(_brokerUri) - 1] = 0;
  parseUriComponents(_brokerUri);
#if defined(ESP32) || defined(ESP8266)
  _pubSubClient.setServer(_brokerHost, _brokerPort);
  _pubSubClient.setKeepAlive(_keepalive);
#endif
  ESP_LOGI(LOG_TAG, "Initialized for %s:%d", _brokerHost, _brokerPort);
}

void MqttClient::begin(const char *brokerUri, const char *username, const char *password, uint16_t keepalive)
{
  begin(brokerUri);
  if (username)
  {
    std::strncpy(_username, username, sizeof(_username) - 1);
    _username[sizeof(_username) - 1] = 0;
  }
  if (password)
  {
    std::strncpy(_password, password, sizeof(_password) - 1);
    _password[sizeof(_password) - 1] = 0;
  }
  _keepalive = keepalive;
#if defined(ESP32) || defined(ESP8266)
  _pubSubClient.setKeepAlive(_keepalive);
#endif
}

bool MqttClient::connect(const char *clientId)
{
#if defined(ESP32) || defined(ESP8266)
  if (_pubSubClient.connected())
    return true;
  if (!clientId)
    clientId = "mqtt_client";
  bool result = false;
  if (_username[0] != 0)
    result = _pubSubClient.connect(clientId, _username, _password);
  else
    result = _pubSubClient.connect(clientId);
  if (result)
  {
    _isConnected = true;
    if (_legacyConnectCb)
      _legacyConnectCb();
  }
  else
  {
    _isConnected = false;
    ESP_LOGE(LOG_TAG, "Connect failed");
  }
  return result;
#else
  _isConnected = true;
  if (_legacyConnectCb)
    _legacyConnectCb();
  return true;
#endif
}

void MqttClient::disconnect()
{
#if defined(ESP32) || defined(ESP8266)
  if (_pubSubClient.connected())
    _pubSubClient.disconnect();
#endif
  _isConnected = false;
  if (_legacyDisconnectCb)
    _legacyDisconnectCb();
}

int MqttClient::publish(const char *topic, const char *payload, bool retain)
{
#if defined(ESP32) || defined(ESP8266)
  if (!_pubSubClient.connected() || !_isConnected)
  {
    ESP_LOGE(LOG_TAG, "Not connected");
    return -1;
  }
  bool ok = _pubSubClient.publish(topic, payload, retain);
  if (!ok)
  {
    ESP_LOGE(LOG_TAG, "Publish failed");
    return -1;
  }
  static int id = 1;
  return id++;
#else
  static int id = 1;
  return id++;
#endif
}

int MqttClient::subscribe(const char *topic, int qos)
{
#if defined(ESP32) || defined(ESP8266)
  if (!_pubSubClient.connected() || !_isConnected)
  {
    ESP_LOGE(LOG_TAG, "Not connected");
    return -1;
  }
  bool ok = _pubSubClient.subscribe(topic, qos);
  if (!ok)
  {
    ESP_LOGE(LOG_TAG, "Subscribe failed: %s", topic);
    return -1;
  }
  static int id = 1;
  return id++;
#else
  static int id = 1;
  return id++;
#endif
}

int MqttClient::unsubscribe(const char *topic)
{
#if defined(ESP32) || defined(ESP8266)
  if (!_pubSubClient.connected() || !_isConnected)
  {
    ESP_LOGE(LOG_TAG, "Not connected");
    return -1;
  }
  bool ok = _pubSubClient.unsubscribe(topic);
  if (!ok)
  {
    ESP_LOGE(LOG_TAG, "Unsubscribe failed: %s", topic);
    return -1;
  }
  static int id = 1;
  return id++;
#else
  static int id = 1;
  return id++;
#endif
}

void MqttClient::onMessage(MessageCallback cb) { _messageCb = cb; }
void MqttClient::onConnect(SimpleCallback cb) { _legacyConnectCb = cb; }
void MqttClient::onDisconnect(SimpleCallback cb) { _legacyDisconnectCb = cb; }
bool MqttClient::isConnected() const { return _isConnected; }

void MqttClient::onMessageReceived(char *topic, byte *payload, unsigned int length)
{
  if (_instance)
  {
    static char buf[512];
    unsigned int n = (length < sizeof(buf) - 1) ? length : sizeof(buf) - 1;
    std::memcpy(buf, payload, n);
    buf[n] = 0;
    _instance->handleMessage(topic, buf);
  }
}

void MqttClient::handleMessage(const char *topic, const char *payload)
{
  if (_messageCb)
    _messageCb(topic, payload);
}