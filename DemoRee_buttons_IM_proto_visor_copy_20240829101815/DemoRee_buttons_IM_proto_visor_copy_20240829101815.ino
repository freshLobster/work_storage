/// @file    DemoReel100.ino
/// @brief   FastLED "100 lines of code" demo reel, showing off some effects
/// @example DemoReel100.ino

#include <FastLED.h>
//#include <Button.h>

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014
//#include <DailyStruggleButton.h>

#define DATA_PIN    D9
#define CLK_PIN     D8
#define LED_TYPE    APA102
#define COLOR_ORDER BGR
#define NUM_LEDS    19
#define BUTTON_C     D0
#define BUTTON_A     D1
#define DEBUG

CRGB leds[NUM_LEDS];
//DailyStruggleButton colorButt;
//DailyStruggleButton patButt;  


#define BRIGHTNESS          60
#define FRAMES_PER_SECOND  120


//#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void setup() {
  delay(3000); // 3 second delay for recovery
  
  #ifdef DEBUG
  Serial.begin(115200);
  #endif

  // tell FastLED about the LED strip configuration
  //FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  
  pinMode(BUTTON_C, INPUT_PULLDOWN);
  pinMode(BUTTON_A, INPUT_PULLDOWN);


  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS/4);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { solid, eyes, sinelon, confetti, bpm, juggle, rainbow, rainbowWithGlitter};
uint8_t gPatternLength = 8;

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t colorIndex = 0;
uint8_t hues[] = {HUE_RED, HUE_RED, HUE_RED, HUE_ORANGE, HUE_ORANGE, HUE_ORANGE, HUE_YELLOW, HUE_YELLOW, HUE_YELLOW,
                           HUE_GREEN, HUE_GREEN, HUE_GREEN, HUE_AQUA, HUE_AQUA, HUE_AQUA, HUE_BLUE, HUE_BLUE, HUE_BLUE, 
                           HUE_PURPLE, HUE_PURPLE, HUE_PURPLE, HUE_PINK, HUE_PINK, HUE_PINK, HUE_RED, HUE_GREEN, HUE_BLUE};
uint8_t huesLength = 27;
bool on[NUM_LEDS] = {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1};
uint8_t brightness[3] = {BRIGHTNESS/4, BRIGHTNESS/2, BRIGHTNESS};
uint8_t brightLength = 3;
bool rainbowMode = false;
uint8_t brightIndex = 0;

int buttonStateC;            // the current reading from the input pin
int lastButtonStateC = LOW;  // the previous reading from the input pin

int buttonStateA;            // the current reading from the input pin
int lastButtonStateA = LOW;  

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTimeC = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTimeA = 0;

unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

  
void loop()
{
  
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();
  

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND);
   // read the state of the switch into a local variable:
  int readingA = digitalRead(BUTTON_A);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (readingA != lastButtonStateA) {
    // reset the debouncing timer
    lastDebounceTimeA = millis();
  }

  if ((millis() - lastDebounceTimeA) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (readingA != buttonStateA) {
      buttonStateA = readingA;

      // only toggle the LED if the new button state is HIGH
      if (buttonStateA == HIGH) {
        changePattern();
      }
    }
  }
  // read the state of the switch into a local variable:
  int readingC = digitalRead(BUTTON_C);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (readingC != lastButtonStateC) {
    // reset the debouncing timer
    lastDebounceTimeC = millis();
  }

  if ((millis() - lastDebounceTimeC) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (readingC != buttonStateC) {
      buttonStateC = readingC;

      // only toggle the LED if the new button state is HIGH
      if (buttonStateC == HIGH) {
        changeColor();
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonStateC = readingC;
  lastButtonStateA = readingA;



  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { if (rainbowMode) gHue++; } // slowly cycle the "base color" through the rainbow

}



void changeColor(){
  
      ( colorIndex < (huesLength -1) ) ? colorIndex++ : colorIndex = 0;
      
      if ( colorIndex >= (huesLength - 3) ) {
        rainbowMode = true;
        #ifdef DEBUG
        Serial.println("rainbow power activated!");
        #endif
      }else{
        rainbowMode = false;
      }
      gHue = hues[ colorIndex ];
      brightIndex = colorIndex % brightLength;
      FastLED.setBrightness( brightness[brightIndex] );

      
      #ifdef DEBUG
      Serial.println("Color Button Pressed");
      #endif

  
}

void changePattern(){
    nextPattern();
    #ifdef DEBUG
    Serial.println("Pattern Button Pressed");
    #endif
  
}


void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

//#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % gPatternLength;
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