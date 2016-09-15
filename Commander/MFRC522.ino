
uint8_t selectPin = 0;

//-----------------------------------------------------------------------------
void MFRC522_SelectChip(int en)
{
    selectPin = en;
}

//-----------------------------------------------------------------------------
// * 函 數 名：ResetMFRC522
// * 功能描述：復位RC522
// * 輸入參數：無
// * 返 回 值：無
void MFRC522_Reset(void)
{
    Write_MFRC522(CommandReg, PCD_RESETPHASE);
}

//-----------------------------------------------------------------------------
// * 函 數 名：InitMFRC522
// * 功能描述：初始化RC522
// * 輸入參數：無
// * 返 回 值：無
void MFRC522_Init(void)
{
    MFRC522_Reset();
    
    //Timer: TPrescaler * TreloadVal / 6.78MHz = 24ms
    Write_MFRC522(TModeReg, 0x8D);      //Tauto=1; f(Timer) = 6.78MHz/TPreScaler
    Write_MFRC522(TPrescalerReg, 0x3E); //TModeReg[3..0] + TPrescalerReg
    Write_MFRC522(TReloadRegL, 30);           
    Write_MFRC522(TReloadRegH, 0);
  
    Write_MFRC522(TxAutoReg, 0x40);     //100%ASK
    Write_MFRC522(ModeReg, 0x3D);       //CRC初始值0x6363  ???
  
    //ClearBitMask(Status2Reg, 0x08);   //MFCrypto1On=0
    //Write_MFRC522(RxSelReg, 0x86);    //RxWait = RxSelReg[5..0]
    //Write_MFRC522(RFCfgReg, 0x7F);    //RxGain = 48dB

    AntennaOn();    //打開天線
}

//-----------------------------------------------------------------------------
// * 函 數 名：MFRC522_Request
// * 功能描述：尋卡，讀取卡類型號
// * 輸入參數：reqMode--尋卡方式，
// *       TagType--返回卡片類型
// *        0x4400 = Mifare_UltraLight
// *        0x0400 = Mifare_One(S50)
// *        0x0200 = Mifare_One(S70)
// *        0x0800 = Mifare_Pro(X)
// *        0x4403 = Mifare_DESFire
// * 返 回 值：成功返回MI_OK
uchar MFRC522_Request(uchar reqMode, uchar *TagType)
{
    uchar status;  
    uint backBits;      //接收到的資料位元數

    Write_MFRC522(BitFramingReg, 0x07);   //TxLastBists = BitFramingReg[2..0] ???
    
    TagType[0] = reqMode;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);
  
    if((status != MI_OK) || (backBits != 0x10)){    
        status = MI_ERR;
    }
    
    return status;
}

//-----------------------------------------------------------------------------
// * 函 數 名：MFRC522_ToCard
// * 功能描述：RC522和ISO14443卡通訊
// * 輸入參數：command--MF522命令字，
// *       sendData--通過RC522發送到卡片的資料, 
// *       sendLen--發送的資料長度    
// *       backData--接收到的卡片返回資料，
// *       backLen--返回資料的位元長度
// * 返 回 值：成功返回MI_OK
uchar MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen)
{
    uchar status = MI_ERR;
    uchar irqEn = 0x00;
    uchar waitIRq = 0x00;
    uchar lastBits;
    uchar n;
    uint i;

    switch(command){
        case PCD_AUTHENT:   //認證卡密
            irqEn = 0x12;
            waitIRq = 0x10;
            break;
        case PCD_TRANSCEIVE:  //發送FIFO中資料
            irqEn = 0x77;
            waitIRq = 0x30;
            break;
        default:
            break;
    }
   
    Write_MFRC522(CommIEnReg, irqEn|0x80);  //允許插斷要求
    ClearBitMask(CommIrqReg, 0x80);         //清除所有插斷要求位
    SetBitMask(FIFOLevelReg, 0x80);         //FlushBuffer=1, FIFO初始化
    
    Write_MFRC522(CommandReg, PCD_IDLE);    //NO action;取消當前命令  ???

    //向FIFO中寫入資料
    for(i = 0; i < sendLen; i++){   
        Write_MFRC522(FIFODataReg, sendData[i]);    
    }

    //執行命令
    Write_MFRC522(CommandReg, command);
    if(command == PCD_TRANSCEIVE){    
        SetBitMask(BitFramingReg, 0x80);    //StartSend=1,transmission of data starts  
    }   
    
    //等待接收資料完成
    i = 2000; //i根據時鐘頻率調整，操作M1卡最大等待時間25ms ???
    do{
        //CommIrqReg[7..0]
        //Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        n = Read_MFRC522(CommIrqReg);
        i--;
    }
    while((i != 0) && !(n & 0x01) && !(n & waitIRq));

    ClearBitMask(BitFramingReg, 0x80);      //StartSend=0
  
    if(i != 0){    
        if(!(Read_MFRC522(ErrorReg) & 0x1B)){    //BufferOvfl Collerr CRCErr ProtecolErr
            status = MI_OK;
            if(n & irqEn & 0x01) status = MI_NOTAGERR;     //??   

            if(command == PCD_TRANSCEIVE){
                n = Read_MFRC522(FIFOLevelReg);
                lastBits = Read_MFRC522(ControlReg) & 0x07;
                if (lastBits){   
                    *backLen = (n - 1) * 8 + lastBits;   
                }
                else{   
                    *backLen = n * 8;   
                }

                if(n == 0) n = 1;    
                
                if(n > MAX_LEN) n = MAX_LEN;   
        
                //讀取FIFO中接收到的資料
                for(i = 0; i < n; i++){   
                    backData[i] = Read_MFRC522(FIFODataReg);    
                }
            }
        }
        else{   
            status = MI_ERR;  
        }       
    }
  
    //SetBitMask(ControlReg,0x80);           //timer stops
    //Write_MFRC522(CommandReg, PCD_IDLE); 

    return status;
}

//-----------------------------------------------------------------------------
// * 函 數 名：MFRC522_Anticoll
// * 功能描述：防衝突檢測，讀取選中卡片的卡序號
// * 輸入參數：serNum--返回4位元組卡序號,第5位元組為校驗位元組
// * 返 回 值：成功返回MI_OK
uchar MFRC522_Anticoll(uchar *serNum)
{
    uchar status;
    uchar i;
    uchar serNumCheck = 0;
    uint unLen;    

    //ClearBitMask(Status2Reg, 0x08);   //TempSensclear
    //ClearBitMask(CollReg,0x80);       //ValuesAfterColl
    Write_MFRC522(BitFramingReg, 0x00);   //TxLastBists = BitFramingReg[2..0]
 
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if(status == MI_OK){
        //校驗卡序號
        for(i = 0; i < 4; i++){   
            serNumCheck ^= serNum[i];
        }
        
        if(serNumCheck != serNum[i]){   
            status = MI_ERR;    
        }
    }

    //SetBitMask(CollReg, 0x80);    //ValuesAfterColl=1

    return status;
}

//-----------------------------------------------------------------------------
// * 函 數 名：MFRC522_SelectTag
// * 功能描述：選卡，讀取卡記憶體容量
// * 輸入參數：serNum--傳入卡序號
// * 返 回 值：成功返回卡容量
uchar MFRC522_SelectTag(uchar *serNum)
{
    uchar i;
    uchar status;
    uchar size;
    uint recvBits;
    uchar buffer[9]; 

    //ClearBitMask(Status2Reg, 0x08);     //MFCrypto1On=0

    buffer[0] = PICC_SELECTTAG;
    buffer[1] = 0x70;
    for(i = 0; i < 5; i++){
        buffer[i + 2] = *(serNum+i);
    }
    
    CalulateCRC(buffer, 7, &buffer[7]);   //??
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
    
    if ((status == MI_OK) && (recvBits == 0x18)){
        size = buffer[0]; 
    }
    else{   
        size = 0;    
    }

    return size;
}

//-----------------------------------------------------------------------------
// * 函 數 名：MFRC522_Auth
// * 功能描述：驗證卡片密碼
// * 輸入參數：authMode--密碼驗證模式
//             0x60 = 驗證A金鑰
//             0x61 = 驗證B金鑰 
//             BlockAddr--塊地址
//             Sectorkey--磁區密碼
//             serNum--卡片序號，4位元組
// * 返 回 值：成功返回MI_OK
uchar MFRC522_Auth(uchar authMode, uchar BlockAddr, uchar *Sectorkey, uchar *serNum)
{
    uchar status;
    uint recvBits;
    uchar i;
    uchar buff[12]; 

    //驗證指令+塊位址＋磁區密碼＋卡序號
    buff[0] = authMode;
    buff[1] = BlockAddr;
    for(i = 0; i < 6; i++){    
        buff[i+2] = *(Sectorkey + i);   
    }
    
    for(i = 0; i < 4; i++){    
        buff[i+8] = *(serNum + i);   
    }
    
    status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);
    if((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08))){   
        status = MI_ERR;   
     }
    
    return status;
}

//-----------------------------------------------------------------------------
// * 函 數 名：MFRC522_Read
// * 功能描述：讀塊數據
// * 輸入參數：blockAddr--塊地址;recvData--讀出的塊數據
// * 返 回 值：成功返回MI_OK
uchar MFRC522_Read(uchar blockAddr, uchar *recvData)
{
uchar status;
uint unLen;

    recvData[0] = PICC_READ;
    recvData[1] = blockAddr;
    
    CalulateCRC(recvData,2, &recvData[2]);
    
    status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);
    if ((status != MI_OK) || (unLen != 0x90)){
        status = MI_ERR;
    }
    
    return status;
}

//-----------------------------------------------------------------------------
// * 函 數 名：MFRC522_Write
// * 功能描述：寫塊數據
// * 輸入參數：blockAddr--塊地址;writeData--向塊寫16位元組資料
// * 返 回 值：成功返回MI_OK
uchar MFRC522_Write(uchar blockAddr, uchar *writeData)
{
    uchar status;
    uint recvBits;
    uchar i;
    uchar buff[18]; 
    
    buff[0] = PICC_WRITE;
    buff[1] = blockAddr;
    CalulateCRC(buff, 2, &buff[2]);
    
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);
    if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)){   
        status = MI_ERR;   
    }
        
    if(status == MI_OK){
        for(i = 0; i < 16; i++){    //向FIFO寫16Byte數據
            buff[i] = *(writeData + i);   
        }
        
        CalulateCRC(buff, 16, &buff[16]);
        
        status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);        
        if((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)){   
            status = MI_ERR;   
        }
    }
    
    return status;
}

//-----------------------------------------------------------------------------
// * 函 數 名：MFRC522_Halt
// * 功能描述：命令卡片進入休眠狀態
// * 輸入參數：無
// * 返 回 值：無
void MFRC522_Halt(void)
{
    uchar status;
    uint unLen;
    uchar buff[4]; 

    buff[0] = PICC_HALT;
    buff[1] = 0;
    
    CalulateCRC(buff, 2, &buff[2]);
 
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}

// Utility functions

//-----------------------------------------------------------------------------
// * 函 數 名：Write_MFRC5200
// * 功能描述：向MFRC522的某一寄存器寫一個位元組資料
// * 輸入參數：addr--寄存器位址；val--要寫入的值
// * 返 回 值：無
void Write_MFRC522(uchar addr, uchar val)
{
    digitalWrite(selectPin, LOW);
  
    //地址格式：0XXXXXX0
    SPI.transfer((addr << 1) & 0x7E); 
    SPI.transfer(val);
    
    digitalWrite(selectPin, HIGH);
}

//-----------------------------------------------------------------------------
// * 函 數 名：Read_MFRC522
// * 功能描述：從MFRC522的某一寄存器讀一個位元組資料
// * 輸入參數：addr--寄存器位址
// * 返 回 值：返回讀取到的一個位元組資料
uchar Read_MFRC522(uchar addr)
{
uchar val;

    digitalWrite(selectPin, LOW);
  
    //地址格式：1XXXXXX0
    SPI.transfer(((addr << 1) & 0x7E) | 0x80);  
    val = SPI.transfer(0x00);
    
    digitalWrite(selectPin, HIGH);
    
    return val; 
}

//-----------------------------------------------------------------------------
// * 函 數 名：SetBitMask
// * 功能描述：置RC522寄存器位
// * 輸入參數：reg--寄存器位址;mask--置位值
// * 返 回 值：無
void SetBitMask(uchar reg, uchar mask)  
{
    uchar tmp;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp | mask);  // set bit mask
}

//-----------------------------------------------------------------------------
// * 函 數 名：ClearBitMask
// * 功能描述：清RC522寄存器位
// * 輸入參數：reg--寄存器位址;mask--清位值
// * 返 回 值：無
void ClearBitMask(uchar reg, uchar mask)  
{
    uchar tmp;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp & (~mask));  // clear bit mask
} 

//-----------------------------------------------------------------------------
// * 函 數 名：AntennaOn
// * 功能描述：開啟天線,每次啟動或關閉天險發射之間應至少有1ms的間隔
// * 輸入參數：無
// * 返 回 值：無
void AntennaOn(void)
{
    uchar temp;
  
    temp = Read_MFRC522(TxControlReg);
    if(!(temp & 0x03)){
        SetBitMask(TxControlReg, 0x03);
    }
}

//-----------------------------------------------------------------------------
// * 函 數 名：AntennaOff
// * 功能描述：關閉天線,每次啟動或關閉天險發射之間應至少有1ms的間隔
// * 輸入參數：無
// * 返 回 值：無
void AntennaOff(void)
{
    ClearBitMask(TxControlReg, 0x03);
}

//-----------------------------------------------------------------------------
// * 函 數 名：CalulateCRC
// * 功能描述：用MF522計算CRC
// * 輸入參數：pIndata--要讀數CRC的資料，len--資料長度，pOutData--計算的CRC結果
// * 返 回 值：無
void CalulateCRC(uchar *pIndata, uchar len, uchar *pOutData)
{
uchar i, n;

    ClearBitMask(DivIrqReg, 0x04);      //CRCIrq = 0
    SetBitMask(FIFOLevelReg, 0x80);     //清FIFO指針
    //Write_MFRC522(CommandReg, PCD_IDLE);

    //向FIFO中寫入資料  
    for(i = 0; i < len; i++){   
        Write_MFRC522(FIFODataReg, *(pIndata + i));   
    }
    Write_MFRC522(CommandReg, PCD_CALCCRC);

    //等待CRC計算完成
    i = 0xFF;
    do{
        n = Read_MFRC522(DivIrqReg);
        i--;
    }
    while ((i != 0) && !(n & 0x04));      //CRCIrq = 1

    //讀取CRC計算結果
    pOutData[0] = Read_MFRC522(CRCResultRegL);
    pOutData[1] = Read_MFRC522(CRCResultRegM);
}

//-----------------------------------------------------------------------------
