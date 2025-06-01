#include <WiFiNINA.h>
#include <PubSubClient.h>

// Wi-Fi credentials
const char* wifiSSID = "DODO-629E";
const char* wifiPassword = "VBUZXWKG92";

// MQTT broker settings
const char* mqttHost = "mbea1725.ala.eu-central-1.emqxsl.com";
const int mqttPort = 8883;
const char* mqttUsername = "kaveen";
const char* mqttPassword = "1234";
const char* mqttTopic = "pub/Wave";

// Hardware pin assignments
const int ledPin = 2;
const int ultrasonicTrigPin = 9;
const int ultrasonicEchoPin = 10;

// MQTT and Wi-Fi clients
WiFiSSLClient secureWiFiClient;
PubSubClient mqttClient(secureWiFiClient);

// Timing control
unsigned long lastGestureTime = 0;
const unsigned long gestureDelay = 3000;

void setup() {
  Serial.begin(9600);

  pinMode(ultrasonicTrigPin, OUTPUT);
  pinMode(ultrasonicEchoPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // Connect to Wi-Fi
  WiFi.begin(wifiSSID, wifiPassword);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");

  // Setup MQTT server and callback
  mqttClient.setServer(mqttHost, mqttPort);
  mqttClient.setCallback(onMqttMessage);
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectToMQTT();
  }
  mqttClient.loop();
  checkForHandGesture();
}

// Reconnect to MQTT broker
void reconnectToMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT broker...");
    if (mqttClient.connect("arduinoClient", mqttUsername, mqttPassword)) {
      Serial.println("connected");
      mqttClient.subscribe(mqttTopic);
    } else {
      Serial.print("Connection failed, rc=");
      Serial.print(mqttClient.state());
      delay(5000);
    }
  }
}

// Handle incoming MQTT messages
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  Serial.print("Incoming message [");
  Serial.print(topic);
  Serial.print("]: ");

  String receivedMsg;
  for (unsigned int i = 0; i < length; i++) {
    receivedMsg += (char)payload[i];
  }

  receivedMsg.trim();
  receivedMsg.toLowerCase();

  Serial.println(receivedMsg);

  if (receivedMsg == "wave") {
    Serial.println("Executing wave pattern...");
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH);
      delay(300);
      digitalWrite(ledPin, LOW);
      delay(300);
    }
  } else {
    Serial.println("Unrecognized command");
  }
}

// Check for hand gesture using ultrasonic sensor
void checkForHandGesture() {
  digitalWrite(ultrasonicTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(ultrasonicTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultrasonicTrigPin, LOW);

  long pulseDuration = pulseIn(ultrasonicEchoPin, HIGH);
  float measuredDistance = pulseDuration * 0.034 / 2;

  if (measuredDistance > 0 && measuredDistance < 30 && millis() - lastGestureTime > gestureDelay) {
    Serial.println("Gesture detected!");
    mqttClient.publish(mqttTopic, "wave");
    lastGestureTime = millis();
  }
}
