#pragma once

#include "Arduino.h"
#include "FastLED.h"

class GammaManager {
  public:
    void Init(const uint8_t* gamR=NULL, const uint8_t* gamG=NULL, const uint8_t* gamB=NULL, const uint8_t* gamRr=NULL, const uint8_t* gamGr=NULL, const uint8_t* gamBr=NULL, uint8_t *bright=NULL);
    void Correct(CRGB& pixel);
    void Inverse(CRGB& pixel);
	CRGB Blend(CRGB& a, CRGB& b, fract8 blendAmount);
	void BlendInPlace(CRGB& a, CRGB& b, fract8 blendAmount);
    void RunTests(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint16_t thickness = 4, uint16_t gradientLength = 32);

  private:
	const uint8_t* gammaR;
	const uint8_t* reverseGammaR;
	const uint8_t* gammaG;
	const uint8_t* reverseGammaG;
	const uint8_t* gammaB;
	const uint8_t* reverseGammaB;

    // For tests only
    uint8_t* brightness; // default brightness
    bool useLookupMatrices = false;
	uint32_t colCorrection = 0xFFFFFF;
    
	uint8_t INITIAL_BRIGHTNESS = 64;
    float fGammaR = 1.40;//1.30;//1.9;//1.60;//1.75;//1.8;//1.65;//1.15;
    float fGammaG = 1.35;//1.75;//1.9;//1.75;//1.90;//1.6;//2.1;//1.65;
    float fGammaB = 1.5;//2.00;//1.8;//1.80;//2.00;//3.1;//3.1;//2.85;
  
    void RunSimpleTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint8_t thickness = 4);
    void RunGradientTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint16_t gradientLength = 32);
    void RunWhiteTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint8_t spacing);
    void RunMidpointTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint8_t thickness = 4, bool onePatternOnly = false);
	void RunDimmingTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint16_t gradientLength);
    void WriteGammaMatrices(float gamma, int max_in = 255, int max_out = 255, String matrixNameSuffix = "", bool includeReverse = true);
    bool ProcessSerialInput();
};
