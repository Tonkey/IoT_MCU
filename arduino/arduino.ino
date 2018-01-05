// NodeMCU -> Arduino digital
int sensorSwitch= 7; // Grey h/s dupont jumper
int pumpSwitch= 6;   // White/White s/s h/h dupont jumper

// Water Sensor
int sensorPower= 5; // Orange h/s dupont jumper
boolean doSense= false;

// Water Pump
int pumpPower= 10; // Black h/h small
boolean doPump= false;

void setup() {
  Serial.begin(9600);
  // NodeMCU -> Arduino
  pinMode(sensorSwitch, INPUT);
  pinMode(pumpSwitch, INPUT);

  // Water Sensor / Water Pump
  pinMode(sensorPower, OUTPUT);
  pinMode(pumpPower, OUTPUT);
}

void loop() {
  if(digitalRead(sensorSwitch) == 1){
    shouldSense();
    waterSensor();
  } else {
    digitalWrite(sensorPower, LOW); // make sure sensor is turned off.
  };
  if(digitalRead(pumpSwitch) == 1){
    shouldPump();
    waterPump();
  } else {
    digitalWrite(pumpPower, LOW); // make sure pump is turned off.
  };

}


void waterSensor(){
  Serial.println("Water Sensor Starting");
  while(doSense){
    digitalWrite(sensorPower, HIGH);
    shouldSense();
  };
  digitalWrite(sensorPower, LOW);
}

void waterPump(){
  Serial.println("Water Pump Starting");
  while(doPump){
    digitalWrite(pumpPower, HIGH);
    shouldPump();
  };
  digitalWrite(pumpPower, LOW);
  Serial.println("Water Pump Done");
}

void shouldSense(){
  if(digitalRead(sensorSwitch) == 1){
    doSense= true;
  } else {
    doSense= false;
  };
}

void shouldPump(){
  if(digitalRead(pumpSwitch) == 1){
    doPump = true;
  } else {
    doPump = false;
  }
}

