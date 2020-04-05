#include "GammaManager.h"
#include "GammaManagerConfig.h"

// Applies gamma correction from matrices defined in the main project
void GammaManager::Correct(CRGB& pixel) {
  pixel.r = pgm_read_byte(&gammaR[pixel.r]);
  pixel.g = pgm_read_byte(&gammaG[pixel.g]);
  pixel.b = pgm_read_byte(&gammaB[pixel.b]);
}

// Inverse function of Correct(); Use on already corrected colors prior to interpolating and re-correcting
void GammaManager::Inverse(CRGB& pixel) {
  pixel.r = pgm_read_byte(&reverseGammaR[pixel.r]);
  pixel.g = pgm_read_byte(&reverseGammaG[pixel.g]);
  pixel.b = pgm_read_byte(&reverseGammaB[pixel.b]);
}

// Initialize the Gamma controller with a pointer to the global brightness variable
void GammaManager::Init(CRGB* _leds, uint8_t* _leds_b, uint8_t* _leds_5bit_brightness, uint16_t _numLEDs, uint8_t *_globalBrightness) {
    leds = _leds;
    leds_b = _leds_b;
    leds_5bit_brightness = _leds_5bit_brightness;
    numLEDs = _numLEDs;
    globalBrightness = _globalBrightness;
    #ifdef ENABLE_COLOR_CORRECTION_TESTS
      SetAllColorCorrections(0xFFFFFF);
    #endif
}

// Blend two previously-corrected CRGBs using Gamma correction
CRGB GammaManager::Blend(CRGB& a, CRGB& b, fract8 blendAmount) {
	CRGB tempA = a;
	CRGB tempB = b;
	Inverse(tempA);
	Inverse(tempB);
	CRGB retVal = blend(tempA, tempB, blendAmount);
	Correct(retVal);
	return retVal;
}

// Destructively blend two previously-corrected CRGBs using Gamma correction
void GammaManager::BlendInPlace(CRGB& a, CRGB& b, fract8 blendAmount) {
	Inverse(a);
	CRGB temp = b;
	Inverse(temp);
	nblend(a, temp, blendAmount);
	Correct(a);
}

void GammaManager::PrepPixelsForFastLED() { // Todo: Optimize performance of this function 
  if(*globalBrightness == 0) {
    for(uint16_t i = 0; i < numLEDs; i++) { leds_5bit_brightness[i] = 0; }
    return;
  }

  for(uint16_t i = 0; i < numLEDs; i++) {
    if(leds_b[i] == 0) {
      leds_5bit_brightness[i] = 0;
    }
    else {
      uint16_t brightness = *globalBrightness * leds_b[i] / 0xFF;
      if(brightness == 0) { brightness = 1; }
      leds_5bit_brightness[i] = pgm_read_byte(&gammaDim_5bit[brightness]);

      // Dimming logic; all done linearly
      uint8_t dimAmount = pgm_read_byte(&gammaDim[brightness]);
      CRGB correction = colorCorrections[leds_5bit_brightness[i]];
      correction.nscale8_video(dimAmount);
      leds[i].nscale8(correction); 
      
      #ifdef ENABLE_COLOR_CORRECTION_TESTS
        if(useLookupMatrices) {
            Correct(leds[i]);
        }
        else {
          leds[i].r = applyGamma_video(leds[i].r, fGammaR);
          leds[i].g = applyGamma_video(leds[i].g, fGammaG);
          leds[i].b = applyGamma_video(leds[i].b, fGammaB);
        }
      #else
        Correct(leds[i]);
      #endif
    }
  }
}


#ifdef ENABLE_COLOR_CORRECTION_TESTS
// The main test loop with serial IO
void GammaManager::RunTests(uint16_t thickness, uint16_t gradientLength) {
  static uint8_t curMode = 0;
  *globalBrightness = INITIAL_TEST_BRIGHTNESS;
  for(uint16_t i = 0; i < numLEDs; i++) { leds_b[i] = 255; }

  while(true) {
    if(curMode == 0) {
      // The most common test; A gradient of RGB at the start and a gradient of HSV at the end
      RunGradientTest(leds, leds_b, numLEDs, gradientLength);
    }
    else if(curMode == 1) { 
      // A single pattern of RGB
      if(numLEDs > 6*(thickness+1)) { RunSimpleTest(leds, leds_b, 3*(thickness+1), thickness); }
    }
    else if(curMode == 2) {
      // A repeating pattern of RGB
      RunSimpleTest(leds, leds_b, numLEDs, thickness);
    }
    else if(curMode == 3) {
      // A single strip of white
      RunWhiteTest(leds, leds_b, min(uint16_t(numLEDs), uint16_t(2*thickness)), 0); // todo: investigate this: this is a hack to get around a bug in the esp libraries
	    if(numLEDs > 4*thickness) {
        RunWhiteTest(&leds[numLEDs-2*thickness], &leds_b[numLEDs-2*thickness], 2*thickness, 0);
      }
    }
    else if(curMode == 4) {
      // White every third pixel
      RunWhiteTest(leds, leds_b, numLEDs, 2);
    }
    else if(curMode == 5) {
      // White every pixel
      RunWhiteTest(leds, leds_b, numLEDs, 0);
    }
    else if(curMode == 6) {
      // One stripe of colors with middle colors drawn; brightness derived from gammaDim
      RunMidpointTest(leds, leds_b, numLEDs, thickness, true);
    }
    else if(curMode == 7) {
      // Stripes of colors with middle colors drawn; brightness derived from gammaDim
      RunMidpointTest(leds, leds_b, numLEDs, thickness);
    }
    else if(curMode == 8) {
      // One long stretch of white to see how overall dimming works
      RunDimmingTest(leds, leds_b, numLEDs, gradientLength);
    }

    for(uint16_t i = 0; i < numLEDs; i++) { leds[i] = CRGB::Black; }
    curMode = (curMode+1) % 9;
  }
}

// ------------ Tests ------------
// Draws a gradient (or double gradient if room) of RGB from the front end, and HSV from the back end
void GammaManager::RunGradientTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint16_t gradientLength) {
  while(numLEDs < 6*gradientLength) { gradientLength--; }
  bool doDouble = numLEDs > 12*gradientLength;

  do {
    fill_gradient(&leds[numLEDs-3*gradientLength], 3*gradientLength, CHSV(255,255,255), CHSV(0,255,255), LONGEST_HUES);
    for(int i = numLEDs-3*gradientLength; i < numLEDs; i++) { Inverse(leds[i]); }
    if(doDouble) {
      fill_gradient(&leds[numLEDs-6*gradientLength], 3*gradientLength, CHSV(255,255,255), CHSV(0,255,255), LONGEST_HUES);
      for(int i = numLEDs-6*gradientLength; i < numLEDs-3*gradientLength; i++) { Inverse(leds[i]); }
    }
    
    fill_gradient_RGB(leds,   0, CRGB(255,0,0),  gradientLength, CRGB(0,255,0));
    fill_gradient_RGB(leds,  gradientLength, CRGB(0,255,0),2*gradientLength, CRGB(0,0,255));
    fill_gradient_RGB(leds,2*gradientLength, CRGB(0,0,255),3*gradientLength, CRGB(255,0,0));
    if(doDouble) {
      fill_gradient_RGB(leds,3*gradientLength, CRGB(255,0,0),4*gradientLength, CRGB(0,255,0));
      fill_gradient_RGB(leds,4*gradientLength, CRGB(0,255,0),5*gradientLength, CRGB(0,0,255));
      fill_gradient_RGB(leds,5*gradientLength, CRGB(0,0,255),6*gradientLength, CRGB(255,0,0));
    }

    PrepPixelsForFastLED();
	  FastLED.show();
  } while(ProcessSerialInput());
}

// Shows bands of pure hues for color balancing
void GammaManager::RunSimpleTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint8_t thickness) {
  uint16_t period = 3*(thickness+1);
  while(numLEDs < period) { thickness--; }
  for(uint16_t i = 0; i < numLEDs; i++) { leds[i] = CRGB::Black; }

  do {
		for(int i=0; i+period <= numLEDs; i+=period) {
      for(int j = 0; j < thickness; j++) {
        leds[i+j] = CRGB(255,0,0);
        leds[i+j+thickness+1] = CRGB(0,255,0);
        leds[i+j+2*(thickness+1)] = CRGB(0,0,255);
        
        leds_b[i+j] = 255;
        leds_b[i+j+thickness+1] = 255;
        leds[i+j+2*(thickness+1)] = 255;
      }
    }

    PrepPixelsForFastLED();
	  FastLED.show();
  } while(ProcessSerialInput());
}

// Shows bands of white LEDs for color balancing and red shift detection
void GammaManager::RunWhiteTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint8_t spacing) {
  uint8_t interval = spacing + 1;
  
  do {
	  for(uint16_t i = 0; i < numLEDs; i+=interval) {
      leds[i] = CRGB(255, 255, 255);
      leds_b[i] = 255;
	  }

	  PrepPixelsForFastLED();
	  FastLED.show();
  } while(ProcessSerialInput());
}

// Shows bands of pure and midpoint colors for color balancing and gamma tuning
void GammaManager::RunMidpointTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint8_t thickness, bool onePatternOnly) {
  uint16_t period = 6*thickness;
  while(numLEDs < period) { thickness--; }
  
  do {
    for(int i=0; i+period < numLEDs; i+=period) {
      for(int j = 0; j < thickness; j++) {
        leds[i+j] = CRGB(255,0,0);
        leds[i+j+2*thickness] = CRGB(0,255,0);
        leds[i+j+4*thickness] = CRGB(0,0,255);
        leds_b[i+j] = 255;
        leds_b[i+j+2*thickness] = 255;
        leds_b[i+j+4*thickness] = 255;
        
        leds[i+j+thickness] = CRGB(128,128,0);
        leds[i+j+3*thickness] = CRGB(0,128,128);
        leds[i+j+5*thickness] = CRGB(128,0,128);
        leds_b[i+j+thickness] = 255;
        leds_b[i+j+3*thickness] = 255;
        leds_b[i+j+5*thickness] = 255;
      }
      if(onePatternOnly) { break; }
    }
    
    PrepPixelsForFastLED();
	  FastLED.show();
  } while(ProcessSerialInput());
}

// Draws dimmed gradients: white, yellow, fusia
void GammaManager::RunDimmingTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint16_t gradientLength) {
	uint8_t length = 2*gradientLength;
	if(numLEDs < length) { length = numLEDs; }
	float fadeStepSize = 255.0 / length;
	
	do {
		for(uint16_t i = 0; i < length; i++) {
			leds[i] = CRGB(255,255,255);
			leds_b[i] = (i+1)*fadeStepSize;
			
			if(numLEDs >= 2*length+5) {
				leds[i+length+5] = CRGB(255,255,0);
				leds_b[i+length+5] = (i+1)*fadeStepSize;
			}
			
			if(numLEDs >= 3*length+10) {
				//leds[i+2*length+10].r = colCorrection >> 8;
				//leds[i+2*length+10].b = leds[i+2*length+10].r >> 3;
				leds[i+2*length+10] = CRGB(255,0,255);
        leds_b[i+2*length+10] = (i+1)*fadeStepSize;
			}
		}
		
		PrepPixelsForFastLED();
	  FastLED.show();
  } while(ProcessSerialInput());
}

// Handles serial IO while running RunTests()
bool GammaManager::ProcessSerialInput() {
  Serial.println("\nTo edit Gamma, enter: r,g,b, or a(all). 'w' to write matrices. 'u' to toggle matrix use");
  Serial.println("'###' sets brightness (1-255). 'n' for next pattern. 'c' for colorCorrection");
  Serial.println("brightness +/- with '-', '='. Shift for *2");
  uint32_t colCorrectInt = (colorCorrections[0].r<<16) + (colorCorrections[0].g<<8)+ (colorCorrections[0].b);
  Serial.println("Brightness: " + String(*globalBrightness) + ", Color Correction: 0x" + String(colCorrectInt, HEX));
  if(useLookupMatrices) { Serial.println("------------- Using matrices defined in your project -------------"); }
  else { Serial.println("Gammas: R:" + String(fGammaR) + "\tG:" + String(fGammaG) + "\tB:" + String(fGammaB)); }
  
  while(Serial.available() == 0) ;
  String s = Serial.readString();
  s.trim();
  
  if(s == "-") {
	  *globalBrightness = *globalBrightness - 1;
  }
  else if(s == "_") {
	  *globalBrightness = *globalBrightness - 2;
  }
  else if(s == "=") {
	  *globalBrightness = *globalBrightness + 1;
  }
  else if(s == "+") {
	*globalBrightness = *globalBrightness + 2;
  }
  else if(s == "n") {
    return false;
  }
  else if(s == "u") {
    useLookupMatrices = !useLookupMatrices;
  }
  else if(s == "a") {
    Serial.println("Enter new gamma value.");
    while(Serial.available() == 0) ;
    s = Serial.readString();
    s.trim();
    fGammaR = s.toFloat();
    fGammaG = fGammaR;
    fGammaB = fGammaR;
  }
  else if(s == "r") {
    Serial.println("Enter new red gamma value. (Current is " + String(fGammaR) + ")");
    while(Serial.available() == 0) ;
    s = Serial.readString();
    s.trim();
    fGammaR = s.toFloat();
  }
  else if(s == "g") {
    Serial.println("Enter new green gamma value. (Current is " + String(fGammaG) + ")");
    while(Serial.available() == 0) ;
    s = Serial.readString();
    s.trim();
    fGammaG = s.toFloat();    
  }
  else if(s == "b") {
    Serial.println("Enter new blue gamma value. (Current is " + String(fGammaB) + ")");
    while(Serial.available() == 0) ;
    s = Serial.readString();
    s.trim();
    fGammaB = s.toFloat();
  }
  else if(s == "w") {
    Serial.println("\n");
    WriteGammaMatrices(fGammaR, 255, 255, "R");
    WriteGammaMatrices(fGammaG, 255, 255, "G");
    WriteGammaMatrices(fGammaB, 255, 255, "B");
    Serial.println();
  }
  else if(s == "c") {
    Serial.println("Enter new color correction as a 6-digit hex number");
    while(Serial.available() == 0) ;
    s = Serial.readString();
    s.trim();
    s.toUpperCase();

    uint32_t colCorrection = 0;
    for(int i=0; s[i]; i++){
      if(s[i] >= '0' && s[i] <= '9') {
          colCorrection *= 0x10;
          colCorrection += s[i] - '0';
      }
      else {
          colCorrection *= 0x10;
          colCorrection += s[i] - 'A' + 10;
      }
    }

    SetAllColorCorrections(colCorrection);
  }
  else {
    *globalBrightness = s.toInt();
  }

  return true;
}

// Uses current gamma and dimming values to output matrices, which should be pasted into the main project
void GammaManager::WriteGammaMatrices(float gamma, int max_in, int max_out, String matrixNameSuffix, bool includeReverse) {
  int forwardGamma[256];
  int reverseGamma[256];
  for(int i=0; i <= 255; i++) {
    forwardGamma[i] = i >= max_in ? max_out : (int)(pow((float)i / (float)max_in, gamma) * max_out + 0.5);
    if(i > 0 && forwardGamma[i] == 0) { forwardGamma[i] = 1; }
  }

  // Write reverse gamma matrix
  int iForward = 0;
  int iScaledLast = -1;
  for(int i = 0; i <= 255; i++) {
    // Scale before lookup
    int iScaled = i * max_out / 255;
    if(iScaled == 0 && i > 0) { iScaled = 1; }
    if(iScaledLast == iScaled) {
      reverseGamma[i] = reverseGamma[i-1];
      continue;
    }
    iScaledLast = iScaled;
    
    if(iForward >= 255) {
      reverseGamma[i] = 255;
    }
    else if(forwardGamma[iForward] == forwardGamma[iForward+1]) {
      int iForwardInit = iForward;
      while(forwardGamma[iForward] == forwardGamma[iForwardInit]) { iForward++; }
      iForward--;
      // indexes now span the range of gammas that match this index
      reverseGamma[i] = (iForward + iForwardInit) / 2;
      iForward++;
    }
    else if(forwardGamma[iForward] == iScaled) {
      reverseGamma[i] = iForward;
      iForward++;
    }
    else {
      // Skipped a value in the gamma matrix; determine which one is closer to i
      if(iScaled - forwardGamma[iForward - 1] <= forwardGamma[iForward] - iScaled) {
        reverseGamma[i] = iForward - 1;
      }
      else {
        reverseGamma[i] = iForward;
      }
    }
  }
  
  // Write forward gamma matrix to serial
  Serial.print("const uint8_t PROGMEM gamma" + matrixNameSuffix + "[] = {");
  for(int i=0; i<=255; i++) {
    if(i > 0) Serial.print(',');
    if((i & 15) == 0) Serial.print("\n  ");
    Serial.print(forwardGamma[i]);
  }
  Serial.println(" };\n");


  if(includeReverse) {
    // Write reverse gamma matrix to serial
    Serial.print("const uint8_t PROGMEM reverseGamma" + matrixNameSuffix + "[] = {");
    for(int i=0; i<=255; i++) {
      if(i > 0) Serial.print(',');
      if((i & 15) == 0) Serial.print("\n  ");
      Serial.print(reverseGamma[i]);
    }
    Serial.println(" };\n");
  }
}

#endif