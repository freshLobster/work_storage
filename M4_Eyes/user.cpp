// SPDX-FileCopyrightText: 2019 Phillip Burgess for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#if 1 // Change to 0 to disable this code (must enable ONE user*.cpp only!)

#include "globals.h"
#include "Wire.h"
#include "SPI.h"
#include "Adafruit_MSA301.h"
#include "Adafruit_Sensor.h"

// This file provides a crude way to "drop in" user code to the eyes,
// allowing concurrent operations without having to maintain a bunch of
// special derivatives of the eye code (which is still undergoing a lot
// of development). Just replace the source code contents of THIS TAB ONLY,
// compile and upload to board. Shouldn't need to modify other eye code.

// User globals can go here, recommend declaring as static, e.g.:
// static int foo = 42;
#define G_SCALE       40.0   // Accel scale; no science, just looks good
#define ELASTICITY     0.0  // Edge-bounce coefficient (MUST be <1.0!)
#define DRAG           0.996 // Dampens motion slightly

float    x  = 0.0, y  = 0.0, // Pupil position, start at center
         vx = 0.0, vy = 0.0; // Pupil velocity (X,Y components)

void user_setup(void) {
  if(accel.begin(0x18) || accel.begin(0x19)) {
   accel.setRange(LIS3DH_RANGE_8_G);
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
void user_loop(void) {
  accel.read();

  // Get time since last frame, in floating-point seconds
  uint32_t t       = micros();
  float    elapsed = (float)(t - lastTime) / 1000000.0;
  lastTime = t;

  // Scale accelerometer readings based on an empirically-derived constant
  // (i.e. looks good, nothing scientific) and time since prior frame.
  // On HalloWing, accelerometer's Y axis is horizontal, X axis is vertical,
  // (vs screen's and conventional Cartesian coords being X horizontal,
  // Y vertical), so swap while we're here, store in ax, ay;
  float scale = G_SCALE * elapsed;
  float ax = accel.y_g * scale, // Horizontal acceleration, pixel units
        ay = accel.x_g * scale; // Vertical acceleration "

  // Add scaled accelerometer readings to pupil velocity, store interim
  // values in vxNew, vyNew...a little friction prevents infinite bounce.
  float vxNew = (vx + ax) * DRAG,
        vyNew = (vy + ay) * DRAG;

  // Limit velocity to pupil size to avoid certain overshoot situations
  float v = vxNew * vxNew + vyNew * vyNew;
  if(v > (irisRadius * irisRadius)) {
    v = irisRadius / sqrt(v);
    vxNew *= v;
    vyNew *= v;
  }

  // Add new velocity to prior position, store interim in xNew, yNew;
  float xNew = x + vxNew,
        yNew = y + vyNew;

  // Get pupil position (center point) distance-squared from origin...
  // here's why we put (0,0) at the center...
  float d = xNew * xNew + yNew * yNew;

  // Is pupil heading out of the eye constraints?  No need for a sqrt()
  // yet...since we're just comparing against a constant at this point,
  // we can square the constant instead, avoid math...
  float r2 = slitPupilRadius * slitPupilRadius; // r^2
  if(d >= r2) {

    // New pupil center position is outside the circle, now the math
    // suddenly gets intense...

    float dx = xNew - x, // Vector from old to new position
          dy = yNew - y; // (crosses slitPupilRadius perimeter)

    // Find intersections between unbounded line and circle...
    float x2   =  x * x,  //  x^2
          y2   =  y * y,  //  y^2
          a2   = dx * dx, // dx^2
          b2   = dy * dy, // dy^2
          a2b2 = a2 + b2,
          n1, n2,
          n = a2*r2 - a2*y2 + 2.0*dx*dy*x*y + b2*r2 - b2*x2;
    if((n >= 0.0) & (a2b2 > 0.0)) {
      // Because there's a square root here...
      n  = sqrt(n);
      // There's two possible intersection points.  Consider both...
      n1 =  (n - dx * x - dy * y) / a2b2;
      n2 = -(n + dx * x + dy * y) / a2b2;
    } else {
      n1 = n2 = 0.0; // Avoid divide-by-zero
    }
    // ...and use the 'larger' one (may be -0.0, that's OK!)
    if(n2 > n1) n1 = n2;
    float ix = x + dx * n1, // Single intersection point of
          iy = y + dy * n1; // movement vector and circle.

    // Pupil needs to be constrained within eye circle, but we can't just
    // stop it's motion at the edge, that's cheesy and looks wrong.  On its
    // way out, it was moving with a certain direction and speed, and needs
    // to bounce back in with suitable changes to both...

    float mag1 = sqrt(dx * dx + dy * dy), // Full velocity vector magnitude
          dx1  = (ix - x),                // Vector from prior pupil pos.
          dy1  = (iy - y),                // to point of edge intersection
          mag2 = sqrt(dx1*dx1 + dy1*dy1); // Magnitude of above vector
    // Difference between the above two magnitudes is the distance the pupil
    // will bounce back into the eye circle on this frame (i.e. it rarely
    // stops exactly at the edge...in the course of a single frame, it will
    // be moving outward a certain amount, contact edge, and move inward
    // a certain amount.  The latter amount is scaled back slightly as it
    // loses some energy in edge the collision.
    float mag3 = (mag1 - mag2) * ELASTICITY;

    float ax = -ix / slitPupilRadius, // Unit surface normal (magnitude 1.0)
          ay = -iy / slitPupilRadius, // at contact point with circle.
          rx, ry;                  // Reverse velocity vector, normalized
    if(mag1 > 0.0) {
      rx = -dx / mag1;
      ry = -dy / mag1;
    } else {
      rx = ry = 0.0;
    }
    // Dot product between the two vectors is cosine of angle between them
    float dot = rx * ax + ry * ay,
          rpx = ax * dot,          // Point to reflect across
          rpy = ay * dot;
    rx += (rpx - rx) * 2.0;        // Reflect velocity vector across point
    ry += (rpy - ry) * 2.0;        // (still normalized)

    // New position is the intersection point plus the reflected vector
    // scaled by mag3 (the elasticity-reduced velocity remainder).
    xNew = ix + rx * mag3;
    yNew = iy + ry * mag3;

    // Velocity magnitude is scaled by the elasticity coefficient.
    mag1 *= ELASTICITY;
    vxNew = rx * mag1;
    vyNew = ry * mag1;
  }

  int x1, y1, x2, y2,                        // Bounding rect of screen update area
      px1 = 64 + (int)xNew - irisRadius / 2, // Bounding rect of new pupil pos. only
      px2 = 64 + (int)xNew + irisRadius / 2 - 1,
      py1 = 64 - (int)yNew - irisRadius / 2,
      py2 = 64 - (int)yNew + irisRadius / 2 - 1;

  if(firstFrame) {
    x1 = y1 = 0;
    x2 = y2 = 127;
    firstFrame = false;
  } else {
    if(xNew >= x) { // Moving right
      x1 = 64 + (int)x    - irisRadius / 2;
      x2 = 64 + (int)xNew + irisRadius / 2 - 1;
    } else {       // Moving left
      x1 = 64 + (int)xNew - irisRadius / 2;
      x2 = 64 + (int)x    + irisRadius / 2 - 1;
    }
    if(yNew >= y) { // Moving up (still using +Y Cartesian coords)
      y1 = 64 - (int)yNew - irisRadius / 2;
      y2 = 64 - (int)y    + irisRadius / 2 - 1;
    } else {        // Moving down
      y1 = 64 - (int)y    - irisRadius / 2;
      y2 = 64 - (int)yNew + irisRadius / 2 - 1;
    }
  }

  x  = xNew;  // Save new position, velocity
  y  = yNew;
  vx = vxNew;
  vy = vyNew;

  // Clip update rect.  This shouldn't be necessary, but it looks
  // like very occasionally an off-limits situation may occur, so...
  if(x1 < 0)   x1 = 0;
  if(y1 < 0)   y1 = 0;
  if(x2 > 127) x2 = 127;
  if(y2 > 127) y2 = 127;

  eyeTargetX = map(x, 0, 127, -1, 1);
  eyeTargetY = map(y, 0, 127, -1, 1);
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
