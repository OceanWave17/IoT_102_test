/*
This code connects an ESP32 to ThingsBoard using MQTT and sends temperature and humidity readings from a DHT22 sensor.
*/
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// DHT sensor configuration
#define DHT_PIN 4      // Pin connected to the DHT sensor
#define DHT_TYPE DHT11 // DHT sensor type

// WiFi credentials
const char *ssid = "MBL - IOT";
const char *password = "iotpassword";

// ThingsBoard server and access token
const char *thingsboardServer = "159.223.80.40"; // Replace with your ThingsBoard server
const int mqttPort = 1883;
const char *accessToken = "lmi9i60shg21zjtopfew";        // Replace with your device access token

// WiFi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// DHT sensor object
DHT dht(DHT_PIN, DHT_TYPE);

// Timer variables
unsigned long previousMillis = 0;
const unsigned long interval = 5000; // Interval to send data (5 seconds)

void connectWifi()
{
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void connectMqtt()
{
    client.setServer(thingsboardServer, mqttPort);

    while (!client.connected())
    {
        Serial.println("Connecting to ThingsBoard...");

        if (client.connect("ESP32", accessToken, NULL))
        {
            Serial.println("Connected to ThingsBoard");
        }
        else
        {
            Serial.print("Failed to connect. State: ");
            Serial.println(client.state());
            delay(2000);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Connecting to ThingsBoard");

    connectWifi();
    connectMqtt();

    dht.begin();
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        connectWifi();
    }

    if (!client.connected())
    {
        connectMqtt();
    }

    client.loop();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();

        if (isnan(temperature) || isnan(humidity))
        {
            Serial.println("Failed to read from DHT sensor!");
            return;
        }

        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print(" °C, Humidity: ");
        Serial.print(humidity);
        Serial.println(" %");

        // Create JSON payload
        String payload = "{";
        payload += "\"temperature\":" + String(temperature) + ",";
        payload += "\"humidity\":" + String(humidity);
        payload += "}";

        // Publish data to ThingsBoard
        if (client.publish("SSN/Project/IoT102/test/telemetry", payload.c_str()))
        {
            Serial.println("Data sent successfully");
        }
        else
        {
            Serial.println("Failed to send data");
        }

        previousMillis = currentMillis;
    }
}