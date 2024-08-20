#include <cmath>
#include "Arduino.h"
#include <stdlib.h>
#include <math.h>
#include "Adafruit_USBD_CDC.h"
// SPDX-FileCopyrightText: 2019 Phillip Burgess for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#if 1 // Change to 0 to disable this code (must enable ONE user*.cpp only!)

#include "globals.h"
#include <Wire.h>
#include <Adafruit_MSA301.h>
#include <Adafruit_Sensor.h>

//#define DEBUG

Adafruit_MSA311 msa;

// This file provides a crude way to "drop in" user code to the eyes,
// allowing concurrent operations without having to maintain a bunch of
// special derivatives of the eye code (which is still undergoing a lot
// of development). Just replace the source code contents of THIS TAB ONLY,
// compile and upload to board. Shouldn't need to modify other eye code.

// User globals can go here, recommend declaring as static, e.g.:
// static int foo = 42;

// Called once near the end of the setup() function. If your code requires
// a lot of time to initialize, make periodic calls to yield() to keep the
// USB mass storage filesystem alive.
void user_setup(void) {
  #ifdef DEBUG
  Serial.begin(115200);

  while (!Serial) yield();     // will pause Zero, Leonardo, etc until serial console opens
  #endif

  #ifdef DEBUG
  Serial.println("Adafruit MSA311 test!");
  #endif

  // Try to initialize!
  if (! msa.begin()) { //0x62
    
    #ifdef DEBUG
    Serial.println("Failed to find MSA311 chip");
    #endif

    while (1) { delay(10); }
  }

  #ifdef DEBUG
  Serial.println("MSA311 Found!");
  #endif

  msa.enableAxes(true, true, true);
  msa.setRange(MSA301_RANGE_4_G);
  msa.setResolution(MSA301_RESOLUTION_12);

  //msa.setDataRate(MSA301_DATARATE_31_25_HZ);

  #ifdef DEBUG
  Serial.print("Data rate set to: ");
  switch (msa.getDataRate()) {
    case MSA301_DATARATE_1_HZ: Serial.println("1 Hz"); break;
    case MSA301_DATARATE_1_95_HZ: Serial.println("1.95 Hz"); break;
    case MSA301_DATARATE_3_9_HZ: Serial.println("3.9 Hz"); break;
    case MSA301_DATARATE_7_81_HZ: Serial.println("7.81 Hz"); break;
    case MSA301_DATARATE_15_63_HZ: Serial.println("15.63 Hz"); break;
    case MSA301_DATARATE_31_25_HZ: Serial.println("31.25 Hz"); break;
    case MSA301_DATARATE_62_5_HZ: Serial.println("62.5 Hz"); break;
    case MSA301_DATARATE_125_HZ: Serial.println("125 Hz"); break;
    case MSA301_DATARATE_250_HZ: Serial.println("250 Hz"); break;
    case MSA301_DATARATE_500_HZ: Serial.println("500 Hz"); break;
    case MSA301_DATARATE_1000_HZ: Serial.println("1000 Hz"); break;
  }

  //msa.setPowerMode(MSA301_SUSPENDMODE);
  Serial.print("Power mode set to: ");
  switch (msa.getPowerMode()) {
    case MSA301_NORMALMODE: Serial.println("Normal"); break;
    case MSA301_LOWPOWERMODE: Serial.println("Low Power"); break;
    case MSA301_SUSPENDMODE: Serial.println("Suspend"); break;
  }

  //msa.setBandwidth(MSA301_BANDWIDTH_31_25_HZ);
  Serial.print("Bandwidth set to: ");
  switch (msa.getBandwidth()) {
    case MSA301_BANDWIDTH_1_95_HZ: Serial.println("1.95 Hz"); break;
    case MSA301_BANDWIDTH_3_9_HZ: Serial.println("3.9 Hz"); break;
    case MSA301_BANDWIDTH_7_81_HZ: Serial.println("7.81 Hz"); break;
    case MSA301_BANDWIDTH_15_63_HZ: Serial.println("15.63 Hz"); break;
    case MSA301_BANDWIDTH_31_25_HZ: Serial.println("31.25 Hz"); break;
    case MSA301_BANDWIDTH_62_5_HZ: Serial.println("62.5 Hz"); break;
    case MSA301_BANDWIDTH_125_HZ: Serial.println("125 Hz"); break;
    case MSA301_BANDWIDTH_250_HZ: Serial.println("250 Hz"); break;
    case MSA301_BANDWIDTH_500_HZ: Serial.println("500 Hz"); break;
  }

  //msa.setRange(MSA301_RANGE_2_G);
  Serial.print("Range set to: ");
  switch (msa.getRange()) {
    case MSA301_RANGE_2_G: Serial.println("+-2G"); break;
    case MSA301_RANGE_4_G: Serial.println("+-4G"); break;
    case MSA301_RANGE_8_G: Serial.println("+-8G"); break;
    case MSA301_RANGE_16_G: Serial.println("+-16G"); break;
  }

  //msa.setResolution(MSA301_RESOLUTION_14 );
  Serial.print("Resolution set to: ");
  switch (msa.getResolution()) {
    case MSA301_RESOLUTION_14: Serial.println("14 bits"); break;
    case MSA301_RESOLUTION_12: Serial.println("12 bits"); break;
    case MSA301_RESOLUTION_10: Serial.println("10 bits"); break;
    case MSA301_RESOLUTION_8: Serial.println("8 bits"); break;
  }
  #endif
  
}

// Called periodically during eye animation. This is invoked in the
// interval before starting drawing on the last eye (left eye on MONSTER
// M4SK, sole eye on HalloWing M0) so it won't exacerbate visible tearing
// in eye rendering. This is also SPI "quiet time" on the MONSTER M4SK so
// it's OK to do I2C or other communication across the bridge.
// This function BLOCKS, it does NOT multitask with the eye animation code,
// and performance here will have a direct impact on overall refresh rates,
// so keep it simple. Avoid loops (e.g. if animating something like a servo
// or NeoPixels in response to some trigger) and instead rely on state
// machines or similar. Additionally, calls to this function are NOT time-
// constant -- eye rendering time can vary frame to frame, so animation or
// other over-time operations won't look very good using simple +/-
// increments, it's better to use millis() or micros() and work
// algebraically with elapsed times instead.

//map function for floating point numbers
float fMap(float v, float in_min, float in_max, float out_min, float out_max) {
  return (v - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//truncates a float to a certain number of decimal places
float fchop( float a, int p){
  int c = (int) ( a * ( pow(10, ( p - 1 )))); 
  a = (float) c/( pow(10, ( p - 1 )));
  return a;

}

//for timing
uint elapsed_millis = millis();
//float previous[2] = {0, 0};


void user_loop(void) {
 

  int t = millis() - elapsed_millis;
  
  //occurance interval in ms
  if (t > 200){

    //read accelerometer and
    //calculate values 
    msa.read();
    float x = (msa.x * msa.y)/(msa.x^2); //mathemagic
    float y = msa.z;

    //msa.read();
    //x +=  (msa.x * msa.y)/(msa.x^2); //optional
    //y += msa.z;                      //second read
    
    x = (x/2000); //divide by 2000
    y = y/2000;   //dont know why
                  //but it works

    int ix, iy; //temporary int for mapping

    //floats get mapped to p positions
    //and converted to ints 
    int p = 6;

    ix = (int) fMap( x, -1, 1, 0, p );
    iy = (int) fMap( y, -1, 1, 0, p );

    //map int back to float range + offset adjustments
    x = fMap( (float) ix, 0, p, -1, 1 ); 
    x += 0.40; 
    y = fMap( (float) iy, 0, p, -1, 1 );

    #ifdef DEBUG
    Serial.printf("unchopped coords: (%f, %f)\n", x, y);
    #endif
    

    //x = (y > 0.2) ? -x : x;         //old mathemagic
    //y = ( abs(y) < 0.126 ) ? 0 : y; //adjustments

    //truncate y to 2 decimal places
    y = fchop(y, 3);
    x = fchop(x, 3);

    //center x & y within bounds
    //if ( x < 0.145 && x > -0.145 ) x = 0;
    if ( y < 0.2 && y > -0.2 ) y = 0;

    //truncate x & restrict values to
    //respective ranges
    eyeTargetX = constrain(x, -1, 1);
    eyeTargetY = constrain(y, -1, 1);
    
    elapsed_millis = millis();
    
    #ifdef DEBUG
    Serial.printf("%f , %f \n", eyeTargetX, eyeTargetY); //4 serial debug
    #endif
  }


}

#endif // 0
