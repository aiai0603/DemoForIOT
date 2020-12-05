#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include"SSD1306Wire.h"
#define WLAN_SSID   "Mi 10000 Ultra"
#define WLAN_PASS   "88888888"
#define AIO_SERVER    "47.100.136.15"
#define AIO_SERVERPORT   1883
#define AIO_USERNAME   ""
#define AIO_KEY        ""
#include<dht11.h>
#include<ArduinoJson.h>
#include<Wire.h>

SSD1306Wire display(0x3c,2,14);
#define Addr 0x53
const uint8_t scl=2;
const uint8_t sda=14;



dht11 DHT11;
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

StaticJsonDocument<200> doc;
StaticJsonDocument<200> led;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname> 
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "CJP/RE");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe  onMSG = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "CJP/MSG");

Adafruit_MQTT_Subscribe  onLED = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "CJP/LED");

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug). 
void MQTT_connect();

void setup() {

Serial.begin(115200); 
delay(10);
display.init();
pinMode(12, OUTPUT); 
pinMode(13, OUTPUT); 
pinMode(15, OUTPUT); 
 

Serial.println(F("Adafruit MQTT demo"));

// Connect to WiFi access point. 
Serial.println(); 
Serial.println(); 
Serial.print("Connecting to "); 
Serial.println(WLAN_SSID);

WiFi.begin(WLAN_SSID, WLAN_PASS);
while (WiFi.status() != WL_CONNECTED) { delay(500);
Serial.print(".");
}
Serial.println();

Serial.println("WiFi connected");
Serial.println("IP address: "); Serial.println(WiFi.localIP());

// Setup MQTT subscription for onoff feed. 
mqtt.subscribe(&onLED);
mqtt.subscribe(&onMSG);
}
uint32_t x=0; 
void loop() {
    int chk = DHT11.read(5);
// Ensure the connection to the MQTT server is alive (this will make the first
//  connection  and automatically reconnect when  disconnected).  See the MQTT_connect
// function definition further below. 
MQTT_connect();

// this is our 'wait for incoming subscription packets' busy subloop
// try to spend your time here

Adafruit_MQTT_Subscribe *subscription;
while ((subscription = mqtt.readSubscription(5000))) { 
  if (subscription == &onMSG) {
  Serial.println((char*)onLED.lastread);
  display.flipScreenVertically();
  display.clear();
  display.drawString(0,20,(char*)onMSG.lastread);
  display.display();
  delay(2000);

  
  }else  if (subscription == &onLED) {
      Serial.println((char*)onLED.lastread);
      deserializeJson(led,(char*)onLED.lastread);
      analogWrite(12, led["valueR"]);   
      analogWrite(13, led["valueG"]);
      analogWrite(15, led["valueB"]);

  
  }
 }
// Now we can publish stuff! 
Serial.print(F("\nSending message!")); 
Serial.print("...");
String output;
doc["humidity"] = (float)DHT11.humidity;
doc["temperature"] = (float)DHT11.temperature;
serializeJson(doc ,output);

if (! photocell.publish(output.c_str())) { Serial.println(F("Failed"));
} else {
Serial.println(F("OK!"));
 
 
}

// ping the server to keep the mqtt connection alive
// NOT required if you are publishing once every KEEPALIVE seconds
/*
if(! mqtt.ping()) { mqtt.disconnect();
}
*/
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting. 
void MQTT_connect() {
int8_t ret;

// Stop if already connected. 
if (mqtt.connected()) {
return;
}
Serial.print("Connecting to MQTT... "); uint8_t retries = 3;
while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected 
  Serial.println(mqtt.connectErrorString(ret));
Serial.println("Retrying MQTT connection in 5 seconds..."); mqtt.disconnect();
delay(5000);  // wait 5 seconds retries--;
if (retries == 0) {
// basically die and wait for WDT to reset me 
while (1);
}
}
Serial.println("MQTT Connected!");
}
