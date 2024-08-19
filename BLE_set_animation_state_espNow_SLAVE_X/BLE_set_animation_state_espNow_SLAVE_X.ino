//SLAVE

/**
   ESPNOW - Basic communication - Slave
   Date: 26th September 2017
   Author: Arvind Ravulavaru <https://github.com/arvindr21>
   Purpose: ESPNow Communication between a Master ESP32 and a Slave ESP32
   Description: This sketch consists of the code for the Slave module.
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/

   << This Device Slave >>

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

#include <esp_now.h>
#include <WiFi.h>

#define CHANNEL 2

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
int value = 0;

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
  Serial.begin(115200);
  while (!Serial) delay(10);
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
  setState(value);
}



void loop() {

}

void setState(int state) {
  state--;
  
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
