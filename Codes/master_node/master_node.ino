#include <Wire.h>
#include <DHT.h>
#include <string.h>

// Pin and sensor configuration
#define DHT_PIN 2
#define DHT_TYPE DHT22
#define LIGHT_SENSOR_LEFT A1
#define LIGHT_SENSOR_RIGHT A2
#define TEMPERATURE_SENSOR A0

#define EDGE_NODE_1 0x10
#define EDGE_NODE_2 0x11

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Wire.begin(); // Master
  Serial.begin(9600);
  dht.begin();
  
  Serial.println("Plant management system initialized");
}

void loop() {
  float temperature = analogRead(TEMPERATURE_SENSOR);
  int lightLeft = analogRead(LIGHT_SENSOR_LEFT);
  int lightRight = analogRead(LIGHT_SENSOR_RIGHT);
  
  int optimalPosition = (lightLeft > lightRight) ? 0 : 60;
  
  manageEdgeNode(EDGE_NODE_1, temperature, optimalPosition);
  manageEdgeNode(EDGE_NODE_2, temperature, optimalPosition);
  
  delay(3000);
}

void manageEdgeNode(int nodeAddress, float temperature, int rotationPosition) {
  char moisture_1 = 0;
  char moisture_2 = 0;

  // Request moisture

  Wire.requestFrom(nodeAddress, 2);
  if(Wire.available()) {
    moisture_1 = Wire.read();
    moisture_2 = Wire.read();
    // Serial.print("Pot ");
    // Serial.print(nodeAddress == 0x10 ? 1 : 2);
    Serial.print(" - Moisture: ");
    Serial.print(moisture_1);
    Serial.println(moisture_2);
    Serial.print("%, Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");

  }
  char most[3];
  most[0] = moisture_1;
  most[1] = moisture_2;
  most[2] = '\0';

  int moisture = atoi(most);
  
  int irrigationRate = calculateIrrigationRate(moisture, temperature);
  
  // Send irrigation command
  Wire.beginTransmission(nodeAddress);
  Wire.write('W');
  Wire.write(sprintf("%d", irrigationRate));
  Wire.endTransmission();
  
  // Send rotation command
  Wire.beginTransmission(nodeAddress);
  Wire.write('R');
  Wire.write(rotationPosition);
  Wire.endTransmission();
  
  Serial.print("Sent command to Pot ");
  Serial.print(nodeAddress == 0x10 ? 1 : 2);
  Serial.print(": Irrigation ");
  Serial.print(irrigationRate);
  Serial.print(" drops/minute, Rotation to ");
  Serial.print(rotationPosition);
  Serial.println(" degrees");
}

int calculateIrrigationRate(int moisture, float temperature) {
  if(moisture > 80) {
    return 0;
  } else if(moisture < 50) {
    return 15;
  } else {
    if(temperature > 25) {
      return 10;
    } else {
      return 5;
    }
  }
}
