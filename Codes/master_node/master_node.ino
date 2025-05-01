#include <string.h>
#include <Wire.h>
// Pin and sensor configuration
// #define LIGHT_SENSOR_LEFT A1
// #define LIGHT_SENSOR_RIGHT A2
// #define TEMPERATURE_SENSOR A0

#define EDGE_NODE_1 8
// #define EDGE_NODE_2 0x11

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Plant management system initialized");
}

void loop() {
  // float temperature = analogRead(TEMPERATURE_SENSOR);
  // Serial.print("temperature: ");
  // Serial.println(temperature);
  // int lightLeft = analogRead(LIGHT_SENSOR_LEFT);
  // int lightRight = analogRead(LIGHT_SENSOR_RIGHT);
  // Serial.print("sensor left: ");
  // Serial.println(lightLeft);
  // Serial.print("sensor right: ");
  // Serial.println(lightRight);
  // int optimalPosition = (lightLeft > lightRight) ? 0 : 60;
  
  // manageEdgeNode(EDGE_NODE_1, temperature, optimalPosition);
  manageEdgeNode(EDGE_NODE_1, 3, 3);
  // manageEdgeNode(EDGE_NODE_2, temperature, optimalPosition);
  
  delay(1000);
}

void manageEdgeNode(int nodeAddress, float temperature, int rotationPosition) {
  // char moisture_2 = 0;

  // Wire.beginTransmission(nodeAddress);
  // Serial.println("go to write W on bus");
  // Wire.write('W');
  // Wire.endTransmission(nodeAddress);
  // return;

  // Request moisture
  Wire.requestFrom(nodeAddress, 1);
  Serial.print("Request sent to ");
  Serial.println(nodeAddress);
  if (Wire.available()){
    byte moisture_1 = Wire.read();
    byte moisture_2 = Wire.read();
    Serial.print("readed data 1 : ");
    Serial.println(moisture_1);
    Serial.print("readed data 2 : ");
    Serial.println(moisture_2);
  }
  else{
    Serial.println("Wire is not available");

  }

  // if(Wire.available()){
  //   byte moisture_1 = Wire.read();
  //   Serial.print("moisture 1 : ");
  //   Serial.println(moisture_1);
  // }
  // else{
  //   Serial.println("No data received");
  // }
  // moisture_2 = Wire.read();
  // Serial.print("Pot ");
  // Serial.print(nodeAddress == 0x10 ? 1 : 2);
  // Serial.print(" - Moisture: ");
  // Serial.print(moisture_1);
  // Serial.print((int)moisture_1);
  // Serial.println(moisture_2);
  // Serial.print("%, Temperature: ");
  // Serial.print(temperature);
  // Serial.println("°C");
  // char most[3];
  // most[0] = moisture_1;
  // most[1] = moisture_2;
  // most[2] = '\0';

  // int moisture = atoi(most);
  
  // int irrigationRate = calculateIrrigationRate(moisture, temperature);
  
  // Send irrigation command
  // Wire.beginTransmission(nodeAddress);
  // Wire.write('W');
  // Wire.write(sprintf("%d", irrigationRate));
  // Wire.endTransmission();
  
  // Send rotation command
  // Wire.beginTransmission(nodeAddress);
  // Wire.write('R');
  // Wire.write(rotationPosition);
  // Wire.endTransmission();
  
  // Serial.print("Sent command to Pot ");
  // Serial.print(nodeAddress == 0x10 ? 1 : 2);
  // Serial.print(": Irrigation ");
  // Serial.print(irrigationRate);
  // Serial.print(" drops/minute, Rotation to ");
  // Serial.print(rotationPosition);
  // Serial.println(" degrees");
}

// int calculateIrrigationRate(int moisture, float temperature) {
//   if(moisture > 80) {
//     return 0;
//   } else if(moisture < 50) {
//     return 15;
//   } else {
//     if(temperature > 25) {
//       return 10;
//     } else {
//       return 5;
//     }
//   }
// }
