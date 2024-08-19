/*
  Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
  Ported to Arduino ESP32 by Evandro Copercini
  updated by chegewara and MoThunderz
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEValue.h>
#include <BLESecurity.h>

//Pins on XIAO
#define audio D5
#define reset D1
#define pin1 D2
#define pin2 D3
#define pin3 D4
#define eye0 "big_blue"
#define eye1 "doom_spiral"
#define eye2 "hypno_red"
#define eye3 "toonstripe"

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
BLEDescriptor *pDescr;
BLE2902 *pBLE2902;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"



class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("connected");
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("disconnected");
  }
};

class CharacteristicCallBack : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) override {
    std::string pChar_value_stdstr = pChar->getValue();
    String pChar_value_string = String(pChar_value_stdstr.c_str());
    int pChar_value_int = pChar_value_string.toInt();
    Serial.println("pChar: " + String(pChar_value_int));
    value = pChar_value_int;
  }
};

void setup() {
   Serial.begin(115200);
  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
  Serial.println("ESPNow/Basic/Master Example");
  // This is the mac address of the Master in Station Mode
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  Serial.print("STA CHANNEL "); Serial.println(WiFi.channel());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  //set pin modes
  pinMode(audio, OUTPUT);
  pinMode(reset, OUTPUT);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  pinMode(D9, OUTPUT);
  pinMode(D10, OUTPUT);
  pinMode(D0, OUTPUT);
  digitalWrite(reset, HIGH);
  clearAll();


  // Create the BLE Device
  BLEDevice::init("Screwball");


  //Security
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setCapability(ESP_IO_CAP_NONE);
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  pSecurity->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  pSecurity->setKeySize(16);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  //Security
  pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE);

  // Create a BLE Descriptor

  pDescr = new BLEDescriptor((uint16_t)0x2901);
  pDescr->setValue("selection number");
  pCharacteristic->addDescriptor(pDescr);

  pBLE2902 = new BLE2902();

  pCharacteristic->addDescriptor(pBLE2902);

  // After defining the desriptors, set the callback functions
  pCharacteristic->setCallbacks(new CharacteristicCallBack());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter

  //pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  //pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();
  delay(10);

  Serial.println("Waiting a client connection to notify...");
}
int prevState = 69;
void loop() {
  if (prevState != value) {
    //just chill, sometimes its ok to do nothing
 // } else {
    // notify changed value
    if (deviceConnected) {
      pCharacteristic->setValue(value);
      // Serial.println(value);
      pCharacteristic->getData();
      
      setState(value);
      prevState = value;
      delay(1000);
    }
  }


  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {

    oldDeviceConnected = deviceConnected;
  }
}

void setState(int state) {
  state--;
  String selection = "";
  switch (state) {

    case 0:
      clearState();
      selection = eye0;
      digitalWrite(reset, LOW);
      Serial.println("RESET");
      break;
    case 1:
      clearState();
      selection = eye1;
      digitalWrite(pin1, HIGH);
      digitalWrite(reset, LOW);
      Serial.println("RESET");
      break;
    case 2:
      clearState();
      selection = eye2;
      digitalWrite(pin2, HIGH);
      digitalWrite(reset, LOW);
      Serial.println("RESET");
      break;
    case 3:
      clearState();
      selection = eye3;
      digitalWrite(pin3, HIGH);
      digitalWrite(reset, LOW);
      Serial.println("RESET");
      break;
    case 4:
      selection = "sound fx";
      digitalWrite(audio, HIGH);
      delay(30);
      digitalWrite(audio, LOW);
    default:
      break;
  }
  Serial.println(selection);
  digitalWrite(reset, HIGH);
}

//clears the pin states back to default
void clearState() {
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);
}

void clearAll() {
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);
  digitalWrite(audio, LOW);
  digitalWrite(D6, LOW);
  digitalWrite(D8, LOW);
  digitalWrite(D0, LOW);
  digitalWrite(D7, LOW);
  digitalWrite(D9, LOW);
  digitalWrite(D10, LOW);
}

//espNow stuff (master)

