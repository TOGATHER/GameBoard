#include "MFRC522.h"

#define MF_DELAY    10

void PrintHex(byte b, bool prefix = false, char suffix = 0);
bool ValidateHex(uchar *str);


byte MifareOne::sTransferMap[4][4] = {
                  /*  TO:   IDLE  , READY , ACTIVE, HALT  */
/* FROM IDLE */           { 4     , 1     , 0     , 0},
/* FROM READY */          { 4     , 0     , 2     , 0},
/* FROM ACTIVE */         { 4     , 0     , 0     , 3},
/* FROM HALT */           { 4     , 1     , 0     , 0}
// 1 => PICC_REQALL
// 2 => Select
// 3 => Halt
// 4 => Reset
};

byte MifareOne::sBuffer[50] = {0};
byte MifareOne::sStatus = 0;

MifareOne::MifareOne(byte chipSelect)
{
  pEnable = chipSelect;
  
  pinMode(pEnable, OUTPUT);
  digitalWrite(pEnable, HIGH);
}

void MifareOne::Init()
{
  pState = MF_IDLE;
  Reset();
}

bool MifareOne::changeState(MFState state)
{
  if (pState == state) return true;

  byte s = sTransferMap[pState][state];
  bool success = false;
  
  switch (s) {
    // Invalidated transition.
    default:
    case 0:
      return false;
      
    case 1:
      success = RequestAll(true);
      break;
    // Select
    case 2:
      success = SelectCard();
      break;
    // Halt
    case 3:
      success = true;
      Halt();
      break;
    // Reset
    case 4:
      success = true;
      Reset();
      break;
  }

  if (success) pState = state;

  return success;
}

// In Ready mode.
bool MifareOne::queryIdentifier(bool verbose)
{
  MFRC522_SelectChip(pEnable);
  if (changeState(MF_READY) == false) return false;

  delay(MF_DELAY);
  
  sStatus = MFRC522_Anticoll(sBuffer);
  if (sStatus != MI_OK) {
    Serial.println("Failed to read card identifier !!!");
    return false;
  }
  memcpy(pCardId, sBuffer, 5);

  if (verbose) {
    Serial.print("\nCard ID => 0x");

    for (int i = 0; i < 4; i ++) {
      PrintHex(pCardId[i]); 
    }
    Serial.print(" - "); PrintHex(pCardId[4], false, '\n');
  }

  return true;
}

// In Active mode.
bool MifareOne::readBlock(uchar blockNo, byte *data)
{
  MFRC522_SelectChip(pEnable);

  delay(MF_DELAY);
  sStatus = MFRC522_Auth(PICC_AUTHENT1A, (blockNo & 0xFC) + 3, keyA, pCardId);
  if (sStatus != MI_OK) {
    Serial.println("Failed to get authentication !!!");
    return false;
  }

  delay(MF_DELAY);
  sStatus = MFRC522_Read(blockNo, sBuffer);
  if (sStatus != MI_OK) {
    Serial.println("Failed to read block data !!!");
    return false;
  }

  for (int i = 0; i < 16; i ++) {
    PrintHex(sBuffer[i], false, ' ');
    data[i] = sBuffer[i];
    if (i == 7) Serial.print("\n");
  }

  return true;
}

// In Active mode.
void MifareOne::writeBlock(uchar blockNo, uchar* data)
{
  MFRC522_SelectChip(pEnable);

  delay(MF_DELAY);
  sStatus = MFRC522_Auth(PICC_AUTHENT1A, (blockNo & 0xFC) + 3, keyA, pCardId);
  if (sStatus != MI_OK) {
    Serial.println("Failed to get authentication !!!");
    return ;
  }

  delay(MF_DELAY);
  sStatus = MFRC522_Write(blockNo, data);
  if (sStatus != MI_OK) {
    Serial.println("Failed to write block data !!!");
    return ;
  }
}

uchar* MifareOne::getCardId()
{
  return pCardId;
}


/*
 * HEX utilities.
 */
void PrintHex(byte b, bool prefix, char suffix)
{
  if (prefix)
    Serial.print("0x");
    
  if (b < 0x10)
    Serial.print(0);

  Serial.print(b, HEX);

  if (suffix)
    Serial.print(suffix);
}

bool ValidateHex(uchar *str)
{
  uchar c = *str;
  
  while (c) {
    
    if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F'))
      return false;
    
    c = *(++ str);
  }

  return true;
}

// Idle -> Ready mode.
bool MifareOne::RequestAll(bool verbose)
{
  MFRC522_SelectChip(pEnable);

  delay(MF_DELAY);
  sStatus = MFRC522_Request(PICC_REQALL, sBuffer);
  
  if (sStatus != MI_OK) {
    Serial.println("No card available !!!");
    return false;
  }

  if (verbose) {
    PrintHex(sBuffer[0], true);
    PrintHex(sBuffer[1], false, '-');
    
    switch (sBuffer[0]) {
      case 0x04:
        Serial.println("Mifare One (S50)");
        break;
      case 0x02:
        Serial.println("Mifare One (S70)");
        break;
      case 0x08:
        Serial.println("Mifare Pro (X)");
        break;
      case 0x44:
        Serial.println("Mifare_UltraLight / Mifare_DESFire");
        break;
      default:
        Serial.println("Unknown card type");
        break;
    }
  }

  return true;
}

// Ready -> Active mode.
bool MifareOne::SelectCard()
{
  MFRC522_SelectChip(pEnable);

  delay(MF_DELAY);
  pCardSize = MFRC522_SelectTag(pCardId);
  
  Serial.print("Card size: ");
  Serial.println(pCardSize);
  
  if (pCardSize == 0) {
    Serial.println("No data available !!!");
    return false;
  }
  
  return true;
}

void MifareOne::Halt()
{
  MFRC522_SelectChip(pEnable);

  delay(MF_DELAY);
  MFRC522_Halt();
}

void MifareOne::Reset()
{
  MFRC522_SelectChip(pEnable);

  delay(MF_DELAY);
  MFRC522_Init();

  pCardSize = 0;
}

