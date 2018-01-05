#include <ESP8266WiFi.h>

#include <WiFiClient.h>

#include <ESP8266WebServer.h>

const char *ssid = "test";

// External Variables
String minMoistureLevel= "500";
String waterTime= "5000";
String plantName= "initial setup";

// Internal Variables
int currMoistureLevel= 0;
int currWaterLevel= 1;
bool lowWater= false;
bool doWater= false;

// Pin Assignment
// Arduino -> NodeMCU
int sensorSwitch= 5; // Grey h/s dupont jumper
int pumpSwitch= 4;   // White/White s/s h/h dupont jumper

// Water Sensor
int wSensorInput= 12; // Yellow s/s dupont jumper

// Moisture Sensor
int mSensorPower= 14; // Purple s/s dupont jumper
// no declaration for input, as there is only one analog input.

ESP8266WebServer server(80);

void handleRoot() {
  Serial.println("Handling client on /");
String temp= "{\"plantName\": \"" + plantName + "\"}";
server.send(200, "application/json", temp);

}
// server calls
void handleData(){
  Serial.println("Handling client on /data");
  if(server.hasArg("minMoistureLevel")){
    minMoistureLevel = server.arg("minMoistureLevel");
    Serial.println("new moistureLevel: " + minMoistureLevel);
  } else {
    Serial.println("did not find an arguement with Key: moistureLevel"); 
  };
  if(server.hasArg("waterTime")){
    waterTime = server.arg("waterTime");
    Serial.println("new waterTime: " + waterTime);
  } else {
    Serial.println("did not find an arguement with Key: waterAt"); 
  };
  if(server.hasArg("plantName")){
    plantName= server.arg("plantName");
    Serial.println("new plantName: " + plantName);
  } else {
    Serial.println("did not find an arguement with Key: plantName");
  };
  
  //server.send(200, "text/html", "<h1>"+minMoistureLevel+"</h1><h2>"+waterTime+"</h2><h2>"+plantName+"</h2>");
  server.send(200, "application/json", "{\"moisture\": \""+minMoistureLevel+"\", \"water\": \""+waterTime+"\", \"plantName\": \""+plantName+"\"}");
}

void sendWarning(){
  server.send(500, "text/html", "<h1>The water level is below minimum, please refill the water tank!!!</h1>");
}

// setup and main loop
void setup() {

delay(1000);

Serial.begin(9600);

Serial.println();

Serial.print("Configuring access point...");
WiFi.softAPConfig(IPAddress(192,168,4,1),IPAddress(192,168,4,1),IPAddress(255,255,255,0));
WiFi.softAP(ssid);

IPAddress myIP = WiFi.softAPIP();

Serial.print("AP IP address: ");

Serial.println(myIP);

server.on("/", handleRoot);

server.on("/data",handleData);

server.begin();

printMsg("HTTP server started");

// Pin Setup
pinMode(sensorSwitch, OUTPUT);
pinMode(pumpSwitch, OUTPUT);

pinMode(wSensorInput, INPUT);

pinMode(mSensorPower, OUTPUT);
}

void loop() {

server.handleClient();

//Read Moisutre and determine wheter the moisture level is acceptable or not
  readMoisture();
  shouldWater();
  if(doWater){ // If the moisture level is low enough, check for waterLevel
    readWaterLevel();
    if(!lowWater){
      waterPlant();
    } else if(lowWater){
      lowWaterWarn();
    };
  };
}

// utility methods
void readMoisture(){
  digitalWrite(mSensorPower, HIGH); // power on 
  delay(2000); // delay 2second to let sensor warm up
  analogRead(0); // dummy read to discard potential fluke at initial reading
  currMoistureLevel= analogRead(0); // read and set moisture level
  delay(500);
  digitalWrite(mSensorPower, LOW); // power off 
  String temp= "CurrentMoistureLevel: " + String(currMoistureLevel);
  printMsg(temp);
}

void readWaterLevel(){
  digitalWrite(sensorSwitch, HIGH); // Tell arduino to turn water sensor on
  delay(500); // delay to make sure water sensor has time to turn on
  digitalRead(wSensorInput); // dummy read to discard potential fluke at initial reading
  currWaterLevel= digitalRead(wSensorInput);
  delay(500);
  digitalWrite(sensorSwitch, LOW); // Tell arduino to turn water sensor off
  if(currWaterLevel == 1){
    lowWater= false;
  } else {
    lowWater= true;
  }
  Serial.println(currWaterLevel);
}

// serves no actual purpose, think about removing this in future
void lowWaterWarn(){
  if(lowWater){
    sendWarning();
  };
}

void shouldWater(){
  if(currMoistureLevel <= minMoistureLevel.toInt()){ // minMoistureLevel aquired from external program!
    doWater= true;
  } else {
    doWater= false;
  };
}

void waterPlant(){
  Serial.println("Starting Water Function");
  digitalWrite(pumpSwitch, HIGH); // Tell arduino to turn water pump on
  delay(2000); // give pump time to start up
  delay(waterTime.toInt()); // Water for x amount of time recieved from external source
  digitalWrite(pumpSwitch, LOW); // Tell arduino to turn water pump off
  Serial.println("Stopping Water Function");
}

void printMsg(String msg){
  Serial.println(msg);
  Serial.println("--------------------------");
}
