/*
The first section of the code sets up the PubSubClient.h library and the WiFi.h library
for establishing a connection between the ESP32 board and the MQTT broker
and sets up DHT.h library for using DHT sensor.
*/
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// DHT sensor configuration
#define DHT_PIN 4      // Pin connected to the DHT sensor (change accordingly)
#define DHT_TYPE DHT22 // DHT sensor type (now we're using DHT22)

// Set the name and password of the WiFi network you want to connect to
const char *ssid = "MBL - IOT";
const char *password = "iotpassword";

// Set the MQTT server and port to connect to
const char *mqttServer = "MQTT_BROKER";
const int mqttPort = 1883;

String HUMI_TOPIC = "Humidity";
String TEMP_TOPIC = "Temperature";
String SUB_TOPIC = "LED/Topic";

// Define the necessary variables and objects
WiFiClient espClient;
PubSubClient client(espClient);

// Create a DHT object
DHT dht(DHT_PIN, DHT_TYPE);

unsigned long previousMillis = 0;    // Timer variables
const unsigned long interval = 5000; // Interval of 5 seconds

// Declare a callback function
void MQTTcallback(char *topic, byte *payload, unsigned int length);

// Function to convert MAC address to String format
String macToStr(const uint8_t *mac)
{
  String result;
  for (int i = 0; i < 6; ++i)
  {
    result += String(mac[i], HEX);
    if (i < 5)
      result += ':';
  }
  return result;
}

// Function to connect to WiFi
void connectWifi()
{
  // Start the WiFi library
  WiFi.begin(ssid, password);

  // Print "Connecting to WiFi.." while the WiFi is connecting
  Serial.print("WiFi: Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Print the SSID and status of the connected WiFi network
  Serial.println("");
  Serial.print("WiFi SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("WiFi Status: ");
  Serial.println("Connected");
  delay(2000);
}

// Function to connect to MQTT broker
void connectMqtt()
{

  // Set the MQTT server and port
  client.setServer(mqttServer, mqttPort);

  // Set the callback function for handling incoming MQTT messages
  client.setCallback(MQTTcallback);

  // Attempt to connect to the MQTT broker
  while (!client.connected())
  {
    Serial.println("");
    Serial.println("Connecting to MQTT...");

    String clientName;
    clientName += "esp32  || MacAddress is ";
    uint8_t mac[6];
    WiFi.macAddress(mac);
    clientName += macToStr(mac);

    Serial.print("Mqtt Server: ");
    Serial.println(mqttServer);
    Serial.print("Clientname: ");
    Serial.println(clientName);

    // If the connection is successful
    if (client.connect((char *)clientName.c_str()))
    {
      Serial.print("Mqtt Status: ");
      Serial.println("connected");
    }

    // If the connection fails
    else
    {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }

  // Topic that we want to subscribe to receive messages for controlling the LED
  client.subscribe(SUB_TOPIC.c_str());
}

// Create function to handle the received MQTT message
void MQTTcallback(char *topic, byte *payload, unsigned int length)
{
  // Print the subscribed topic
  Serial.print("Message arrived in topic: ");
  Serial.print(topic);

  Serial.print("Message: ");

  String message;
  for (int i = 0; i < length; i++)
  {
    message = message + (char)payload[i];
  }

  // Print the received message content
  Serial.print(message);
  Serial.println();
  Serial.println("-----------------------");

  // If the received message is "on"
  if (message == "on")
  {
    // Turn on the built-in LED
    digitalWrite(LED_BUILTIN, HIGH);
  }
  // If the received message is "off"
  else if (message == "off")
  {
    // Turn off the built-in LED
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Lab #10: Node-red UI");
  connectWifi();
  connectMqtt();
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize the DHT sensor
  dht.begin();
}

void loop()
{
  // Reconnect wifi if the connection is lost
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWifi();
  }

  // Reconnect to MQTT broker if the connection is lost
  if (!client.connected())
  {
    connectMqtt();
  }

  // Maintain the MQTT connection
  client.loop();

  // Check if the interval has elapsed
  unsigned long currentMillis = millis();

  // Send sensor value every 2 seconds
  if (currentMillis - previousMillis >= interval)
  {
    // Read temperature and humidity from the DHT sensor
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Check if any reading failed
    if (isnan(temperature) || isnan(humidity))
    {
      Serial.println("Failed to read data from DHT sensor!");
    }

    // Print the temperature and humidity to serial monitor
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C\t");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Publish and display the data
    client.publish(TEMP_TOPIC.c_str(), String(temperature).c_str());
    client.publish(HUMI_TOPIC.c_str(), String(humidity).c_str());

    // Update the previousMillis variable
    previousMillis = currentMillis;
  }
}