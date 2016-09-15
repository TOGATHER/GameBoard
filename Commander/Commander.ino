#include <Wire.h>
#include <SPI.h>
#include "MFRC522.h"
#include "MifareOne.h"

#include "S2x2.h"

MifareOne Reader(8);
const int NRSTPD = 9;   //14=A0, 15=A1, 16=A2, 17=A3, 18=A4=SDA, 19=A5=SCL

//----------------------------------------------------------------------------------------------
void setup()
{
  Wire.begin();
  SPI.begin();
  Serial.begin(38400);

  // Trigger reset signal.
  pinMode(NRSTPD, OUTPUT);
  digitalWrite(NRSTPD, LOW);

  delay(1000);
  digitalWrite(NRSTPD, HIGH);
    
  delay(50);
    
  // Set digital pin 10 as OUTPUT to connect it to the RFID /ENABLE pin
  Serial.println("Reset Card Reader.");

  Reader.Init();
  
  Serial.println("Ready !!!");
  
  delay(50);
  DisplayUsage();

  Serial.setTimeout(5000);
}

void DisplayUsage()
{
  Serial.print("\n\n");
  Serial.println("(i) Read card ID.");
  Serial.println("(r) Read block and display.");
  Serial.println("(a) Set passcode A.");
  Serial.println("(b) Set passcode B.\n");
  
  Serial.println("(x) Reset\n");
  
  Serial.println("(1) Idle/Halt  -> Ready");
  Serial.println("(2) Ready      -> Active");
  Serial.println("(3) Active     -> Halt");
  Serial.println("(4) Issue card\n");
  Serial.println("(6) Test S2x2 module.");
  Serial.println(": ");
}

//----------------------------------------------------------------------------------------------
int cmd;
byte color[12] = {255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255};

void loop()
{
  while (Serial.available()) {
    cmd = Serial.read();

    switch (toLowerCase(cmd)) {
      case '1':
        Reader.changeState(MF_READY);
        break;
      case '2':
        Reader.changeState(MF_ACTIVE);
        break;
      case '3':
        Reader.changeState(MF_HALT);
        break;
      case '4':
        IssueCard();
        break;
      case '6':
        TestS2x2Module();
        break;
      case 'i':
        Reader.queryIdentifier(true);
        break;
      case 'r': {
        Serial.print("Block No.? ");
        
        long blockNo;
        /*
        do {
          Serial.print("Serial Buffer: ");
          Serial.println(Serial.available());
          
          while (Serial.available() == 0) {
            delay(100);
          }
          
          blockNo = Serial.parseInt();
          Serial.println(blockNo);
          if (blockNo < 0 || blockNo > 63)
            Serial.println("Block No. must be within 0 to 63.");
        } while (blockNo < 0 || blockNo > 63);
*/
        Reader.readBlock(4);
        break;
      }
      case 'a':
        break;
      case 'b':
        break;
      case 'x':
        Reader.changeState(MF_IDLE);
        break;
      default:
        DisplayUsage();
        break;
    }
  }
}

byte sideColor = 0x00;

void DisplayIssueCommand()
{
  Serial.println("(1) King");   // KI
  Serial.println("(2) Queen");  // QU
  Serial.println("(3) Knight"); // KN
  Serial.println("(4) Bishop"); // BI
  Serial.println("(5) Rook");   // RK
  Serial.println("(6) Pawn");   // PW
  Serial.println("(7) ID Card");
  // Which side?
  Serial.print("(x) ");
  Serial.print(sideColor ? "White" : "Black");
  Serial.println(" side");
  Serial.println("(0) Exit.");
}


uchar issueData[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void IssueCard()
{
  int c;
  byte id;
  
  DisplayIssueCommand();  

  while (1) {
    if (Serial.available()) {
      c = Serial.read();
      
      switch (c) {
        case '1': // King
          issueData[0] = 'R';
          issueData[1] = sideColor | 0x0A;
          break;
        case '2': // Queen
          issueData[0] = 'R';
          issueData[1] = sideColor | 0x0B;
          break;
        case '3': // Knight
          issueData[0] = 'R';
          issueData[1] = sideColor | 0x0C;
          break;
        case '4': // Bishop
          issueData[0] = 'R';
          issueData[1] = sideColor | 0x0D;
          break;
        case '5': // Rook
          issueData[0] = 'R';
          issueData[1] = sideColor | 0x0E;
          break;
        case '6': // Pawn
          issueData[0] = 'R';
          issueData[1] = sideColor | 0x0F;
          break;
        case '7':
          Serial.println("Enter ID: ");
          id = Serial.parseInt();
          issueData[0] = 'I';
          issueData[1] = id;
          break;
        case 'x':
          sideColor ^= 0x10;
          DisplayIssueCommand();
          break;
        case '0': // [exit]
          return ;
      }

      if (c >= '1' && c <= '7') {
        Reader.writeBlock(4, issueData);
        DisplayIssueCommand();
      }
    }
  }
}

void DisplayS2x2Command()
{
  Serial.println("(1) Setup I2C address.");
  Serial.println("(2) Enter board model.");
  Serial.println("(3) Check board.");
  Serial.println("(s) Start Polling");
  Serial.println("(e) Stop Polling");
  Serial.println("(d) Detail Information");
  Serial.println("(p) Show polling data");
  Serial.println("(l) Test LED");
  Serial.println("(0) Exit.");
}

byte LEDTestData[] = {0, 255, 255, 255,
                      0, 255, 0, 0,
                      0, 0, 255, 0,
                      0, 0, 0, 255};
                      
void TestS2x2Module()
{
  int c;
  int moduleId;
  
  DisplayS2x2Command();
  
  while (1) {
    if (Serial.available()) {
      c = Serial.read();
      
      switch (c) {
        case '1':
          Wire.beginTransmission(S2_GENERAL_ID);
          Wire.write(S2_SETUP_ID);
          Wire.endTransmission();
          break;
        case '2':
          Wire.beginTransmission(S2_GENERAL_ID);
          Wire.write(S2_SETUP_GAME);
          Wire.endTransmission();
          break;
        case '3':
          moduleId = 0x10;
          
          for (int i = 0; i < 4; i ++) {
            Wire.beginTransmission(moduleId);
            Wire.write(S2_IS_ALIVE);
            Wire.endTransmission();
            
            Wire.requestFrom(moduleId, 2, true);
            delay(500);
            Serial.print(Wire.read(), HEX);
            Serial.println(Wire.read(), HEX);

            moduleId ++;
          }
        case 's':
          Wire.beginTransmission(S2_GENERAL_ID);
          Wire.write(S2_START_POLL);
          Wire.endTransmission();
          break;
        case 'e':
          Wire.beginTransmission(S2_GENERAL_ID);
          Wire.write(S2_STOP_POLL);
          Wire.endTransmission();
          break;
        case 'd':
          getS2ModulePoll(true);
          break;
        case 'l':
          for (int i = 0; i < 4; i ++) {
            LEDTestData[0]  = i;
            LEDTestData[4]  = (i + 1) % 4;
            LEDTestData[8]  = (i + 2) % 4;
            LEDTestData[12] = (i + 3) % 4;
            Wire.beginTransmission(S2_GENERAL_ID);
            Wire.write(S2_LED_COLOR);
            Wire.write(4);
            Wire.write(LEDTestData, sizeof(LEDTestData));
            Wire.endTransmission();
            delay(1000);
          }
          break;
        case 'p':
          getS2ModulePoll(false);
          break;
        case '0':
          return ;
          break;
      }
    }
  }
}

void getS2ModulePoll(bool detail)
{
  byte details[24], c;

  if (!detail) {
    Wire.beginTransmission(S2_GENERAL_ID);
    Wire.write(S2_DETAILS);
    Wire.endTransmission();
  }
  
  Wire.requestFrom(S2_GENERAL_ID, 24);

  // Waiting for response.
  while (!Wire.available()) delay(10);
  
  for (int i = 0; i < 24; i ++) {
    if ((i % 6) == 0) Serial.print("\n");
    
    c = Wire.read();
    PrintHex(c);
    details[i] = c;
  }

  Serial.println("");
}

