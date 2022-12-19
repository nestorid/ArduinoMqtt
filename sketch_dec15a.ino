#include <SPI.h>
#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include "arduino_secrets.h"
#include "DHT.h"

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

char ssid[] = SECRET_SSID;     //  your network SSID (name)
char pass[] = SECRET_PASS;    // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
DHT dht(DHTPIN, DHTTYPE);

const char broker[] = "iotlab.csd.auth.gr";
int        port     = 1883;
const char topic[]  = "v1/devices/me/telemetry";

//set interval for sending messages (milliseconds)
const long interval = 8000;
unsigned long previousMillis = 0;
int count = 0;

void setup() {

  // initialize serial and wait for the port to open:

  Serial.begin(9600);
  while(!Serial);

  // attempt to connect using WEP encryption:
  Serial.println("Initializing Wifi...");
  
  printMacAddress();
  // scan for existing networks:
  Serial.println("Scanning available networks...");
  listNetworks();
  delay(10000);
 
  connectToSSID();

  
  
  mqttClient.setId("arduino");
  mqttClient.setUsernamePassword("arduino", "arduino");
  delay(1000);
  if (!mqttClient.connect(broker, port)) {

    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);

  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

   Serial.println("Starting measurements...");
  dht.begin();
  
}



void printMacAddress() {

  // the MAC address of your Wifi shield

  byte mac[6];

  // print your MAC address:

  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.println(mac[0],HEX);
}

void listNetworks() {

  // scan for nearby networks:

  Serial.println("** Scan Networks **");

  byte numSsid = WiFi.scanNetworks();
  
  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);
  
  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet<numSsid; thisNet++) {

    Serial.print(thisNet);
    Serial.print(") ");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print("\tSignal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");

    Serial.print("\tEncryption: ");
    Serial.println(WiFi.encryptionType(thisNet));

  }
}


void connectToSSID() {

  // attempt to connect using WPA2 encryption:

  Serial.println("Attempting to connect to WPA network...");
  status = WiFi.begin(ssid, pass);
  // if you're not connected, stop here:
  if ( status != WL_CONNECTED) {

    Serial.println("Couldn't get a wifi connection");
    while(true);
  }

  // if you are connected, print out info about the connection:

  else {

    Serial.println("Connected to network");

  }

}

void loop() {

  mqttClient.poll();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {

    // save the last time a message was sent

    previousMillis = currentMillis;


    // Wait a few seconds between measurements.
     delay(2000);

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    Serial.print("Humidity: "); 
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: "); 
    Serial.print(t);
    Serial.println(" *C ");

    
    Serial.print("Sending message to topic: ");
    Serial.println(topic);
    Serial.println(h);
    

    // send message, the Print interface can be used to set the message contents

    String payload;

    payload += "{humidity :";
    payload += " ";
    payload += h;
    payload += ",temperature :";
    payload += " ";
    payload += t;
    payload += "}";

    mqttClient.beginMessage(topic);
    mqttClient.print(payload);
    mqttClient.endMessage();

    Serial.println();

  }
}
