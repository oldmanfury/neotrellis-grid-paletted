/* 
 *  NeoTrellis Grid
 *  
 *  Many thanks to scanner_darkly, Szymon Kaliski, John Park, todbot, Juanma and others
 *
*/
#include "MonomeSerialDevice.h"
#include <Adafruit_NeoTrellis.h>
#include "debug.h"

// IF USING ADAFRUIT M0 or M4 BOARD
#define M0 0
//#include <Arduino.h>
//#include <Adafruit_TinyUSB.h>
//#include <elapsedMillis.h>


#define NUM_ROWS 8 // DIM_Y number of rows of keys down
#define NUM_COLS 16 // DIM_X number of columns of keys across
#define INT_PIN 9
#define LED_PIN 13 // teensy LED 

uint32_t hexColor;

// This assumes you are using a USB breakout board to route power to the board 
// If you are plugging directly into the teensy, you will need to adjust this brightness to a much lower value
int BRIGHTNESS = 255; // overall grid brightness - use gamma table below to adjust levels
int resetkeyspressed = 0;
int page = 0;
boolean allthreekeys = false;
boolean isDirty = false;

uint8_t R;
uint8_t G;
uint8_t B;

// set your monome device name here
String deviceID = "neo-monome";

// DEVICE INFO FOR ADAFRUIT M0 or M4
char mfgstr[32] = "monome";
char prodstr[32] = "monome";

bool isInited = false;
int selected_palette = 0;
elapsedMillis monomeRefresh;


// Monome class setup
MonomeSerialDevice mdp;

int prevLedBuffer[mdp.MAXLEDCOUNT]; 


// NeoTrellis setup
// 16x8 
Adafruit_NeoTrellis trellis_array[NUM_ROWS / 4][NUM_COLS / 4] = {
  { Adafruit_NeoTrellis(0x32), Adafruit_NeoTrellis(0x30), Adafruit_NeoTrellis(0x2F), Adafruit_NeoTrellis(0x2E)}, // top row
  { Adafruit_NeoTrellis(0x33), Adafruit_NeoTrellis(0x31), Adafruit_NeoTrellis(0x3E), Adafruit_NeoTrellis(0x36)} // bottom row
};
Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)trellis_array, NUM_ROWS / 4, NUM_COLS / 4);

// gamma table for 16 levels of brightness
//int gammaTable[16] = { 0, 13,  15,  23,  32, 41, 52, 63, 76, 91, 108, 129, 153, 185, 230, 255}; 
int gammaTable[16] = {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};

const uint8_t allpalettes[25][3][16] = {
  {//BuPu
     {75, 98, 123, 140, 157, 167, 177, 188, 198, 208, 218, 224, 230, 237, 244, 248},
     {0, 7, 15, 39, 64, 86, 107, 129, 150, 169, 188, 200, 211, 224, 236, 244},
     {77, 101, 128, 132, 136, 138, 139, 140, 140, 149, 158, 175, 191, 209, 224, 237}
  },
  {//accent
     {127, 152, 181, 208, 236, 253, 254, 240, 152, 69, 127, 206, 227, 204, 177, 136},
     {201, 189, 178, 179, 187, 204, 231, 244, 179, 117, 67, 20, 25, 65, 92, 97},
     {127, 161, 200, 188, 154, 137, 145, 154, 164, 174, 156, 135, 99, 52, 34, 70}
  },
  {//Ametrine
     {30, 58, 87, 115, 144, 172, 187, 197, 206, 216, 225, 231, 229, 227, 225, 223},
     {60, 66, 71, 77, 83, 88, 90, 89, 88, 87, 86, 95, 120, 146, 172, 197},
     {151, 152, 153, 154, 154, 155, 144, 127, 110, 93, 76, 60, 48, 36, 23, 11}
  },
  {//winter
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
     {0, 14, 30, 47, 62, 79, 96, 111, 127, 143, 159, 175, 191, 207, 223, 240},
     {255, 247, 239, 231, 223, 215, 207, 199, 190, 183, 175, 167, 159, 150, 143, 135}
  },
  {//summer
     {0, 14, 30, 47, 62, 79, 96, 111, 127, 143, 159, 175, 191, 207, 223, 240},
     {127, 135, 143, 150, 159, 167, 175, 183, 191, 199, 207, 215, 223, 231, 239, 247},
     {102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102}
  },
  {//gist_heat
     {0, 21, 43, 66, 90, 114, 136, 159, 184, 206, 228, 251, 255, 255, 255, 255},
     {0, 0, 0, 0, 0, 0, 0, 0, 13, 42, 72, 101, 133, 164, 192, 223},
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 67, 125, 191}
  },
  {//autumn
     {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
     {0, 14, 30, 47, 62, 79, 96, 111, 127, 143, 159, 175, 191, 207, 223, 240},
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  },
  {//spectral
     {158, 189, 219, 239, 248, 253, 253, 254, 253, 239, 213, 178, 134, 93, 61, 69},
     {1, 35, 72, 102, 140, 180, 211, 235, 254, 248, 238, 223, 206, 184, 148, 110},
     {66, 73, 76, 68, 81, 102, 128, 159, 188, 166, 155, 162, 164, 168, 183, 176}
  },
  {//Set1
     {228, 148, 58, 65, 76, 115, 152, 204, 254, 255, 253, 209, 168, 209, 244, 195},
     {26, 72, 124, 149, 173, 126, 79, 102, 130, 190, 251, 169, 87, 108, 129, 141},
     {28, 100, 179, 130, 76, 118, 159, 80, 3, 25, 50, 45, 44, 120, 189, 169}
  },
  {//gist_heat
     {0, 21, 43, 66, 90, 114, 136, 159, 184, 206, 228, 251, 255, 255, 255, 255},
     {0, 0, 0, 0, 0, 0, 0, 0, 13, 42, 72, 101, 133, 164, 192, 223},
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 67, 125, 191}
  },
  {//dark2
     {27, 103, 190, 187, 143, 138, 188, 221, 164, 110, 151, 206, 212, 184, 156, 127},
     {158, 132, 103, 99, 107, 98, 67, 49, 105, 158, 167, 170, 157, 132, 115, 108},
     {119, 71, 18, 54, 132, 170, 153, 129, 82, 37, 19, 7, 9, 20, 39, 72}
  },
  {//ExtendedKindlmann256
     {0, 35, 44, 5, 3, 4, 5, 51, 120, 210, 247, 249, 250, 228, 233, 236},
     {0, 2, 4, 42, 67, 86, 103, 118, 124, 105, 105, 130, 151, 189, 213, 238},
     {0, 58, 103, 110, 67, 40, 13, 5, 6, 10, 76, 170, 244, 252, 253, 255}
  },
  {//gist_earth
     {0, 5, 15, 27, 40, 51, 58, 66, 92, 125, 152, 178, 188, 201, 219, 237},
     {0, 17, 55, 87, 114, 132, 141, 150, 160, 169, 175, 182, 170, 166, 182, 212},
     {0, 117, 120, 123, 126, 118, 99, 79, 75, 83, 88, 93, 98, 119, 159, 206}
  },
  {//hot
     {10, 48, 92, 134, 176, 219, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
     {0, 0, 0, 0, 0, 0, 6, 49, 92, 131, 175, 216, 255, 255, 255, 255},
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 70, 132, 198}
  },
  {//linearGray4-bluegreen
     {99, 105, 109, 112, 117, 124, 128, 129, 125, 127, 136, 145, 154, 163, 179, 207},
     {72, 78, 87, 95, 110, 124, 139, 152, 164, 177, 191, 199, 208, 212, 213, 226},
     {73, 73, 74, 75, 77, 82, 92, 103, 114, 127, 151, 173, 195, 214, 227, 247}
  },
  {//isolum
     {81, 103, 122, 139, 155, 173, 188, 203, 218, 232, 245, 251, 243, 234, 226, 217},
     {172, 167, 161, 156, 151, 150, 149, 148, 145, 138, 129, 131, 149, 166, 181, 196},
     {221, 214, 207, 199, 190, 176, 160, 143, 125, 110, 94, 79, 70, 59, 47, 31}
  },
{//inferno
  {0,16,29,67,107,117,163,183,231,245,251,251,251,243,242,244},
  {0,20,12,10,23,27,43,55,95,124,157,179,185,224,243,247},
  {0,112,69,104,110,109,97,83,44,21,6,24,30,86,129,141}
},

{//viridis
  {0,68, 72, 67, 57, 48, 42, 39, 34, 30, 50, 83, 116,167,218,248},
  {0,3,33, 59, 84, 104, 117, 126,137, 156, 181, 197, 208, 219, 226, 230},
  {0,87, 114, 131, 139, 141, 148, 142, 141, 137, 122, 103, 84,  51,  24,  33}
},
{//terrain
  {0,44, 24, 1,0,65, 129,197,254,228,198,168,131,163,196,237},
  {0,64 ,104,149,192,217,229,243,253,220,182,143,96, 137,180,231},
  {0,166,206,251,136,115,127,141,152,138,122,106,88 ,131,177,230}
},
{//ocean
  {0,0,0,0,0,0,0,0,0,0,0,20, 96, 137,197,246},
  {0,124,82,99, 63, 26, 7,24, 48, 75, 116,137,175,196,226,250},
  {0,2,30,19, 43, 67, 89, 101,117,135,163,177,202,216,236,252}
},
{//cubehelix
  {0,35, 26, 22, 23, 49, 113,161,194,209,210,202,194,194,212,242},
  {0,21, 29, 55, 89, 114,122,121,121,128,149,169,194,216,239,251},
  {0,56, 59, 76, 73, 54, 50, 74, 114,156,207,230,242,242,238,246}
},
{//cividis
  {0,0,0,31, 62, 85, 101,117,134,154,175,190,206,229,239,253},
  {0,37, 47, 58, 75, 91, 104,117,131,147,163,176,188,207,216,231},
  {0,84, 109,110,107,109,112,117,120,118,112,106,98, 80, 70, 55}
},
{//gnuplot
  {0,57, 84, 102,121,136,148,158,168,179,196,211,225,234,250,255},
  {0,0,0,1,3,5,9,14, 21, 30, 54, 82, 121,154,228,249},
  {0,80, 162,215,252,248,217,164,100,9,0,0,0,0,0,0}
},
{//white
  {20,35,50,65,80,95,110,125,140,155,170,185,200,215,230,255},
  {20,35,50,65,80,95,110,125,140,155,170,185,200,215,230,255},
  {20,35,50,65,80,95,110,125,140,155,170,185,200,215,230,255}
},
{//blank
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}
};


// ***************************************************************************
// **                          FUNCTIONS FOR TRELLIS                        **   
// ***************************************************************************


//define a callback for key presses
TrellisCallback keyCallback(keyEvent evt){
  uint8_t x;
  uint8_t y;
  x  = evt.bit.NUM % NUM_COLS; //NUM_COLS; 
  y = evt.bit.NUM / NUM_COLS; //NUM_COLS; 
  if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING){
    //on
    mdp.sendGridKey(x, y, 1);
//------------color palette initial selection-------------------
    if ((x==13 && y==7) or (x==14 && y==7)or (x==15 && y==7)){
      resetkeyspressed += 1;
      if (resetkeyspressed == 3){
         allthreekeys = true;
      }
    }
    if (isInited == false && x>0){//-------select-the--palette---------
        selected_palette = y+8*page;
        isInited=true;
        for(int i=0; i<8; i++){
           colorpalettedisplay(selected_palette,i);
        }
        trellis.show();  
  //      sendLeds();
        delay(1000);
        for(int i=0; i<8; i++){
           colorpalettedisplay(24,i);//-clears-display-----
        }
        trellis.show();
        isDirty = true;  
        sendLeds();
        }
      else if (isInited == false && x==0){//------select-a-brightness-level-------
        if (y<7){
          setGamma(y);
          for(int i=0; i<8; i++){
            colorpalettedisplay(i,i);
          }
        }else if (y==7){
           page += 1;
           page = page % 3;  
           for(int i=0; i<8; i++){
             colorpalettedisplay(i+8*page,i);
           }       
        }
        gammaselect();
        trellis.show();  
  //      sendLeds();       
      }
//--------------------------------------------------------------
  }else if(evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING){
    //off 
    if ((x==13 && y==7) or (x==14 && y==7)or (x==15 && y==7)){
      resetkeyspressed -= 1;
    }
    if (allthreekeys && resetkeyspressed == 0){
         colorpaletteselector();
         isInited = false;
         delay(2000);
         allthreekeys = false;
    }
    mdp.sendGridKey(x, y, 0); 
  }
  return 0;
  
}

// ***************************************************************************
// **                                 SETUP                                 **
// ***************************************************************************

void setup(){
/*
// for Adafruit M0 or M4
  USBDevice.setManufacturerDescriptor(mfgstr);
  USBDevice.setProductDescriptor(prodstr);
*/
  Serial.begin(115200);
  R = 255;
  G = 255;
  B = 255;
  mdp.isMonome = true;
  mdp.deviceID = deviceID;
  mdp.setupAsGrid(NUM_ROWS, NUM_COLS);

//  delay(200);

  trellis.begin();
  
/*  if(!trellis.begin()){
    Serial.println("failed to begin trellis");
    while(1);
  }
*/

  // key callback
  uint8_t x, y;
  for (x = 0; x < NUM_COLS; x++) {
    for (y = 0; y < NUM_ROWS; y++) {
      trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_RISING, true);
      trellis.activateKey(x, y, SEESAW_KEYPAD_EDGE_FALLING, true);
      trellis.registerCallback(x, y, keyCallback);
    }
  }
  setBright();
  setGamma(2);
  delay(300);
  mdp.setAllLEDs(0);
  sendLeds();
  monomeRefresh = 0;
  colorpaletteselector();
}

// ***************************************************************************
// **                                SEND LEDS                              **
// ***************************************************************************

void sendLeds(){
  uint8_t value, prevValue = 0;
  uint8_t Rvalue = 64, Gvalue = 0, Bvalue = 0;
//  bool isDirty = false;
//  isDirty = false;
  for(int i=0; i< NUM_ROWS * NUM_COLS; i++){ 
    value = mdp.leds[i];
    prevValue = prevLedBuffer[i];
      Rvalue = allpalettes[selected_palette][0][value]*gammaTable[value]/255; 
      Gvalue = allpalettes[selected_palette][1][value]*gammaTable[value]/255; 
      Bvalue = allpalettes[selected_palette][2][value]*gammaTable[value]/255;    
    if (value != prevValue) {
      hexColor =  ((Rvalue << 16) + (Gvalue << 8) + (Bvalue << 0));
      trellis.setPixelColor(i, hexColor);
      prevLedBuffer[i] = value;
      isDirty = true;
    }
  }
 
  if (isDirty) {
    trellis.show();
    isDirty = false;
  }
}
// ***************************************************************************
// **                       color palette selector                          **
// ***************************************************************************
void colorpaletteselector(){
   for(int i=page*8; i<8*(1+page); i++){
   colorpalettedisplay(i,i);
   } 
   gammaselect();
   trellis.show();  
}
void colorpalettedisplay(int selected, int row){
  for(int i=0; i<NUM_COLS; i++){
    uint8_t gamvalue = gammaTable[i];    
    uint8_t Rvalue = allpalettes[selected][0][i];    
    uint8_t Gvalue = allpalettes[selected][1][i];    
    uint8_t Bvalue = allpalettes[selected][2][i];    
        trellis.setPixelColor((i+row*16),((Rvalue*gamvalue/255 << 16) + (Gvalue*gamvalue/255 << 8) + (Bvalue*gamvalue/255 << 0))); //addressed with keynum
         delay(0);
  } 
}

void gammaselect(){// leftmost column for brightness selection
  for(int i=0; i<NUM_ROWS; i++){
    int startval = 0;
    uint8_t Rvalue = startval + ((7.0-i)/7.0)*(255-startval); 
    uint8_t Gvalue = startval + ((7.0-i)/7.0)*(255-startval);
    uint8_t Bvalue = startval + ((7.0-i)/7.0)*(255-startval);
        trellis.setPixelColor((i*16),((Rvalue << 16) + (Gvalue << 8) + (Bvalue << 0))); //addressed with keynum
         delay(0);
  } 
}

// ***************************************************************************
// **                    SET OVERALL BRIGHTNESS                             **
// ***************************************************************************
void setBright(){
  // set overall brightness for all pixels
  uint8_t x, y;
  for (x = 0; x < NUM_COLS / 4; x++) {
    for (y = 0; y < NUM_ROWS / 4; y++) {
      trellis_array[y][x].pixels.setBrightness(BRIGHTNESS);
    }
  }
}
// ***************************************************************************
// **                         SET GAMMA TABLE                               **
// ***************************************************************************
void setGamma(float value){
  // set overall brightness for all pixels
  int i;
  int startval = 0.0;
  for (i = 0; i < 16; i++) {
    gammaTable[i] = startval + (((8.0-value)/8.0)*(255.0-startval)/15.0)*i;  
    }
  }
// ***************************************************************************
// **                                 LOOP                                  **
// ***************************************************************************

void loop() {

    mdp.poll(); // process incoming serial from Monomes
 
    // refresh every 16ms or so
    if (isInited && monomeRefresh > 16) {
        trellis.read();
        sendLeds();
        monomeRefresh = 0;
    }
    else if (isInited==false){
        trellis.read();
 //       sendLeds();
    }
}
