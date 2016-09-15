enum MFState {
  MF_IDLE   = 0,
  MF_READY  = 1,
  MF_ACTIVE = 2,
  MF_HALT   = 3  
};

void PrintHex(byte b, bool prefix = false, char suffix = 0);
bool ValidateHex(uchar *str);

class MifareOne {
  public:
    MifareOne(byte chipSelect);

    void Init();
    void changeState(MFState state);
    
    bool queryIdentifier(bool verbose);
    void readBlock(uchar blockNo);
    void writeBlock(uchar blockNo, uchar* data);
    
  private:
    enum MFState pState;
    byte pEnable;

    uchar keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uchar keyB[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    uchar pCardId[5];
    uchar pCardSize;

    static byte sTransferMap[4][4];
    static byte sBuffer[50];
    static byte sStatus;
    
    bool RequestAll(bool verbose);
    bool SelectCard();
    void Halt();
    void Reset();
};
