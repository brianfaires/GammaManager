#pragma once
#include "Arduino.h"
#include "FastLED.h"

#define ENABLE_COLOR_CORRECTION_TESTS

class GammaManager {
  public:
    void Init(CRGB* _leds, uint8_t* _leds_b, uint8_t* _leds_5bit_brightness, uint16_t _numLEDs, uint8_t *_globalBrightness);
    void Correct(CRGB& pixel);
    void Inverse(CRGB& pixel);
	  CRGB Blend(CRGB& a, CRGB& b, fract8 blendAmount);
	  void BlendInPlace(CRGB& a, CRGB& b, fract8 blendAmount);
    void PrepPixelsForFastLED();
    #ifdef  ENABLE_COLOR_CORRECTION_TESTS
      void RunTests(uint16_t thickness = 4, uint16_t gradientLength = 32);
    #endif
  private:
    CRGB* leds;
    uint8_t* leds_b;
    uint8_t* leds_5bit_brightness;
    uint16_t numLEDs;
    uint8_t* globalBrightness;

#ifdef ENABLE_COLOR_CORRECTION_TESTS
	  uint8_t INITIAL_TEST_BRIGHTNESS = 64;
    bool useLookupMatrices = false;
    float fGammaR = 1.40;//1.30;//1.9;//1.60;//1.75;//1.8;//1.65;//1.15;
    float fGammaG = 1.35;//1.75;//1.9;//1.75;//1.90;//1.6;//2.1;//1.65;
    float fGammaB = 1.5;//2.00;//1.8;//1.80;//2.00;//3.1;//3.1;//2.85;
  
    void RunGradientTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint16_t gradientLength = 32);
    void RunSimpleTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint8_t thickness = 4);
    void RunWhiteTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint8_t spacing);
    void RunMidpointTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint8_t thickness = 4, bool onePatternOnly = false);
	  void RunDimmingTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint16_t gradientLength);
    void WriteGammaMatrices(float gamma, int max_in = 255, int max_out = 255, String matrixNameSuffix = "", bool includeReverse = true);
    bool ProcessSerialInput();
#endif
};
