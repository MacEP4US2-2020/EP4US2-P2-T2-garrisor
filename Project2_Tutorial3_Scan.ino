/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

//*******WIFI CODE BEGIN*******
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <iostream>
#include <chrono>
#include <ctime> 
//*******WIFI CODE END*******

//*******WIFI CODE BEGIN*******
// Replace the next variables with your SSID/Password combination
const char* ssid = "92 Westwood";
const char* password = "mariocart";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "192.168.1.122";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      //digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      //digitalWrite(ledPin, LOW);
    }
  }
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Ronan's ESP32")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//*******WIFI CODE END*******

int scanTime = 15; //In seconds
float rssi; //initialize RSSI variable
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      //
      
      //Check beacon 1
      if (strcmp(advertisedDevice.getAddress().toString().c_str(), "24:62:ab:f9:65:c6") == 0) {
      //if (strcmp(advertisedDevice.getName().c_str(), "Ronan_Beacon1") == 0) { 
        float rssi = advertisedDevice.getRSSI(); // extract RSSI
        char result[8];
        dtostrf(rssi, 1, 2, result);
        client.publish("esp32/rssi", result);
        Serial.println(rssi); 

        if (rssi<-60) {
          const char* location = "Kitchen";
          Serial.println("Kitchen"); 
          client.publish("esp32/location", location);
        }
        else {
          const char* location = "Living Room";
          Serial.println("Living Room");
          client.publish("esp32/location", location);
        }
        
       }
      else { 
        const char* result = "Scanning...";
        const char* location = "Updating...";
        client.publish("esp32/rssi", result);
        client.publish("esp32/location", location);
                 
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  //*******WIFI CODE BEGIN*******
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //*******WIFI CODE BEGIN*******
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    // put your main code here, to run repeatedly:
    Serial.println("Scan started!");
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    //Serial.print("Devices found: ");
    //Serial.println(foundDevices.getCount());
    Serial.println("Scan done!");
    pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
    delay(2000);

    
    
  }
}
