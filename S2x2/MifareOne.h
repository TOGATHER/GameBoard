enum MFState {
  MF_IDLE   = 0,
  MF_READY  = 1,
  MF_ACTIVE = 2,
  MF_HALT   = 3  
};


class MifareOne {
  public:
    MifareOne(byte chipSelect);

    void Init();
    bool changeState(MFState state);
    
    bool queryIdentifier(bool verbose);
    bool readBlock(uchar blockNo, byte *data);
    void writeBlock(uchar blockNo, uchar* data);

    uchar *getCardId();
    
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
