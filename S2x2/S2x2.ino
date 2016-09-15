#include <Wire.h>
#include <cRGB.h>
#include <WS2812.h>
#include <SPI.h>
#include "MFRC522.h"
#include "MifareOne.h"
#include "S2x2.h"

// MOSI	   MISO	   SCK	   SS (slave)	SS (master)	Levelf
// 11      12      13      10

/////////////////////////////////////////////////////////////////////
//set the pin
/////////////////////////////////////////////////////////////////////
MifareOne *Readers[4];

const int NRSTPD = 9;   //14=A0, 15=A1, 16=A2, 17=A3, 18=A4=SDA, 19=A5=SCL

#define NUM_READER    4


//- WS2812 RGB LED Line -------------------------------
const int cLEDCount = 4;
WS2812 LED(cLEDCount);   // 4 LED
cRGB RGB_Value;
byte ledSeq[] = {2, 1, 3, 0};

const int cRGB_LED = 4;

byte val = 127;

//--------------------------------------
uchar iii, tmp;
bool success;
uchar str[MAX_LEN];

byte moduleID = S2_GENERAL_ID;

struct GRID_INFO {
  byte CardId[5];
  byte Role;  
};

struct GRID_INFO  details[NUM_READER];

S2State moduleState = S2_STATE_GENERAL;
S2Response responseMode = S2_RESP_ACK;

//----------------------------------------------------------------------------------------------
void setup()
{
    Wire.begin(moduleID);
    Wire.onReceive(generalReceiveEvent);
    Wire.onRequest(generalRequestEvent);
    
    SPI.begin();
    Serial.begin(38400);
    
    LED.setOutput(cRGB_LED);    // Digital Pin 4

    pinMode(NRSTPD, OUTPUT);                      // Set digital pin 10 , Not Reset and Power-down
    digitalWrite(NRSTPD, LOW);
    //接 2,3,4,5 有時都只有4,5
    //38400 接 3,4,5,6 最正常

    delay(1000);
    digitalWrite(NRSTPD, HIGH);
    
    delay(50);
    Readers[0] = new MifareOne(8);
    Readers[1] = new MifareOne(14);
    Readers[2] = new MifareOne(10);
    Readers[3] = new MifareOne(15);
    
    for (iii = 0; iii < NUM_READER; iii++) {
      Readers[iii]->Init();
    }
    
    Serial.println("Ready !!!");
   
    delay(50);
        
    iii = 0;
}

bool polling = false;

//----------------------------------------------------------------------------------------------
void loop()
{
  if (polling)
    Polling(false);
}

void Polling(bool detail)
{
  byte c;
  byte data[16];
  
  for (int i = 0; i < NUM_READER; i ++) {
    c = 0;
    data[0] = 0;  // Clean up buffer.
    
    if (Readers[i]->changeState(MF_IDLE)) {
      if (Readers[i]->queryIdentifier(true)) {
        memcpy(details[i].CardId, Readers[i]->getCardId(), 5);
        
        // Just polling unique ID, without furthur data.
        if (!detail) continue;
        
        if (Readers[i]->changeState(MF_ACTIVE))
          if (Readers[i]->readBlock(4, data))
            if (data[0] == 'R') {
              c = data[1];
              Serial.print("Role ID : 0x");
              Serial.println(c, HEX);
              allLED(255, 255, 255);
              delay(2000);
              allLED(0, 0, 0);
            }
            else
              BlinkLED();
      }
      else
        memset(details[i].CardId, 0, 5);
    }
      
    details[i].Role = c;
    delay(100);
  }
}


//----------------------------------------------------------------------------------------------
void fRGB_LED(int vNo, byte vR, byte vG, byte vB)
{
  RGB_Value.r = vR; RGB_Value.g = vG; RGB_Value.b = vB;  // RGB Value -> Blue
  LED.set_crgb_at(ledSeq[vNo], RGB_Value);   // Set value at LED found at index 0
}

void allLED(byte vR, byte vG, byte vB)
{
  for (int i = 0; i < 4; i ++) {
    RGB_Value.r = vR; RGB_Value.g = vG; RGB_Value.b = vB;  // RGB Value -> Blue
    LED.set_crgb_at(i, RGB_Value);   // Set value at LED found at index 0
  }
  LED.sync();
}

//----------------------------------------------------------------------------------------------

void BlinkLED()
{
  allLED(255, 255, 255);
  delay(800);
  allLED(0, 0, 0);
  delay(1000);
  allLED(255, 255, 255);
  delay(500);
  allLED(0, 0, 0);
}

void boardReceiveEvent(int numBytes)
{
  char c;
  byte data[16];

  c = Wire.read();

  switch (c) {
    case S2_EXIT_GAME:
      Wire.end();
      moduleID = S2_GENERAL_ID;
      Wire.begin(moduleID);
      Wire.onReceive(generalReceiveEvent);
      Wire.onRequest(generalRequestEvent);
      Serial.println("Exit Game Mode.");
      polling = false;
      break;
    case S2_START_POLL:
      polling = true;
      Serial.println("Start Polling ...");
      break;
    case S2_STOP_POLL:
      polling = false;
      Serial.println("Stop Polling !!!");
      break;
    case S2_DETAILS:
      polling = false;
      Serial.println("Stop Polling !!!");

      responseMode = S2_RESP_DETAILS;
      Polling(true);
      break;
    case S2_LED_COLOR:
      Serial.print("Test LED - No:");
      Serial.println(numBytes);
      applyLEDs(numBytes - 1);
      break;
  }
}

void boardRequestEvent()
{
  switch (responseMode) {
    case S2_RESP_ACK:
      if (!polling) {
        Wire.write(0xAA);
        Wire.write(0x55);
        break;
      }
    case S2_RESP_DETAILS:
      for (int i = 0; i < NUM_READER; i ++) {
        Wire.write(details[i].CardId, 5);
        Wire.write(details[i].Role);
      }
      break;
  }

  responseMode = S2_RESP_ACK;
}

void generalReceiveEvent(int numBytes)
{
  char c;
  byte data[16];
  
  c = Wire.read();

  switch (c) {
    case S2_SETUP_ID:
      if (Readers[0]->changeState(MF_IDLE))
        if (Readers[0]->queryIdentifier(true))
          if (Readers[0]->changeState(MF_ACTIVE))
            if (Readers[0]->readBlock(4, data))
              if (data[0] == 'I') {
                moduleID = data[1];
                Serial.print("I2C Address : 0x");
                Serial.println(moduleID, HEX);
                allLED(255, 255, 255);
                delay(1000);
                allLED(0, 0, 0);
              }
              else
                BlinkLED();
      break;
    case S2_SETUP_GAME:
      if (moduleID == S2_GENERAL_ID) {
        // Notify user.
        break;
      }

      Wire.end();
      Wire.begin(moduleID);
      Wire.onReceive(boardReceiveEvent);
      Wire.onRequest(boardRequestEvent);
      Serial.println("Enter Game Mode.");
      break;
    case S2_START_POLL:
      polling = true;
      Serial.println("Start Polling ...");
      break;
    case S2_STOP_POLL:
      polling = false;
      Serial.println("Stop Polling !!!");
      break;
    case S2_LED_COLOR:
      Serial.print("Test LED - No:");
      Serial.println(numBytes);
      applyLEDs(numBytes - 1);
      break;
  }
}

void generalRequestEvent()
{
  if (!polling) return ;

  Serial.println("general Request Event ----------------- !!!");
  
  for (int i = 0; i < NUM_READER; i ++) {
    Wire.write(details[i].CardId, 5);
    Wire.write(details[i].Role);
  }
}

void applyLEDs(int numBytes)
{
  byte num = Wire.read();

  if (numBytes != (num * 4 + 1)) {
    Serial.println("Number of color data is incorrect !!!");
    return ;
  }

  byte i;
  byte r, g, b;
  while (num > 0) {
    i = Wire.read();
    r = Wire.read();
    g = Wire.read();
    b = Wire.read();
    Serial.print("No: ");
    Serial.print(i);
    Serial.print("  R: ");
    Serial.print(r);
    Serial.print("  G: ");
    Serial.print(g);
    Serial.print("  B: ");
    Serial.println(b);
    fRGB_LED(ledSeq[i], r, g, b);
    num --;
  }

  LED.sync();  
}

