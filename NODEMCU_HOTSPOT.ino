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

// Pin Assigments
// Water Sensor
int wPower= 14; // currently not used as digital pins doesn't provide enough power
int wInput= 2;

// Moisture Sensor
int mPower= 4;
// ADC Input always 0

// Water Pump
int pPower= 0;

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
pinMode(mPower, OUTPUT); // set Moisture Power Pin to OUTPUT
digitalWrite(mPower, LOW); // Make sure Moisture Power Pin is turned off

pinMode(wPower, OUTPUT); // set WaterSensor Power Pin to OUTPUT
digitalWrite(wPower, LOW); // Make sure WaterSensor Power Pin is turned off
pinMode(wInput, INPUT); // Set WaterSensor Input Pin to INPUT

pinMode(pPower, OUTPUT); // set Water Pump Pin to OUTPUT
digitalWrite(pPower, LOW); // Make sure Water Pump Pin is turned off
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
  digitalWrite(mPower, HIGH); // power on 
  delay(2000); // delay 2second to let sensor warm up
  analogRead(0); // dummy read to discard potential fluke at initial reading
  currMoistureLevel= analogRead(0); // read and set moisture level
  delay(500);
  digitalWrite(mPower, LOW); // power off 
  String temp= "CurrentMoistureLevel: " + String(currMoistureLevel);
  printMsg(temp);
}

void readWaterLevel(){
  analogWrite(wPower, 1023);
  //digitalWrite(wPower, HIGH); // power on
  delay(1000); // delay 1second to let sensor warm up
  currWaterLevel= digitalRead(wInput); // read and set water level
  Serial.println(currWaterLevel);
  if(currWaterLevel == 0){
    lowWater= true; // if water level is 0, set lowWater to true
  } else {
    lowWater= false; // else set to false
  };
  delay(500);
  analogWrite(wPower, 0);
  //digitalWrite(wPower, LOW); // power off
  if(lowWater){
    printMsg("WaterLevel is LOW");
  } else {
    printMsg("WaterLevel is FINE");
  };
  
}

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
  
  if(doWater){
    digitalWrite(pPower, HIGH); // power on
    delay(waterTime.toInt()); // water time in milliseconds, aquired from external program! OBS! Pumps 0,0276 L per 1 Second
    digitalWrite(pPower, LOW); // power off
  }; 
}

void printMsg(String msg){
  Serial.println(msg);
  Serial.println("--------------------------");
}
