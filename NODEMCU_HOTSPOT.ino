#include <ESP8266WiFi.h>

#include <WiFiClient.h>

#include <ESP8266WebServer.h>

const char *ssid = "test";

const char *password = "password";

// External Variables
int minMoistureLevel= 0;
int waterTime= 0;
String plantName= "";

// Internal Variables
int currMoistureLevel= 0;
int currWaterLevel= 1;
bool lowWater= false;
bool doWater= false;

// Pin Assigments
// Water Sensor
int wPower= GPIO5;
int wInput= GPIO16;

// Moisture Sensor
int mPower= GPIO4;
// ADC Input always 0

// Water Pump
int pPower= GPIO0;

ESP8266WebServer server(80);

void handleRoot() {

server.send(200, "text/html", "<h1>You are connected</h1>");

}
// server calls
void handleData(){
  if(server.hasArg("minMoistureLevel")){
    minMoistureLevel = server.arg("minMoistureLevel");
  } else {
    Serial.println("did not find an arguement with Key: moistureLevel"); 
  };
  if(server.hasArg("waterTime")){
    waterTime = server.arg("waterTime");
  } else {
    Serial.println("did not find an arguement with Key: waterAt"); 
  };
  if(server.hasArg("plantName")){
    plantName= server.arg("plantName");
  } else {
    Serial.println("did not find an arguement with Key: plantName");
  };
  server.send(200, "text/html", "<h1>"+moistureLevel+"</h1><h2>"+waterTime+"</h2>");
}

void sendWarning(){
  server.send(500, "text/html", "<h1>The water level is below minimum, please refill the water tank!!!</h1>")
}

// setup and main loop
void setup() {

delay(1000);

Serial.begin(115200);

Serial.println();

Serial.print("Configuring access point...");

WiFi.softAP(ssid, password);

IPAddress myIP = WiFi.softAPIP();

Serial.print("AP IP address: ");

Serial.println(myIP);

server.on("/", handleRoot);

server.on("/data",handleData);

server.begin();

printMsg("HTTP server started");

// pin setup
gpio.mode(mPower, OUTPUT); // set Moisture Power Pin to OUTPUT
gpio.write(mPower, LOW); // Make sure Moisture Power Pin is turned off

gpio.mode(wPower, OUTPUT); // set WaterSensor Power Pin to OUTPUT
gpio.write(wPower, LOW); // Make sure WaterSensor Power Pin is turned off
gpio.mode(wInput, INPUT); // Set WaterSensor Input Pin to INPUT

gpio.mode(pPower, OUTPUT); // set Water Pump Pin to OUTPUT
gpio.write(pPower, LOW); // Make sure Water Pump Pin is turned off
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
  gpio.write(mPower, HIGH); // power on 
  delay(2000); // delay 2second to let sensor warm up
  adc.read(0); // dummy read to discard potential fluke at initial reading
  currMoistureLevel= adc.read(0); //read and set moisture level
  delay(500);
  gpio.write(mPower, LOW); // power off 
}

void readWaterLevel(){
  gpio.write(wPower, HIGH); // power on
  delay(1000); // delay 1second to let sensor warm up
  currWaterLevel= gpio.read(wInput); // read and set water level
  if(currWaterLevel == 0){
    lowWater= true; // if water level is 0, set lowWater to true
  } else {
    lowWater= false; // else set to false
  };
  delay(500);
  gpio.write(wPower, LOW); // power off
}

void lowWaterWarn(){
  if(lowWater){
    sendWarning();
  };
}

void shouldWater(){
  if(currMoistureLevel <= minMoistureLevel){ // minMoistureLevel aquired from external program!
    doWater= true;
  } else {
    doWater= false;
  };
}

void waterPlant(){
  if(doWater){
    gpio.write(pPower, HIGH); // power on
    delay(waterTime); // water time in milliseconds, aquired from external program! OBS! Pumps 0,0276 L per 1 Second
    gpio.write(pPower, LOW); // power off
  }; 
}

void printMsg(String msg){
  Serial.println(msg);
  Serial.println("--------------------------");
}


