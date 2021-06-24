#include "nimBleEsp.h"

#include <NimBLEDevice.h>

void scanEndedCB(NimBLEScanResults results);

static NimBLEAdvertisedDevice* advDevice;

static bool doConnect = false;
static uint32_t scanTime = 0; /** 0 = scan forever */
bool nimSendData = false;
bool nimConnected = false;

NimBLERemoteService* pNimRemoteService = nullptr;
NimBLERemoteCharacteristic* pNinRemoteSensorChr = nullptr;
NimBLERemoteCharacteristic* pNinRemoteNotifyChr = nullptr;
/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) {
        Serial.println("Connected");
        pClient->updateConnParams(120,120,0,60);
    };

    void onDisconnect(NimBLEClient* pClient) {
        Serial.print(pClient->getPeerAddress().toString().c_str());
        Serial.println(" Disconnected - Starting scan");
        NimBLEDevice::getScan()->start(scanTime, scanEndedCB);
    };

};

/** Create a single global instance of the callback class to be used by all clients */
static ClientCallbacks clientCB;

/** Define a class to handle the callbacks when advertisments are received */
class AdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {

    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        Serial.print("Advertised Device found: ");
        Serial.println(advertisedDevice->toString().c_str());
        if(advertisedDevice->isAdvertisingService(NimBLEUUID("19B10000-E8F2-537E-4F6C-D104768A1214")))
        {
            Serial.println("Found Our Service");
            /** stop scan before connecting */
            NimBLEDevice::getScan()->stop();
            /** Save the device reference in a global for the client to use*/
            advDevice = advertisedDevice;
            /** Ready to connect now */
            doConnect = true;
        }
    };
};

void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
    Serial.println("notify");
    nimSendData = true;
}



/** Callback to process the results of the last scan or restart it */
void scanEndedCB(NimBLEScanResults results){
    Serial.println("Scan Ended");
}

NimBleEsp::NimBleEsp() {

}

void NimBleEsp::initBle() {

    NimBLEDevice::init("ESP32");
    NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); 

    /** create new scan */
    NimBLEScan* pScan = NimBLEDevice::getScan();

    pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());

    /** Set scan interval (how often) and window (how long) in milliseconds */
    pScan->setInterval(45);
    pScan->setWindow(15);

    pScan->setActiveScan(true);

    pScan->start(scanTime, scanEndedCB);
}

/** Handles the provisioning of clients and connects / interfaces with the server */
bool NimBleEsp::connectToServer() {
    NimBLEClient* pClient = nullptr;

    /** Check if we have a client we should reuse first **/
    if(NimBLEDevice::getClientListSize()) {
        /** Special case when we already know this device, we send false as the
         *  second argument in connect() to prevent refreshing the service database.
         *  This saves considerable time and power.
         */
        pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if(pClient){
            if(!pClient->connect(advDevice, false)) {
                Serial.println("Reconnect failed");
                return false;
            }
            Serial.println("Reconnected client");
        }
        /** We don't already have a client that knows this device,
         *  we will check for a client that is disconnected that we can use.
         */
        else {
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }

    /** No client to reuse? Create a new one. */
    if(!pClient) {
        if(NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
            Serial.println("Max clients reached - no more connections available");
            return false;
        }

        pClient = NimBLEDevice::createClient();

        Serial.println("New client created");

        pClient->setClientCallbacks(&clientCB, false);
 
        pClient->setConnectionParams(12,12,0,51);
        pClient->setConnectTimeout(5);


        if (!pClient->connect(advDevice)) {
            /** Created a client but failed to connect, don't need to keep it as it has no data */
            NimBLEDevice::deleteClient(pClient);
            Serial.println("Failed to connect, deleted client");
            return false;
        }
    }

    if(!pClient->isConnected()) {
        if (!pClient->connect(advDevice)) {
            Serial.println("Failed to connect");
            return false;
        }
    }

    pNimRemoteService = pClient->getService("19B10000-E8F2-537E-4F6C-D104768A1214");
    if(pNimRemoteService) {     /** make sure it's not null */
        pNinRemoteSensorChr = pNimRemoteService->getCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214");
        pNinRemoteNotifyChr = pNimRemoteService->getCharacteristic("19B10003-E8F2-537E-4F6C-D104768A1214");
        if(pNinRemoteSensorChr->canWrite()) {
                if(pNinRemoteSensorChr->writeValue("No tip!")) {
                    Serial.print("Wrote new value to: ");
                    Serial.println(pNinRemoteSensorChr->getUUID().toString().c_str());
                }
                else {
                    /** Disconnect if write failed */
                    pClient->disconnect();
                    return false;
                }

                if(pNinRemoteSensorChr->canRead()) {
                    Serial.print("The value of: ");
                    Serial.print(pNinRemoteSensorChr->getUUID().toString().c_str());
                    Serial.print(" is now: ");
                    Serial.println(pNinRemoteSensorChr->readValue().c_str());
                }
            }

        if(pNinRemoteNotifyChr->canNotify()) {
            //if(!pChr->registerForNotify(notifyCB)) {
            if(!pNinRemoteNotifyChr->subscribe(true, notifyCB)) {
                /** Disconnect if subscribe failed */
                pClient->disconnect();
                return false;
            }
        }


    } else {
        Serial.println("service not found.");
    }
    Serial.println("Done with this device!");
    doConnect = false;
    nimConnected = true;
    return true;
}

void NimBleEsp::writeCardiacValue(String value) {
    
    if(pNinRemoteSensorChr->canWrite()) {
        Serial.println("writing");
        pNinRemoteSensorChr->writeValue("40");
        nimSendData = false;
    } else {
        Serial.println("Cant write");
    }
}

bool NimBleEsp::getDoConnect() {
    return doConnect;
}

bool NimBleEsp::getSendData() {
    return nimSendData;
}

bool NimBleEsp::getConnected() {
    return nimConnected;
}