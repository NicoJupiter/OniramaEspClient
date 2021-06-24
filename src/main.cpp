#include <Arduino.h>
#include "bleEsp.h"
#include "nimBleEsp.h"
#include "DFRobot_Heartrate.h"
    
DFRobot_Heartrate heartrate(DIGITAL_MODE); ///< ANALOG_MODE or DIGITAL_MODE

//BleEsp bleEsp;
NimBleEsp nimBleEsp;

#define heartPin 13
void setup()
{
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  //bleEsp.initBle();
  nimBleEsp.initBle();
}

void loop()
{

  if(nimBleEsp.getDoConnect()) {
    if(nimBleEsp.connectToServer())  {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("Failed to connect to server");
    }
   // NimBLEDevice::getScan()->start(scanTime,scanEndedCB);
  }

if(nimBleEsp.getConnected()) {
  if(nimBleEsp.getSendData()) {
      int randNumber = random(80, 90);
      nimBleEsp.writeCardiacValue(String(randNumber));
      /*uint8_t rateValue;
  
      heartrate.getValue(heartPin);
      rateValue = heartrate.getRate();
      if(rateValue)  {
        Serial.println("----Hearth rate-----");
        Serial.println(rateValue);
        nimBleEsp.writeCardiacValue(String(rateValue));
      }*/
    }
  }
  

  delay(2000);
  
}
