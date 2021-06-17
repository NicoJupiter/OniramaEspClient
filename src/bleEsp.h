#ifndef BLEESP_H
#define BLEESP_H

#include "Arduino.h"

class BleEsp {

    public : 
        BleEsp();
        void initBle();
        void startScan();
        bool connectToServer();
        void writeSensorValue(String value);
        bool getConnected();
        bool getDoConnect();
        bool getDoScan();
        bool getSendData();
};

#endif