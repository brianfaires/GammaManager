#ifndef GAMMA_MANAGER_H
#define GAMMA_MANAGER_H

#include "Arduino.h"
#include "FastLED.h"

class GammaManager {
  public:
    void Init(uint32_t colorCorrection, const uint8_t* gamR=NULL, const uint8_t* gamG=NULL, const uint8_t* gamB=NULL, const uint8_t* gamDim=NULL, const uint8_t* gamRr=NULL, const uint8_t* gamGr=NULL, const uint8_t* gamBr=NULL);
    void Correct(CRGB& pixel);
    void Inverse(CRGB& pixel);
    void ScaleBrightness(CRGB& pixel, uint8_t brightness);
    void SetPixel(CRGB& pixel, uint8_t brightness);
    void FixFloors(CRGB* leds, uint16_t numLEDs);
    
    void RunTests(CRGB* leds, uint16_t numLEDs, uint16_t thickness = 4, uint16_t gradientLength = 32, uint32_t defaultColorCorrection = 0xFFFFFF, uint32_t defaultTemp = 0xFFFFFF);

  private:
	const uint8_t* gammaR;
	const uint8_t* reverseGammaR;
	const uint8_t* gammaG;
	const uint8_t* reverseGammaG;
	const uint8_t* gammaB;
	const uint8_t* reverseGammaB;
	const uint8_t* gammaDim;
    uint8_t minBrightness_R = 1;
    uint8_t minBrightness_G = 1;
    uint8_t minBrightness_B = 1;

    // For tests only
    uint8_t b = 255; // default brightness
    uint32_t colCorrection;
    uint32_t temp;
    bool useLookupMatrices = false;
    
    float fGammaR = 1.30;//1.9;//1.60;//1.75;//1.8;//1.65;//1.15;
    float fGammaG = 1.75;//1.9;//1.75;//1.90;//1.6;//2.1;//1.65;
    float fGammaB = 2.00;//1.8;//1.80;//2.00;//3.1;//3.1;//2.85;
    float fGammaDim = 1.50;
  
    void RunSimpleTest(CRGB* leds, uint16_t numLEDs, uint8_t thickness = 4);
    void RunGradientTest(CRGB* leds, uint16_t numLEDs, uint16_t gradientLength = 32, uint32_t defaultColorCorrection = 0xFFFFFF);
    void RunWhiteTest(CRGB* leds, uint16_t numLEDs, uint8_t spacing);
    void RunMidpointTest(CRGB* leds, uint16_t numLEDs, uint8_t thickness = 4, bool onePatternOnly = false);
    void WriteGammaMatrices(float gamma, int max_in = 255, int max_out = 255, String matrixNameSuffix = "", bool includeReverse = true);
    bool ProcessSerialInput();
};

#endif

