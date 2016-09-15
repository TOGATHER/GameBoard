#define uchar unsigned char
#define uint unsigned int

//陣列最大長度
#define MAX_LEN 16

// MF522命令字
#define PCD_IDLE              0x00               //NO action;取消當前命令
#define PCD_AUTHENT           0x0E               //驗證金鑰
#define PCD_RECEIVE           0x08               //接收資料
#define PCD_TRANSMIT          0x04               //發送資料
#define PCD_TRANSCEIVE        0x0C               //發送並接收資料
#define PCD_RESETPHASE        0x0F               //復位
#define PCD_CALCCRC           0x03               //CRC計算

// Mifare_One卡片命令字
#define PICC_REQIDL           0x26               //尋天線區內未進入休眠狀態
#define PICC_REQALL           0x52               //尋天線區內全部卡
#define PICC_ANTICOLL         0x93               //防衝撞
#define PICC_SELECTTAG        0x93               //選卡
#define PICC_AUTHENT1A        0x60               //驗證A金鑰
#define PICC_AUTHENT1B        0x61               //驗證B金鑰
#define PICC_READ             0x30               //讀塊
#define PICC_WRITE            0xA0               //寫塊
#define PICC_DECREMENT        0xC0               
#define PICC_INCREMENT        0xC1               
#define PICC_RESTORE          0xC2               //調塊數據到緩衝區
#define PICC_TRANSFER         0xB0               //保存緩衝區中資料
#define PICC_HALT             0x50               //休眠

// 和MF522通訊時返回的錯誤代碼
#define MI_OK                 0
#define MI_NOTAGERR           1
#define MI_ERR                2

//------------------MFRC522寄存器---------------
//Page 0:Command and Status
#define     Reserved00            0x00    
#define     CommandReg            0x01    
#define     CommIEnReg            0x02    
#define     DivlEnReg             0x03    
#define     CommIrqReg            0x04    
#define     DivIrqReg             0x05
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08    
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     Reserved01            0x0F

//Page 1:Command     
#define     Reserved10            0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     Reserved11            0x1A
#define     Reserved12            0x1B
#define     MifareReg             0x1C
#define     Reserved13            0x1D
#define     Reserved14            0x1E
#define     SerialSpeedReg        0x1F

//Page 2:CFG    
#define     Reserved20            0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     Reserved21            0x23
#define     ModWidthReg           0x24
#define     Reserved22            0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsPReg              0x28
#define     ModGsPReg             0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F

//Page 3:TestRegister     
#define     Reserved30            0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     Reserved31            0x3C   
#define     Reserved32            0x3D   
#define     Reserved33            0x3E   
#define     Reserved34            0x3F

//-----------------------------------------------------------------------------
void MFRC522_SelectChip(int en);

//-----------------------------------------------------------------------------
// * 函 數 名：ResetMFRC522
// * 功能描述：復位RC522
// * 輸入參數：無
// * 返 回 值：無
void MFRC522_Reset(void);

//-----------------------------------------------------------------------------
// * 函 數 名：InitMFRC522
// * 功能描述：初始化RC522
// * 輸入參數：無
// * 返 回 值：無
void MFRC522_Init(void);

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
uchar MFRC522_Request(uchar reqMode, uchar *TagType);

//-----------------------------------------------------------------------------
// * 函 數 名：MFRC522_ToCard
// * 功能描述：RC522和ISO14443卡通訊
// * 輸入參數：command--MF522命令字，
// *       sendData--通過RC522發送到卡片的資料, 
// *       sendLen--發送的資料長度    
// *       backData--接收到的卡片返回資料，
// *       backLen--返回資料的位元長度
// * 返 回 值：成功返回MI_OK
uchar MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen);

//-----------------------------------------------------------------------------
// * 函 數 名：MFRC522_SelectTag
// * 功能描述：選卡，讀取卡記憶體容量
// * 輸入參數：serNum--傳入卡序號
// * 返 回 值：成功返回卡容量
uchar MFRC522_SelectTag(uchar *serNum);

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
uchar MFRC522_Auth(uchar authMode, uchar BlockAddr, uchar *Sectorkey, uchar *serNum);

//-----------------------------------------------------------------------------
// * 函 數 名：MFRC522_Read
// * 功能描述：讀塊數據
// * 輸入參數：blockAddr--塊地址;recvData--讀出的塊數據
// * 返 回 值：成功返回MI_OK
uchar MFRC522_Read(uchar blockAddr, uchar *recvData);

//-----------------------------------------------------------------------------
// * 函 數 名：MFRC522_Write
// * 功能描述：寫塊數據
// * 輸入參數：blockAddr--塊地址;writeData--向塊寫16位元組資料
// * 返 回 值：成功返回MI_OK
uchar MFRC522_Write(uchar blockAddr, uchar *writeData);

//-----------------------------------------------------------------------------
// * 函 數 名：MFRC522_Halt
// * 功能描述：命令卡片進入休眠狀態
// * 輸入參數：無
// * 返 回 值：無
void MFRC522_Halt(void);
