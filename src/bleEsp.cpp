#include "bleEsp.h"
#include "BLEDevice.h"

//uuid du serveur
static BLEUUID serviceUUID("19B10000-E8F2-537E-4F6C-D104768A1214");

//uuid characteristic temperature du serveur
static BLEUUID tempCharUUID("19B10001-E8F2-537E-4F6C-D104768A1214");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteTempCharacteristic;
static BLEAdvertisedDevice* myDevice;

//ajout de callback pour le ble Client
class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient* pclient)
  {
    Serial.println("connected");
  }

  void onDisconnect(BLEClient* pclient)
  {
    connected = false;
    Serial.println("onDisconnect");
  }
};

//ajout d'un callback pour l'advertise
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
  //appeller à chaque résultat d'un scan
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.getName().c_str());


    /* Check si le device scanné correspond à ce que l'on cherche */
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
    {
        Serial.println("!!!!!!characteristic and service foud!!!!!!!!!!");
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    }
  }
};



BleEsp::BleEsp() {

}

void BleEsp::initBle() {
     BLEDevice::init("ESP32-BLE-Client");
    BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  //interval entre chaque scan
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

bool BleEsp::connectToServer() {

  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

    //connection au server
  pClient->connect(myDevice); 
  Serial.println(" - Connected to server");

   //recupération du service
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");


  //récupération characteristic temperature
  pRemoteTempCharacteristic = pRemoteService->getCharacteristic(tempCharUUID);
  if (pRemoteTempCharacteristic == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(tempCharUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

    connected = true;
    doConnect = false;
    return true;
}

void BleEsp::writeTempValue(String value) {
     pRemoteTempCharacteristic->writeValue(value.c_str(), value.length());
}

bool BleEsp::getConnected() {
    return connected;
}

bool BleEsp::getDoConnect() {
    return doConnect;
}