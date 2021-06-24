#ifndef NIMBLEESP_H
#define NIMBLEESP_H

#include "Arduino.h"

class NimBleEsp {

    public : 
        NimBleEsp();
        void initBle();
        bool connectToServer();
        bool getDoConnect();
        void writeCardiacValue(String value);
        bool getSendData();
        bool getConnected();
};

#endif