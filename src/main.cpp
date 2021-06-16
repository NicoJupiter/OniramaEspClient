#include <Arduino.h>
#include "bleEsp.h"

BleEsp bleEsp;

const int tempPin = 4;     //analog input du capteur de temperature
int tempVal;  
float volts; 
float temp; 

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  bleEsp.initBle();
}

void loop()
{

  if (bleEsp.getDoConnect()) {
    if (bleEsp.connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("Failed to connect to server");
    }
  }

 if (bleEsp.getConnected()) {
   if(bleEsp.getSendData()) {
    tempVal = analogRead(tempPin);
    volts = tempVal/1023.0;             // normalize by the maximum temperature raw reading range
    temp = (volts - 0.5) * 100 ;         //calculate temperature celsius from voltage as per the equation found on the sensor spec sheet.
    String newValue = String(temp);
    Serial.println(newValue);
    bleEsp.writeTempValue(newValue);
   }
  }
  if(bleEsp.getDoScan()) {
    ESP.restart();
    //bleEsp.startScan();
  }
  delay(2000);
}
