#include <Arduino.h>
#include "bleEsp.h"
#include "DFRobot_Heartrate.h"
    
DFRobot_Heartrate heartrate(DIGITAL_MODE); ///< ANALOG_MODE or DIGITAL_MODE

BleEsp bleEsp;

#define heartPin 13
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
     
     int randNumber = random(80, 90);
      bleEsp.writeSensorValue(String(randNumber));
     /* uint8_t rateValue;
 
      heartrate.getValue(heartPin);
      rateValue = heartrate.getRate();
      if(rateValue)  {
        Serial.println("----Hearth rate-----");
        Serial.println(rateValue);
        bleEsp.writeSensorValue(String(rateValue));
      }*/
   }
  }
  if(bleEsp.getDoScan()) {
    //ESP.restart();
    bleEsp.startScan();
  }
  delay(20);
}
