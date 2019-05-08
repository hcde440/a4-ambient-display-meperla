//Feather 2 
//Sensor input and MQTT code

#include <ESP8266WiFi.h>
#include "Wire.h"          
#include <PubSubClient.h>  
#include <ArduinoJson.h>  

#define wifi_ssid "Half-G Guest"               // Wifi network SSID
#define wifi_password "BeOurGuest"      // Wifi network password

#define mqtt_server "mediatedspaces.net"  //this is its address, unique to the server
#define mqtt_user "hcdeiot"               //this is its server login, unique to the server
#define mqtt_password "esp8266"           //this is it server password, unique to the server
#define topic_name "maria/bathroom"        //topic you are subscribing to

WiFiClient espClient;             //espClient
PubSubClient mqtt(espClient);     //tie PubSub (mqtt) client to WiFi client

#define DOOR 13    //Magnetic Contact Switch
#define PIR 2      // PIR Motion Sensor

//Variables to test change in state 
int curr_door;
int prev_door;
//starts assuming no motioin is detected
int pirState = LOW;
//variable for reading the pin status
int pirRead = 0;

char mac[6];
char message[201];
unsigned long currentMillis, timerOne, timerTwo, timerThree; 

/////SETUP_WIFI/////
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");  //get the unique MAC address to use as MQTT client ID, a 'truly' unique ID.
  Serial.println(WiFi.macAddress());  //macAddress returns a byte array 6 bytes representing the MAC address
}

 /////CONNECT/RECONNECT/////Monitor the connection to MQTT server, if down, reconnect
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect(mac, mqtt_user, mqtt_password)) { //<<---using MAC as client ID, always unique!!!
      Serial.println("connected");
      mqtt.subscribe("maria/bathroom"); //we are subscribing to 'fromJon/words' and all subtopics below that topic
    } else {                        //please change 'theTopic' to reflect your topic you are subscribing to
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  setup_wifi();

  while(!Serial);
  mqtt.setServer(mqtt_server, 1883);
  //mqtt.setCallback(callback);
  timerOne = timerTwo = timerThree = millis();

  pinMode(DOOR, INPUT_PULLUP);  //pullup will be HIGH when magnetic switch is OPEN, LOW when CLOSED 
  pinMode(PIR, INPUT);  //PIR Motion sensor setting as input
}

void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }

  mqtt.loop(); //this keeps the mqtt connection 'active'
  //Here we will deal with a JSON string
  if (millis() - timerTwo > 15000) {

  //reads the status of the magnetic door switch
    curr_door = digitalRead(DOOR);
    
    if(curr_door != prev_door) {
      prev_door = curr_door;

      if(curr_door == LOW) {
        Serial.println("Bathroom Door is closed");
      }
      else {
        Serial.println("Bathroom Door is open");
      }
    }

    //checks if motion is detected 
    pirRead = digitalRead(PIR);

    
    if (pirRead == HIGH) {
      if (pirState == LOW) {
        Serial.println("Motion detected");
        pirState = HIGH;
      }
    }
    else {
      if (pirState == HIGH) {
        Serial.println("Motion ended");
        pirState = LOW;
      }
    }
    delay(1000);

    sprintf(message, "{\"Door\":\"%d\", \"Motion\":\"%d\"}",curr_door, pirState);
    mqtt.publish("maria/bathroom", message);
    timerTwo = millis();
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic); //'topic' refers to the incoming topic name, the 1st argument of the callback function
  Serial.println("] ");

  DynamicJsonBuffer  jsonBuffer; //DJB
  JsonObject& root = jsonBuffer.parseObject(payload); //parse it!

  if (!root.success()) { //
    Serial.println("parseObject() failed, are you sure this message is JSON formatted.");
    return;
  }

  else if (strcmp(topic, "maria/bathroom") == 0) {
    Serial.println("Some bathroom info has arrived . . .");
  }

  root.printTo(Serial); //print out the parsed message
  Serial.println(); //give us some space on the serial monitor read out

}

