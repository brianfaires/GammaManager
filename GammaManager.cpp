#include "GammaManager.h"

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

// Sets and scales 5-bit and 8-bit brightnesses according to the gamma matrices defined in the main project
void GammaManager::SetBrightness(CRGB& pixel, uint8_t& pixel_b, uint8_t brightness) {
  pixel_b = pgm_read_byte(&gammaDim_5bit[brightness]);
  pixel %= pgm_read_byte(&gammaDim[brightness]);
  /*const uint8_t breakpoint = 64; // The point that led_b reaches 1
    
  if(brightness <= breakpoint) {
    pixel_b = 1;
    pixel %= pgm_read_byte(&gammaDim[255 * brightness / breakpoint]);
  }
  else {
    pixel_b = 1 + 30 * (brightness - breakpoint) / (255 - breakpoint);
	Correct_5bit(pixel_b);
  }*/
}

// Shortcut to apply gamma and brightness scaling
void GammaManager::SetPixel(CRGB& pixel, uint8_t& pixel_b, uint8_t brightness) {
  Correct(pixel);
  SetBrightness(pixel, pixel_b, brightness);
}

// Initialize the Gamma controller. Optionally ignore the gamma matrix references to only adjust color correction floors
void GammaManager::Init(uint32_t colorCorrection, const uint8_t* gamR, const uint8_t* gamG, const uint8_t* gamB, const uint8_t* gamDim, const uint8_t* gamDim5, const uint8_t* gamRr, const uint8_t* gamGr, const uint8_t* gamBr) {
  if(gamR != NULL) { gammaR = gamR; }
  if(gamG != NULL) { gammaG = gamG; }
  if(gamB != NULL) { gammaB = gamB; }
  if(gamDim != NULL) { gammaDim = gamDim; }
  if(gamDim5 != NULL) { gammaDim_5bit = gamDim5; }
  if(gamRr != NULL) { reverseGammaR = gamRr; }
  if(gamGr != NULL) { reverseGammaG = gamGr; }
  if(gamBr != NULL) { reverseGammaB = gamBr; }
  
  uint16_t correctionR = colorCorrection >> 16;
  uint16_t correctionG = (colorCorrection  & 0xFF00) >> 8;
  uint16_t correctionB = colorCorrection & 0xFF;

  minBrightness_R = 1;
  if(correctionR > 0) {
    minBrightness_R = 255 / correctionR;
    if(minBrightness_R * correctionR < 255) { minBrightness_R++; }
  }

  minBrightness_G = 1;
  if(correctionG > 0) {
    minBrightness_G = 255 / correctionG;
    if(minBrightness_G * correctionG < 255) { minBrightness_G++; }
  }
  
  minBrightness_B = 1;
  if(correctionB > 0) {
    minBrightness_B = 255 / correctionB;
    if(minBrightness_B * correctionB < 255) { minBrightness_B++; }\
  }
}

// Makes sure that FastLED's COLOR_CORRECTION does not turn 1's and 2's into 0's
void GammaManager::FixFloors(CRGB* leds, uint16_t numLEDs) {
  // For speed's sake, do a 7 way conditional to avoid pointless checks against a maxed out color
  if(minBrightness_R > 1) {
    if(minBrightness_G > 1) {
      if(minBrightness_B > 1) {
        for(uint16_t i = 0; i < numLEDs; i++) {
          if(leds[i].r != 0 && leds[i].r < minBrightness_R) { leds[i].r = minBrightness_R; }
          if(leds[i].g != 0 && leds[i].g < minBrightness_G) { leds[i].g = minBrightness_G; }
          if(leds[i].b != 0 && leds[i].b < minBrightness_B) { leds[i].b = minBrightness_B; }
        }
      }
      else {
        for(uint16_t i = 0; i < numLEDs; i++) {
          if(leds[i].r != 0 && leds[i].r < minBrightness_R) { leds[i].r = minBrightness_R; }
          if(leds[i].g != 0 && leds[i].g < minBrightness_G) { leds[i].g = minBrightness_G; }
        }
      }
    }
    else {
      // R>1, G<=1
      if(minBrightness_B > 1) {
        for(uint16_t i = 0; i < numLEDs; i++) {
          if(leds[i].r != 0 && leds[i].r < minBrightness_R) { leds[i].r = minBrightness_R; }
          if(leds[i].b != 0 && leds[i].b < minBrightness_B) { leds[i].b = minBrightness_B; }
        }
      }
      else {
        for(uint16_t i = 0; i < numLEDs; i++) {
          if(leds[i].r != 0 && leds[i].r < minBrightness_R) { leds[i].r = minBrightness_R; }
        }
      }
    }
  }
  else {
    // R<=1
    if(minBrightness_G > 1) {
      if(minBrightness_B > 1) {
        for(uint16_t i = 0; i < numLEDs; i++) {
          if(leds[i].g != 0 && leds[i].g < minBrightness_G) { leds[i].g = minBrightness_G; }
          if(leds[i].b != 0 && leds[i].b < minBrightness_B) { leds[i].b = minBrightness_B; }
        }
      }
      else {
        for(uint16_t i = 0; i < numLEDs; i++) {
          if(leds[i].g != 0 && leds[i].g < minBrightness_G) { leds[i].g = minBrightness_G; }
        }
      }
    }
    else {
      // R<=1, G<=1
      if(minBrightness_B > 1) {
        for(uint16_t i = 0; i < numLEDs; i++) {
          if(leds[i].b != 0 && leds[i].b < minBrightness_B) { leds[i].b = minBrightness_B; }
        }
      }
    }
  }  
}

// The main test loop with serial IO
void GammaManager::RunTests(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint16_t thickness, uint16_t gradientLength, uint32_t defaultColorCorrection, uint32_t defaultTemp) {
  static uint8_t curMode = 0;
  colCorrection = defaultColorCorrection;
  temp = defaultTemp;
  FastLED.setCorrection(colCorrection);
  FastLED.setTemperature(temp);
  Init(colCorrection);
  
  while(true) {
    if(curMode == 0) {
      // The most common test; A gradient of RGB at the start and a gradient of HSV at the end
      RunGradientTest(leds, leds_b, numLEDs, gradientLength, defaultColorCorrection);
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
      RunWhiteTest(leds, leds_b, min(numLEDs, thickness), 0);
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
		RunDimmingTest(leds, leds_b, numLEDs);
	}

    for(uint16_t i = 0; i < numLEDs; i++) { leds[i] = CRGB::Black; }
    curMode = (curMode+1) % 9;
  }
}

// ------------ Tests ------------
// Shows bands of pure hues for color balancing
void GammaManager::RunSimpleTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint8_t thickness) {
  uint16_t period = 3*(thickness+1);
  while(numLEDs < period) { thickness--; }
  for(uint16_t i = 0; i < numLEDs; i++) { leds[i] = CRGB::Black; }

  do {
    if(useLookupMatrices) {
      for(int i=0; i+period <= numLEDs; i+=period) {
        for(int j = 0; j < thickness; j++) {
          leds[i+j] = CRGB(255,0,0);
          leds[i+j+thickness+1] = CRGB(0,255,0);
          leds[i+j+2*(thickness+1)] = CRGB(0,0,255);
          SetPixel(leds[i+j], leds_b[i+j], b);
          SetPixel(leds[i+j+thickness+1], leds_b[i+j+thickness+1], b);
          SetPixel(leds[i+j+2*(thickness+1)], leds_b[i+j+2*(thickness+1)], b);
        }
      }
    }
    else {
      for(int i=0; i+period <= numLEDs; i+=period) {
        for(int j = 0; j < thickness; j++) {
          leds[i+j] = CRGB(255,0,0);
          leds[i+j+thickness+1] = CRGB(0,255,0);
          leds[i+j+2*(thickness+1)] = CRGB(0,0,255);
		  SetBrightness(leds[i+j], leds_b[i+j], b);
		  SetBrightness(leds[i+j+thickness+1], leds_b[i+j+thickness+1], b);
		  SetBrightness(leds[i+j+2*(thickness+1)], leds_b[i+j+2*(thickness+1)], b);
        }
      }
    }

    FixFloors(leds, numLEDs);
    FastLED.show();
  } while(ProcessSerialInput());
}

// Shows bands of white LEDs for color balancing and red shift detection
void GammaManager::RunWhiteTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint8_t spacing) {
  uint8_t interval = spacing + 1;
  
  do {
    if(useLookupMatrices) {
      for(uint16_t i = 0; i < numLEDs; i+=interval) {
        leds[i] = CRGB(255,255,255);
        SetPixel(leds[i], leds_b[i], b);
      }
    }
    else {
      for(uint16_t i = 0; i < numLEDs; i+=interval) {
        leds[i] = CRGB(255,255,255);
        SetBrightness(leds[i], leds_b[i], b);
	  }
    }

    FixFloors(leds, numLEDs);
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
		SetBrightness(leds[i+j], leds_b[i+j], b);
		SetBrightness(leds[i+j+2*thickness], leds_b[i+j+2*thickness], b);
		SetBrightness(leds[i+j+4*thickness], leds_b[i+j+4*thickness], b);
        if(useLookupMatrices) {
		  leds[i+j+thickness] = CRGB(128,128,0);
		  leds[i+j+3*thickness] = CRGB(0,128,128);
		  leds[i+j+5*thickness] = CRGB(128,0,128);
		  SetPixel(leds[i+j+thickness], leds_b[i+j+thickness], b);
		  SetPixel(leds[i+j+3*thickness], leds_b[i+j+3*thickness], b);
		  SetPixel(leds[i+j+5*thickness], leds_b[i+j+5*thickness], b);
        }
        else {
		  uint8_t adjR = b/2 * pow(2, 1/fGammaR);
		  uint8_t adjG = b/2 * pow(2, 1/fGammaG);
		  uint8_t adjB = b/2 * pow(2, 1/fGammaB);
		  leds[i+j+thickness] = CRGB(adjR, adjG,0);
		  leds[i+j+3*thickness] = CRGB(0, adjG, adjB);
		  leds[i+j+5*thickness] = CRGB(adjR, 0, adjB);
		  SetBrightness(leds[i+j+thickness], leds_b[i+j+thickness], b);
		  SetBrightness(leds[i+j+3*thickness], leds_b[i+j+3*thickness], b);
		  SetBrightness(leds[i+j+5*thickness], leds_b[i+j+5*thickness], b);
        }
      }
      if(onePatternOnly) { break; }
    }
    
    FixFloors(leds, numLEDs);
    FastLED.show();
  } while(ProcessSerialInput());
}

// Draws a gradient (or double gradient if room) of RGB from the front end, and HSV from the back end
void GammaManager::RunGradientTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs, uint16_t gradientLength, uint32_t defaultColorCorrection) {
  while(numLEDs < 6*gradientLength) { gradientLength--; }
  bool doDouble = numLEDs > 12*gradientLength;
  int limit = doDouble ? 6*gradientLength : 3*gradientLength;
  
  do {
    fill_gradient(&leds[numLEDs-3*gradientLength], 3*gradientLength, CHSV(255,255,b), CHSV(0,255,b), LONGEST_HUES);
    if(doDouble) {
      fill_gradient(&leds[numLEDs-6*gradientLength], 3*gradientLength, CHSV(255,255,b), CHSV(0,255,b), LONGEST_HUES);
    }
    
	
    fill_gradient_RGB(leds,   0, CRGB(255,0,0),  gradientLength, CRGB(0,255,0));
    fill_gradient_RGB(leds,  gradientLength, CRGB(0,255,0),2*gradientLength, CRGB(0,0,255));
    fill_gradient_RGB(leds,2*gradientLength, CRGB(0,0,255),3*gradientLength, CRGB(255,0,0));
    if(doDouble) {
      fill_gradient_RGB(leds,3*gradientLength, CRGB(255,0,0),4*gradientLength, CRGB(0,255,0));
      fill_gradient_RGB(leds,4*gradientLength, CRGB(0,255,0),5*gradientLength, CRGB(0,0,255));
      fill_gradient_RGB(leds,5*gradientLength, CRGB(0,0,255),6*gradientLength, CRGB(255,0,0));
    }
  
    if(useLookupMatrices) {
      for(int i = 0; i <= limit; i++) {
        SetPixel(leds[i], leds_b[i], b);
      }
    }
    else {
      for(int i = 0; i <= limit; i++) {
        leds[i].r = applyGamma_video(leds[i].r, fGammaR);
        leds[i].g = applyGamma_video(leds[i].g, fGammaG);
        leds[i].b = applyGamma_video(leds[i].b, fGammaB);
		SetBrightness(leds[i], leds_b[i], b);
      }
    }

    FixFloors(leds, numLEDs);
    FastLED.show();
  } while(ProcessSerialInput());
}

// Draws dimmed gradients: white, yellow, fusia
void GammaManager::RunDimmingTest(CRGB* leds, uint8_t* leds_b, uint16_t numLEDs) {
	uint8_t length = 64;
	if(numLEDs < length) { length = numLEDs; }
	float fadeStepSize = 255.0 / length;
	
	do {
		for(uint16_t i = 0; i < length; i++) {
			leds[i] = CRGB(b, b, b);
			if(useLookupMatrices) { SetBrightness(leds[i], leds_b[i], (i+1)*fadeStepSize); }
			else {
				uint16_t linear = fadeStepSize * (i+1);
				leds[i] %= 255 - (uint8_t)max(1, 255 - pow(linear / 255.0, fGammaDim) * 255 + 0.5);
				leds_b[i] = 255;
			}
			
			if(numLEDs >= 2*length+5) {
				leds[i+length+5] = CRGB(255,255,0);
				if(useLookupMatrices) { SetBrightness(leds[i+length+5], leds_b[i+length+5], (i+1)*fadeStepSize); }
				else {
					uint16_t linear = fadeStepSize * (i+1);
					leds[i+length+5] %= 255 - (uint8_t)max(1, 255 - pow(linear / 255.0, fGammaDim) * 255 + 0.5);
					leds_b[i+length+5] = 255;
				}
			}
			
			if(numLEDs >= 3*length+10) {
				leds[i+2*length+10] = CRGB(255,0,32);
				if(useLookupMatrices) { SetBrightness(leds[i+2*length+10], leds_b[i+2*length+10], (i+1)*fadeStepSize); }
				else {
					uint16_t linear = fadeStepSize * (i+1);
					leds[i+2*length+10] %= 255 - (uint8_t)max(1, 255 - pow(linear / 255.0, fGammaDim) * 255 + 0.5);
					leds_b[i+2*length+10] = 255;
				}
			}
		}
		
		FixFloors(leds, length);
		FastLED.show();
  } while(ProcessSerialInput());
}

// Handles serial IO while running RunTests()
bool GammaManager::ProcessSerialInput() {
  Serial.println("\nTo edit Gamma, enter: r,g,b,d(dimming), or a(all). 'w' to write matrices. 'u' to toggle matrix use");
  Serial.println("'c' for color correction. 't' for temperature. '###' sets brightness (1-255). 'n' for next pattern.");
  Serial.println("brightness +/- with '-', '='. Shift for *2");
  Serial.println("Brightness: " + String(b) + "\tColorCorrection: 0x" + String(colCorrection, HEX) + "\tTemperature: 0x" + String(temp, HEX));
  if(useLookupMatrices) { Serial.println("------------- Using matrices defined in your project -------------"); }
  else { Serial.println("Gammas: R:" + String(fGammaR) + "\tG:" + String(fGammaG) + "\tB:" + String(fGammaB) + "\tDim:" + String(fGammaDim)); }
  
  while(Serial.available() == 0) ;
  String s = Serial.readString();
  s.trim();
  
  if(s == "-") {
	  b--;
  }
  else if(s == "_") {
	  b -= 2;
  }
  else if(s == "=") {
	  b++;
  }
  else if(s == "+") {
	b += 2;
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
    fGammaDim = fGammaR;
  }
  else if(s == "d") {
    Serial.println("Enter new dimming gamma value. (Current is " + String(fGammaDim) + ")");
    while(Serial.available() == 0) ;
    s = Serial.readString();
    s.trim();
    fGammaDim = s.toFloat();
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
    WriteGammaMatrices(fGammaDim, 255, 255, "Dim", false);
    Serial.println();
  }
  else if(s == "c") {
    Serial.println("Enter new color correction as a 6-digit hex number");
    while(Serial.available() == 0) ;
    s = Serial.readString();
    s.trim();
    s.toUpperCase();

    colCorrection = 0;
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
    Init(colCorrection);
    FastLED.setCorrection(colCorrection);
  }
  else if(s == "t") {
    Serial.println("Enter new temperature as a 6-digit hex number");
    while(Serial.available() == 0) ;
    s = Serial.readString();
    s.trim();
    s.toUpperCase();

    temp = 0;
    for(int i=0; s[i]; i++){
      if(s[i] >= '0' && s[i] <= '9') {
          temp *= 0x10;
          temp += s[i] - '0';
      }
      else {
          temp *= 0x10;
          temp += s[i] - 'A' + 10;
      }
    }
    
    FastLED.setTemperature(temp);
  }
  else {
    b = s.toInt();
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

