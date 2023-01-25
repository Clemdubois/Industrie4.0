#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include "ESP8266WiFi.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 4
#define DHTTYPE DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

const char* ssid = "TheLabIOT"; //Enter SSID
const char* password = "Yaay!ICanTalkNow"; //Enter Password
#define AIO_SERVER      "192.168.1.4"
#define AIO_SERVERPORT  1883
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, "admin", "admin");
Adafruit_MQTT_Publish group = Adafruit_MQTT_Publish(&mqtt, "ESIEA/grp3");
void MQTT_connect();

void setup(void)
{ 
  Serial.begin(9600);
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  delayMS = sensor.min_delay / 1000;
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
     delay(500);
     Serial.print("*");
  }
  
  Serial.println("");
  Serial.println("WiFi connection Successful");
  Serial.print("The IP Address of ESP8266 Module is: ");
  Serial.print(WiFi.localIP());// Print the IP address
}

void loop() 
{
  delay(delayMS);
  MQTT_connect();
  // Get temperature event and print its value.
  sensors_event_t temp_event;
  sensors_event_t humid_event;
  dht.temperature().getEvent(&temp_event);
  dht.humidity().getEvent(&humid_event);
  if (isnan(temp_event.temperature) || isnan(humid_event.relative_humidity)) {
    Serial.println(F("Error reading temperature or humidity!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(temp_event.temperature);
    Serial.println(F("°C"));
    Serial.print(F("Humidity: "));
    Serial.print(humid_event.relative_humidity);
    Serial.println(F("%"));

    DynamicJsonDocument json(1024);
    json["temperature"] = String(temp_event.temperature);
    json["humidity"] = String(humid_event.relative_humidity);

    char charBuf[256];
    String output;
    serializeJson(json, output);
    output.toCharArray(charBuf, 256);
    if (! group.publish(charBuf)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }
  }
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}