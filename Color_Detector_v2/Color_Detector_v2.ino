// John Poirier
// Color Detector using RGB LED + LDR
// 12/12/25
//
// Requires colors.h generated with Color_List_Parser.py (lets you change/add/remove color naming and RGB values)


#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "colors.h" //672 colors!!!

// define pins
const int redPin  = 9;
const int greenPin = 10;
const int bluePin = 11;
const int buttonPin = 2;

// initialize LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// LED flash timings
const uint16_t SETTLE_RGB_MS = 30;
const uint16_t SETTLE_W_MS  = 30;

// calibration definitions (defaults)
uint16_t R_max = 800, G_max = 800, B_max = 800; // white reference (should be higher)
uint16_t R_min = 100, G_min = 100, B_min = 100; // black reference (should be lower)

// LED controls
void ledOff(){digitalWrite(redPin,HIGH); digitalWrite(greenPin,HIGH); digitalWrite(bluePin,HIGH);}
void ledR(){digitalWrite(redPin,LOW); digitalWrite(greenPin,HIGH); digitalWrite(bluePin,HIGH);}
void ledG(){digitalWrite(redPin,HIGH); digitalWrite(greenPin,LOW); digitalWrite(bluePin,HIGH);}
void ledB(){digitalWrite(redPin,HIGH); digitalWrite(greenPin,HIGH); digitalWrite(bluePin,LOW);}
void ledW(){digitalWrite(redPin,LOW); digitalWrite(greenPin,LOW); digitalWrite(bluePin,LOW);}

// sampling
uint16_t sampleWith(void (*fn)(), uint16_t settle){
 fn();
 delay(settle);
 uint16_t v = analogRead(A0);
 ledOff();
 return v;
}

// finds median of 3 values (noise stability)
uint16_t median3(uint16_t a, uint16_t b, uint16_t c){
 uint16_t t;
 if(a > b){ t = a; a = b; b = t; }
 if(b > c){ t = b; b = c; c = t; }
 if(a > b){ t = a; a = b; b = t; }
 return b;
}

// reads three times, finds median (noise stability)
uint16_t readMedian3(void (*fn)(), uint16_t settle){
 uint16_t a = sampleWith(fn, settle); delay(3);
 uint16_t b = sampleWith(fn, settle); delay(3);
 uint16_t c = sampleWith(fn, settle);
 return median3(a,b,c);
}

// read for medians
uint16_t readR(){return readMedian3(ledR, SETTLE_RGB_MS);}
uint16_t readG(){return readMedian3(ledG, SETTLE_RGB_MS);}
uint16_t readB(){return readMedian3(ledB, SETTLE_RGB_MS);}
uint16_t readW(){return readMedian3(ledW, SETTLE_W_MS);}

// button functions
bool waitForPress(){
 if(digitalRead(buttonPin)==LOW){
  while(digitalRead(buttonPin)==LOW);
  delay(20); //debouncing
  return true;
 }
 return false;
}

// double-press button
bool checkDoublePress(unsigned long timeout_ms = 300){
 unsigned long startTime = millis();
 if(waitForPress()){
  // got first press, wait for second
  while(millis() - startTime < timeout_ms){
   if(digitalRead(buttonPin)==LOW){
    while(digitalRead(buttonPin)==LOW); // wait for release
    delay(20);
    return true; // valid double press
   }
  }
 }
 return false;
}

// calibration mode
void calibrationMode(){
 lcd.clear();
 lcd.print("Calibration Mode");
 lcd.setCursor(0,1);
 lcd.print("Place on WHITE");
 Serial.println("\n--- Calibration Mode ---");
 Serial.println("Place on WHITE surface and press button.");

 while(!waitForPress());

 // scan white
 R_max = readR();
 G_max = readG();
 B_max = readB();

 lcd.clear();
 lcd.print("WHITE: R/G/B");
 char buf[16];
 snprintf(buf,16,"%4d %4d %4d",R_max,G_max,B_max); // show LDR readings (raw)
 lcd.setCursor(0,1);
 lcd.print(buf);
 Serial.print("WHITE (MAX): R:"); Serial.print(R_max);
 Serial.print(" G:"); Serial.print(G_max);
 Serial.print(" B:"); Serial.println(B_max);
 Serial.println("Place on BLACK surface and press button.");
 delay(1000); // display result for a bit, auto move on

 lcd.clear();
 lcd.print("Calibration Mode");
 lcd.setCursor(0,1);
 lcd.print("Place on BLACK");

 while(!waitForPress());

 // scan black
 R_min = readR();
 G_min = readG();
 B_min = readB();

 lcd.clear();
 lcd.print("BLACK: R/G/B");
 snprintf(buf,16,"%4d %4d %4d",R_min,G_min,B_min);
 lcd.setCursor(0,1);
 lcd.print(buf);
 Serial.print("BLACK (MIN): R:"); Serial.print(R_min);
 Serial.print(" G:"); Serial.print(G_min);
 Serial.print(" B:"); Serial.println(B_min);
 delay(1000); // show result, auto move on

 // check for reversed min/max (in case you calibrate wrong, won't be any errors (still won't work right though, need to recalibrate))
 if (R_max <= R_min) R_max = R_min + 1;
 if (G_max <= G_min) G_max = G_min + 1;
 if (B_max <= B_min) B_max = B_min + 1;

 lcd.clear();
 lcd.print("Calibration Done");
 delay(1000);
 lcd.clear();
 lcd.print("Ready");
 Serial.println("Calibration Done");
}

// progressive nearest color search
// start with a small search tolerance, then repeatedly search, increasing the tolerance, until a match is found
const uint8_t INITIAL_RANGE = 5; //initial search range (within _ of the defined color RGB value)
const uint8_t RANGE_INCREMENT = 5;
const uint8_t MAX_RANGE = 50; //max search range

const char* nearestColorProgressive(uint8_t R8, uint8_t G8, uint8_t B8){
 static char bestName[32];

 for(uint8_t range = INITIAL_RANGE; range <= MAX_RANGE; range += RANGE_INCREMENT){
  int bestDist = 1000;
  int found = 0;
  uint16_t bestIdx = 0;

  for(uint16_t i=0; i<NUM_COLORS; i++){
   Color c;
   memcpy_P(&c, &colors[i], sizeof(Color));

  // skip colors outside current search range
   if(abs((int)R8 - c.r) > range) continue;
   if(abs((int)G8 - c.g) > range) continue;
   if(abs((int)B8 - c.b) > range) continue;

  // find manhattan distance, log closest match
   int dist = abs((int)R8 - c.r) + abs((int)G8 - c.g) + abs((int)B8 - c.b);
   if(dist < bestDist){
    bestDist = dist;
    bestIdx = i;
    found = 1;
   }
  }

  // if a match is found, give the name
  if(found){
   const char* ptr = (const char*)pgm_read_ptr(&colors[bestIdx].name);
   strcpy_P(bestName, ptr);
   return bestName;
  }
 }
 // if match not found, return unknown (super unlikely to happen with 627 colors)
 strcpy(bestName, "unknown");
 return bestName;
}


// setup things
void setup(){
 pinMode(redPin,OUTPUT);
 pinMode(greenPin,OUTPUT);
 pinMode(bluePin,OUTPUT);
 pinMode(buttonPin,INPUT_PULLUP);

 lcd.init();
 lcd.backlight();

 Serial.begin(115200);
 lcd.print("Initializing...");
 Serial.println("Initializing.");

 // calibrate on startup
 calibrationMode();
 Serial.println("Ready.");
}

// main
void loop(){

 // check for double click
 if(checkDoublePress(300)){
  calibrationMode();
  return;
 }

 if(!waitForPress()) return;

 // read raw reflectances
 uint16_t Rraw = readR();
 uint16_t Graw = readG();
 uint16_t Braw = readB();

 // red - scale max, min LDR values to 0-255 for RGB vals
 long R_range = (long)R_max - R_min;
 uint8_t R8 = constrain(255L * ((long)Rraw - R_min) / R_range, 0, 255);

 // green - scale max, min LDR values to 0-255 for RGB vals
 long G_range = (long)G_max - G_min;
 uint8_t G8 = constrain(255L * ((long)Graw - G_min) / G_range, 0, 255);

 // blue - scale max, min LDR values to 0-255 for RGB vals
 long B_range = (long)B_max - B_min;
 uint8_t B8 = constrain(255L * ((long)Braw - B_min) / B_range, 0, 255);

 // find nearest color name
 const char* cname = nearestColorProgressive(R8, G8, B8);

 // show off the color RGB vals and the name found
 lcd.clear();
 char buf[16];
 snprintf(buf,16,"R%3d G%3d B%3d",R8,G8,B8);
 lcd.print(buf);

 lcd.setCursor(0,1);
 lcd.print(cname);

 // serial debug outputs
 Serial.print("Raw: R"); Serial.print(Rraw); Serial.print(" G"); Serial.print(Graw); Serial.print(" B"); Serial.print(Braw);
 Serial.print(" Scaled: R"); Serial.print(R8); Serial.print(" G"); Serial.print(G8); Serial.print(" B"); Serial.print(B8);
 Serial.print(" Closest: ");
 Serial.println(cname);
}