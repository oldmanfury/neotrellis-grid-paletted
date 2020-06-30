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
int BRIGHTNESS = 127; // overall grid brightness - use gamma table below to adjust levels
int resetkeyspressed = 0;
boolean allthreekeys = false;

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
Adafruit_NeoTrellis trellis_array[NUM_ROWS / 4][NUM_COLS / 4] = {
  { Adafruit_NeoTrellis(0x33), Adafruit_NeoTrellis(0x31), Adafruit_NeoTrellis(0x2F), Adafruit_NeoTrellis(0x2E)}, // top row
  { Adafruit_NeoTrellis(0x35), Adafruit_NeoTrellis(0x39), Adafruit_NeoTrellis(0x3F), Adafruit_NeoTrellis(0x37) } // bottom row
};//0x39 mine is wrong - set to 0x3B
Adafruit_MultiTrellis trellis((Adafruit_NeoTrellis *)trellis_array, NUM_ROWS / 4, NUM_COLS / 4);

// gamma table for 16 levels of brightness
//int gammaTable[16] = { 0, 13,  15,  23,  32, 41, 52, 63, 76, 91, 108, 129, 153, 185, 230, 255}; 
int gammaTable[16] = {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};

const uint8_t allpalettes[9][3][16] = {
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
    if (isInited == false && x>0){
        selected_palette = y;
        isInited=true;
        for(int i=0; i<8; i++){
           colorpalettedisplay(y,i);
        }
        trellis.show();  
        sendLeds();
        delay(1000);
        for(int i=0; i<8; i++){
           colorpalettedisplay(8,i);
        }
        trellis.show();  
        sendLeds();      
      }else if (isInited == false && x==0){
        setGamma(y);
        for(int i=0; i<8; i++){
           colorpalettedisplay(i,i);
        }
        gammaselect();
        trellis.show();  
        sendLeds();       
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
  //isInited = true;
  // blink one led to show it's started up  
  trellis.setPixelColor(0, 0xFFFFFF);
  trellis.show();
  delay(100);
  trellis.setPixelColor(0, 0x000000);
  trellis.show();
  colorpaletteselector();
}

// ***************************************************************************
// **                                SEND LEDS                              **
// ***************************************************************************

void sendLeds(){
  uint8_t value, prevValue = 0;
  uint8_t Rvalue = 64, Gvalue = 0, Bvalue = 0;
  bool isDirty = false;

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
  }
}
// ***************************************************************************
// **                       color palette selector                          **
// ***************************************************************************
void colorpaletteselector(){
   for(int i=0; i<8; i++){
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
  int startval = 10.0;
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
