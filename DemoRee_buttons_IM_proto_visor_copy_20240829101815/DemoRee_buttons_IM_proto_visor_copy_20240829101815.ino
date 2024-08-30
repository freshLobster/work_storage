/// @file    DemoReel100.ino
/// @brief   FastLED "100 lines of code" demo reel, showing off some effects
/// @example DemoReel100.ino

#include <FastLED.h>

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014
#include <DailyStruggleButton.h>

#define DATA_PIN    D7
#define CLK_PIN     D8
#define LED_TYPE    APA102
#define COLOR_ORDER BGR
#define NUM_LEDS    19
#define button1     D0
#define button2     D1

CRGB leds[NUM_LEDS];
DailyStruggleButton colorButt;
DailyStruggleButton patButt;  


#define BRIGHTNESS          80
#define FRAMES_PER_SECOND  120

void setup() {
  delay(3000); // 3 second delay for recovery
  Serial.begin(115200);
  // tell FastLED about the LED strip configuration
  //FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  
  pinMode(button1, INPUT_PULLDOWN);
  pinMode(button2, INPUT_PULLDOWN);

  colorButt.set(button2, changeColor, EXT_PULL_DOWN);
 
  patButt.set(button1, changePattern, EXT_PULL_DOWN);


  //attachInterrupt(button1, nextPattern, RISING);
  //attachInterrupt(button2, changeColor, RISING);


  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { solid, eyes, sinelon, confetti, bpm, juggle, rainbow, rainbowWithGlitter};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t colorIndex = 0;
uint8_t hues[8] = {HUE_RED, HUE_ORANGE, HUE_YELLOW, HUE_GREEN, HUE_AQUA, HUE_BLUE, HUE_PURPLE , HUE_PINK};
bool on[NUM_LEDS] = {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1};
bool rainbowMode = false;
  
void loop()
{
  
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();
  

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { if (rainbowMode) gHue++; } // slowly cycle the "base color" through the rainbow
  //EVERY_N_MILLISECONDS( 15 ) { patButt.poll(); } 
  //EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
  colorButt.poll();
  patButt.poll();
  
}

void changeColor(byte btnStatus){
  switch ( btnStatus ){
    case onPress:
      //if ( rainbowMode ) rainbowMode = false;  
      (colorIndex < 8) ? colorIndex++ : colorIndex = 0;
      if (colorIndex == 8) {
        rainbowMode = true;
      }else{
        rainbowMode = false;
      }
      gHue = hues[colorIndex];
      Serial.println("Color Button Pressed");
      break;

  }
  
}

void changePattern(byte btnStatus){
  if ( btnStatus == onPress ){ 
    nextPattern();
    Serial.println("Pattern Button Pressed");
  }
}


void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = RainbowColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10), LINEARBLEND);
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  uint8_t dothue = gHue;
  for( int i = 0; i < 8; i++) {
    uint8_t brt = random8(75, 200);
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, brt);
    dothue += random8(16);

  }
}

void cylon(){
  static uint8_t hue = gHue;
  //uint8_t bright = 255;
// First slide the led in one direction
	for(int i = 0; i < NUM_LEDS; i++) {
		// Set the i'th led to red 
		leds[i] = CHSV(hue, 255, 255);
		// Show the leds
		FastLED.show(); 
		// now that we've shown the leds, reset the i'th led to black
		// leds[i] = CRGB::Black;
		fadeall();
		// Wait a little bit before we loop around and do it again
		delay(5);
	}
	//Serial.print("x");

	// Now go in the other direction.  
	for(int i = (NUM_LEDS)-1; i >= 0; i--) {
		// Set the i'th led to red 
		leds[i] = CHSV(hue, 255, 255);
		// Show the leds
		FastLED.show();
		// now that we've shown the leds, reset the i'th led to black
		// leds[i] = CRGB::Black;
		fadeall();
		// Wait a little bit before we loop around and do it again
		delay(5);
	}
}



void eyes(){
  for (int i = 0; i < NUM_LEDS; i++) {
    on[i] ? leds[i] = CHSV(gHue, 255, 255) : leds[i] = CRGB::Black ;
  }


}

void solid(){
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(gHue, 255, 255);
  }
}