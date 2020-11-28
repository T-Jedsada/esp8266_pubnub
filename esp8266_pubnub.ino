// PubNub MQTT example using ESP8266.
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
// Connection info.
const char* ssid = "YOUR_NETWORK_SSID";
const char* password =  "YOUR_NETWORK_PASSWORD";
const char* mqttServer = "mqtt.pndsn.com";
const int mqttPort = 1883;
const char* clientID = "YOUR_PUB_KEY_HERE/YOUR_SUB_KEY_HERE/CLIENT_ID";
const char* channelName = "hello_world";
int state = 0;
WiFiClient MQTTclient;
PubSubClient client(MQTTclient);
void callback(char* topic, byte* payload, unsigned int length) {
  String payload_buff;
  for (int i = 0; i < length; i++) {
    payload_buff = payload_buff + String((char)payload[i]);
    if (payload_buff == "\"on\"") {
      state = 1;
      digitalWrite(LED_BUILTIN, LOW);
    } else if (payload_buff == "\"off\"") {
      state = 0;
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
  Serial.println(payload_buff); // Print out messages.
}
long lastReconnectAttempt = 0;
long lastState = 0;
boolean reconnect() {
  if (client.connect(clientID)) {
    client.subscribe(channelName); // Subscribe.
  }
  return client.connected();
}
void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Attempting to connect...");
  WiFi.begin(ssid, password); // Connect to WiFi.
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Couldn't connect to WiFi.");
    while (1) delay(100);
  }
  client.setServer(mqttServer, mqttPort); // Connect to PubNub.
  client.setCallback(callback);
  lastReconnectAttempt = 0;
}
void loop() {
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) { // Try to reconnect.
      lastReconnectAttempt = now;
      if (reconnect()) { // Attempt to reconnect.
        lastReconnectAttempt = 0;
      }
    }
  } else { // Connected.
    Serial.println("PubNub is connected");
    client.loop();
    long now = millis();
    if (now - lastState > 4000) {
      lastState = now;
      if (state == 1) {
        client.publish(channelName, "{\"state\": \"on\"}");
      } else {
        client.publish(channelName, "{\"state\": \"off\"}");
      }
    }
    delay(1000);
  }
}
