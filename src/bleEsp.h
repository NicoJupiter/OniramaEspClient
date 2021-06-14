#ifndef BLEESP_H
#define BLEESP_H

#include "Arduino.h"

class BleEsp {

    public : 
        BleEsp();
        void initBle();
        bool connectToServer();
        void writeTempValue(String value);
        bool getConnected();
        bool getDoConnect();
};

#endif