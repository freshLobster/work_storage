

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
#include "esp_sleep.h"
#include "esp_pm.h"

//#define SLAVE
#define MASTER

//#define FUN_DEBUG

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
uint32_t value = 0;


void setState(int state) {
  state--;
  //String selection = "";
  switch (state) {

    case 0:
      clearState();
      Serial.println(eye0);
      digitalWrite(reset, LOW);
      Serial.println("RESET");
      break;
    case 1:
      clearState();
      Serial.println(eye1);
      digitalWrite(pin1, HIGH);
      digitalWrite(reset, LOW);
      Serial.println("RESET");
      break;
    case 2:
      clearState();
      Serial.println(eye2);
      digitalWrite(pin2, HIGH);
      digitalWrite(reset, LOW);
      Serial.println("RESET");
      break;
    case 3:
      clearState();
      Serial.println(eye3);
      digitalWrite(pin3, HIGH);
      digitalWrite(reset, LOW);
      Serial.println("RESET");
      break;
    case 4:
      Serial.println("sound fx");
      digitalWrite(audio, HIGH);
      delay(30);
      digitalWrite(audio, LOW);
    default:
      Serial.print("something happened?");
      Serial.println(state);
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



#ifdef MASTER

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "6d030d7d-1cb1-4f65-a1f0-f6db1560bd1f"  // Use Heart Rate Service UUID for testing
#define CHARACTERISTIC_UUID "0192df61-0577-7e14-b84d-1698bd9d419f"  // Test UUID

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("connected");
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("disconnected");
    BLEDevice::startAdvertising();
  }
};

class CharacteristicCallBack : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) override {
    String pChar_value_string = String(pChar->getValue().c_str());
    int pChar_value_int = pChar_value_string.toInt();
    Serial.printf("pChar:\t%d \n", pChar_value_int);
    value = pChar_value_int;
  }
};

void initBLE() {
  BLEDevice::init("Screwball");  // Use a short, compatible name for iOS
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());  // Enable notifications
  pCharacteristic->setCallbacks(new CharacteristicCallBack());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setMinPreferred(0x06);  // Min advertising interval (30 ms)
  pAdvertising->setMaxPreferred(0x10);  // Max advertising interval (100 ms)
  BLEDevice::startAdvertising();
}


void setup() {

  Serial.begin(115200);

  // Create the BLE Device
  initBLE();  

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
  
  /*
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  pinMode(D9, OUTPUT);
  pinMode(D10, OUTPUT);
  pinMode(D0, OUTPUT);
  */
  digitalWrite(reset, LOW);
  digitalWrite(reset, HIGH);
  clearAll();


  /*
  //Security
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setCapability(ESP_IO_CAP_NONE);
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  pSecurity->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  pSecurity->setKeySize(16);
  */


  delay(10);

  Serial.println("Waiting a client connection to notify...");

  
}


int prevState = 69;

void loop() {
  //uint64_t loops;
  yield();
  if (prevState != value) {
    //just chill, sometimes its ok to do nothing
 
    // notify changed value
    if (deviceConnected) {
      pCharacteristic->setValue(value);
      Serial.print("state: ");
      Serial.println(value);
      //pCharacteristic->getData();
      
      setState(value);
      //loops = 0;
      
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
        delay(1000);

      } else {
        // slave pair failed
        Serial.println("Slave pair failed!");
      }
    }
  }/*else if(prevState == value){
    //loops++;
    if(deviceConnected && (prevState == value) && (loops > 999999999)){
      if(esp_sleep_enable_bt_wakeup() == ESP_OK){   //I may have gone a little bit overboard
#ifdef FUN_DEBUG
        Serial.println("Falling asleep in... ");    //with the prints here... Dont blame me,
        for(int i = 5; i >= 0; i--){
          Serial.print('\t');                       //I'm in the first chapter of K & R's C!
          Serial.println(i, DEC);
          delay(1000);
        }
        Serial.println("       now");
        for(int i = 5; i >= 0; i--){
          Serial.println("...zzZZZzzZz...");
          delay(1000);
        }
        Serial.println(" ");
        Serial.println(" ");
        Serial.println(" ");
#endif FUN_DEBUG
        esp_sleep_enable_bt_wakeup();
        loops = 0;
        if(esp_light_sleep_start() == ESP_OK){
          Serial.println("\t*\tAWAKE!\t*");
        }
      }
    }
  }*/

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
    Serial.println("Slave Found, processing...");
  } else {
    Serial.println("Slave Not Found, trying again.");
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

#endif

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


#ifdef SLAVE
// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// config AP SSID
void configDeviceAP() {
  const char *SSID = "Slave_1";
  bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    Serial.print("AP CHANNEL "); Serial.println(WiFi.channel());
  }
}

void setup() {
  //set pin modes
  pinMode(audio, OUTPUT);
  pinMode(reset, OUTPUT);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  //pinMode(battery, INPUT);
  /*pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  pinMode(D9, OUTPUT);
  pinMode(D10, OUTPUT);
  pinMode(D0, OUTPUT);
  */
  digitalWrite(reset, LOW);
  digitalWrite(reset, HIGH);
  clearAll();
  Serial.begin(115200);
  Serial.println("screwball_debug SLAVE");
  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP);
  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Recv from: "); Serial.println(macStr);
  Serial.print("Last Packet Recv Data: "); Serial.println(*data);
  Serial.println("");
  value = *data;
  Serial.println(value);
  setState(value);
}



void loop() {
  
}
#endif



