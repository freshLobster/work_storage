//MASTER


//BLE
/*
  Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
  Ported to Arduino ESP32 by Evandro Copercini
  updated by chegewara and MoThunderz
*/
//EspNow
/**
   ESPNOW - Basic communication - Master
   Date: 26th September 2017
   Author: Arvind Ravulavaru <https://github.com/arvindr21>
   Purpose: ESPNow Communication between a Master ESP32 and a Slave ESP32
   Description: This sketch consists of the code for the Master module.
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/

   << This Device Master >>

   Flow: Master
   Step 1 : ESPNow Init on Master and set it in STA mode
   Step 2 : Start scanning for Slave ESP32 (we have added a prefix of `slave` to the SSID of slave for an easy setup)
   Step 3 : Once found, add Slave as peer
   Step 4 : Register for send callback
   Step 5 : Start Transmitting data from Master to Slave

   Flow: Slave
   Step 1 : ESPNow Init on Slave
   Step 2 : Update the SSID of Slave with a prefix of `slave`
   Step 3 : Set Slave in AP mode
   Step 4 : Register for receive callback and wait for data
   Step 5 : Once data arrives, print it in the serial monitor

   Note: Master and Slave have been defined to easily understand the setup.
         Based on the ESPNOW API, there is no concept of Master and Slave.
         Any devices can act as master or salve.
*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEValue.h>
#include <BLESecurity.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h> // only for esp_wifi_set_channel()

// Global copy of slave
esp_now_peer_info_t slave;
#define CHANNEL 2
#define PRINTSCANRESULTS 0
#define DELETEBEFOREPAIR 0

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
    Serial.println(F("connected"));
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println(F("disconnected"));
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
  Serial.println(F("ESPNow/Basic/Master Example"));
  // This is the mac address of the Master in Station Mode
  Serial.print(F("STA MAC: ")); Serial.println(WiFi.macAddress());
  Serial.print(F("STA CHANNEL ")); Serial.println(WiFi.channel());
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
  /*pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  pinMode(D9, OUTPUT);
  pinMode(D10, OUTPUT);
  pinMode(D0, OUTPUT);
  */
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

  Serial.println(F("Waiting a client connection to notify..."));
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
      //pCharacteristic->getData();
      
      setState(value);
      
      delay(1000);
    }
    prevState = value;

     // In the loop we scan for slave
    ScanForSlave();
    if(manageSlave()){
      // If Slave is found, it would be populate in `slave` variable
      // We will check if `slave` is defined and then we proceed further
      if (slave.channel == CHANNEL) { // check if slave channel is defined
       // `slave` is defined

        sendData(value);

      } else {
        // slave pair failed
        Serial.println(F("Slave pair failed!"));
     }
    }
  }


  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println(F("start advertising"));
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {

    oldDeviceConnected = deviceConnected;
  }
  
}

void setState(int state) {
  state--;
  //String selection = "";
  switch (state) {

    case 0:
      clearState();
      Serial.println(eye0);
      digitalWrite(reset, LOW);
      Serial.println(F("RESET"));
      break;
    case 1:
      clearState();
      Serial.println(eye1);
      digitalWrite(pin1, HIGH);
      digitalWrite(reset, LOW);
      Serial.println(F("RESET"));
      break;
    case 2:
      clearState();
      Serial.println(eye2);
      digitalWrite(pin2, HIGH);
      digitalWrite(reset, LOW);
      Serial.println(F("RESET"));
      break;
    case 3:
      clearState();
      Serial.println(eye3);
      digitalWrite(pin3, HIGH);
      digitalWrite(reset, LOW);
      Serial.println(F("RESET"));
      break;
    case 4:
      Serial.println(F("sound fx"));
      digitalWrite(audio, HIGH);
      delay(30);
      digitalWrite(audio, LOW);
    default:
      break;
  }
  //Serial.println(selection);
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
void sendData(uint8_t data) {
  const uint8_t *peer_addr = slave.peer_addr;
  Serial.print(F("Sending: ")); Serial.println(data);
  esp_err_t result = esp_now_send(peer_addr, &data, sizeof(data));
  Serial.print(F("Send Status: "));
  if (result == ESP_OK) {
    Serial.println(("Success"));
  } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println(F("ESPNOW not Init."));
  } else if (result == ESP_ERR_ESPNOW_ARG) {
    Serial.println(F("Invalid Argument"));
  } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
    Serial.println(F("Internal Error"));
  } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
    Serial.println(F("ESP_ERR_ESPNOW_NO_MEM"));
  } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println(F("Peer not found."));
  } else {
    Serial.println(F("Not sure what happened"));
  }
}

// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(F("Last Packet Sent to: ")); Serial.println(macStr);
  Serial.print(F("Last Packet Send Status: ")); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println(F("ESPNow Init Success"));
  }
  else {
    Serial.println(F("ESPNow Init Failed"));
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// Scan for slaves in AP mode
void ScanForSlave() {
  int16_t scanResults = WiFi.scanNetworks(false, false, false, 300, CHANNEL); // Scan only on one channel
  // reset on each scan
  bool slaveFound = 0;
  memset(&slave, 0, sizeof(slave));

  Serial.println("");
  if (scanResults == 0) {
    Serial.println(F("No WiFi devices in AP Mode found"));
  } else {
    Serial.print(F("Found ")); Serial.print(scanResults); Serial.println(F(" devices "));
    for (int i = 0; i < scanResults; ++i) {
      // Print SSID and RSSI for each device found
      String SSID = WiFi.SSID(i);
      int32_t RSSI = WiFi.RSSI(i);
      String BSSIDstr = WiFi.BSSIDstr(i);

      if (PRINTSCANRESULTS) {
        Serial.print(i + 1);
        Serial.print(F(": "));
        Serial.print(SSID);
        Serial.print(F(" ("));
        Serial.print(RSSI);
        Serial.print(F(")"));
        Serial.println(F(""));
      }
      delay(10);
      // Check if the current device starts with `Slave`
      if (SSID.indexOf("Slave") == 0) {
        // SSID of interest
        Serial.println(F("Found a Slave."));
        Serial.print(i + 1); Serial.print(F(": ")); Serial.print(SSID); Serial.print(F(" [")); Serial.print(BSSIDstr); Serial.print(F("]")); Serial.print(F(" (")); Serial.print(RSSI); Serial.print(F(")")); Serial.println(F(""));
        // Get BSSID => Mac Address of the Slave
        int mac[6];
        if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
          for (int ii = 0; ii < 6; ++ii ) {
            slave.peer_addr[ii] = (uint8_t) mac[ii];
          }
        }

        slave.channel = CHANNEL; // pick a channel
        slave.encrypt = 0; // no encryption

        slaveFound = 1;
        // we are planning to have only one slave in this example;
        // Hence, break after we find one, to be a bit efficient
        break;
      }
    }
  }

  if (slaveFound) {
    Serial.println(F("Slave Found, processing.."));
  } else {
    Serial.println(F("Slave Not Found, trying again."));
  }

  // clean up ram
  WiFi.scanDelete();
}

// Check if the slave is already paired with the master.
// If not, pair the slave with master
bool manageSlave() {
  if (slave.channel == CHANNEL) {
    if (DELETEBEFOREPAIR) {
      deletePeer();
    }

    Serial.print(F("Slave Status: "));
    // check if the peer exists
    bool exists = esp_now_is_peer_exist(slave.peer_addr);
    if ( exists) {
      // Slave already paired.
      Serial.println(F("Already Paired"));
      return true;
    } else {
      // Slave not paired, attempt pair
      esp_err_t addStatus = esp_now_add_peer(&slave);
      if (addStatus == ESP_OK) {
        // Pair success
        Serial.println(F("Pair success"));
        return true;
      } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println(F("ESPNOW Not Init"));
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
        Serial.println(F("Invalid Argument"));
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
        Serial.println(F("Peer list full"));
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println(F("Out of memory"));
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
        Serial.println(F("Peer Exists"));
        return true;
      } else {
        Serial.println(F("Not sure what happened"));
        return false;
      }
    }
  } else {
    // No slave found to process
    Serial.println(F("No Slave found to process"));
    return false;
  }
}

void deletePeer() {
  esp_err_t delStatus = esp_now_del_peer(slave.peer_addr);
  Serial.print(F("Slave Delete Status: "));
  if (delStatus == ESP_OK) {
    // Delete success
    Serial.println(F("Success"));
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println(F("ESPNOW Not Init"));
  } else if (delStatus == ESP_ERR_ESPNOW_ARG) {
    Serial.println(F("Invalid Argument"));
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println(F("Peer not found."));
  } else {
    Serial.println(F("Not sure what happened"));
  }
}
