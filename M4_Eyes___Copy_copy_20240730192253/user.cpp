// SPDX-FileCopyrightText: 2019 Phillip Burgess for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#if 1 // Change to 0 to disable this code (must enable ONE user*.cpp only!)

#include "globals.h"
#include <Wire.h>
#include <Adafruit_MSA301.h>
#include <Adafruit_Sensor.h>

Adafruit_MSA301 msa;

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
  Serial.begin(115200);
  //while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MSA301 test!");
  
  // Try to initialize!
  if (! msa.begin()) {
    Serial.println("Failed to find MSA301 chip");
    while (1) { delay(10); }
  }
  Serial.println("MSA301 Found!");

  //msa.setDataRate(MSA301_DATARATE_31_25_HZ);
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

uint elapsed_millis = millis();

void user_loop(void) {
  int t = millis() - elapsed_millis;
  if (t > 200){
    msa.read();
    float x = msa.x;
    float y = msa.z;
    msa.read();
    //x += msa.x;
    y += msa.z;
    
    x = x/2000;
    y = y/3000;

    eyeTargetX = constrain(x, -1, 1);
    eyeTargetY = constrain(y, -0.2, 0.3);
    elapsed_millis = millis();
  }


/*
  Suppose we have a global bool "animating" (meaning something is in
  motion) and global uint32_t's "startTime" (the initial time at which
  something triggered movement) and "transitionTime" (the total time
  over which movement should occur, expressed in microseconds).
  Maybe it's servos, maybe NeoPixels, or something different altogether.
  This function might resemble something like (pseudocode):

  if(!animating) {
    Not in motion, check sensor for trigger...
    if(read some sensor) {
      Motion is triggered! Record startTime, set transition
      to 1.5 seconds and set animating flag:
      startTime      = micros();
      transitionTime = 1500000;
      animating      = true;
      No motion actually takes place yet, that will begin on
      the next pass through this function.
    }
  } else {
    Currently in motion, ignore trigger and move things instead...
    uint32_t elapsed = millis() - startTime;
    if(elapsed < transitionTime) {
      Part way through motion...how far along?
      float ratio = (float)elapsed / (float)transitionTime;
      Do something here based on ratio, 0.0 = start, 1.0 = end
    } else {
      End of motion reached.
      Take whatever steps here to move into final position (1.0),
      and then clear the "animating" flag:
      animating = false;
    }
  }
*/
}

#endif // 0
