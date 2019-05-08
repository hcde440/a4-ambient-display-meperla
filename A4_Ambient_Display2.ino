//Feather 1 
//Ambient Display Code

#include <ESP8266WiFi.h>
#include "Wire.h"          
#include <PubSubClient.h>  
#include <ArduinoJson.h>  
#include <Servo.h>

#define wifi_ssid "Half-G Guest"               // Wifi network SSID
#define wifi_password "BeOurGuest"      // Wifi network password

#define mqtt_server "mediatedspaces.net"  //this is its address, unique to the server
#define mqtt_user "hcdeiot"               //this is its server login, unique to the server
#define mqtt_password "esp8266"           //this is it server password, unique to the server
#define topic_name "maria/bathroom"        //topic you are subscribing to

WiFiClient espClient;             //espClient
PubSubClient mqtt(espClient);     //tie PubSub (mqtt) client to WiFi client

Servo myservo;
int pos = 0;
int ledPin = 2;


char mac[6];
char message[201];

double motionStatus;
double doorStatus;
char incoming[100]; //an array to hold the incoming message

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
  Serial.println(WiFi.macAddress());  //.macAddress returns a byte array 6 bytes representing the MAC address
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

  double doorStatus=root["Door"];
  double motionStatus=root["Motion"];

  Serial.print("Bathroom door status: ");
  Serial.println(doorStatus);
  Serial.print("Motion status: ");
  Serial.println(motionStatus);

  //activates servo motor when bathroom door is closed or open
  if (doorStatus == 1) {  //closes door when bathroom door is in use
      Serial.println("door close");
      myservo.write(0);
      delay(15); 
  } else {  //opens door when bathroom door is open
      Serial.println("door open");
      myservo.write(100);
      delay(15);
  }
  
  //activates LED light when motion is detected
  if(motionStatus == 1){
    digitalWrite (ledPin, HIGH);
    Serial.println("LED is on");
  } else {
    Serial.println("Led is off");
    digitalWrite (ledPin, LOW);
  }
}

void setup() {
  Serial.begin(115200);

  setup_wifi();

  while(!Serial);
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback);

  myservo.attach(13);
  pinMode (ledPin, OUTPUT);
}

void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop(); //this keeps the mqtt connection 'active' 
}
