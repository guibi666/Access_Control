#include "RC522.h"

#define 	MAXRLEN 18

void RC522_Init(void)
{
	//初始化SPI
  MX_SPI1_Init();
  //初始化RC522_RST,RC522_CS(在GPIO初始化中);
  //片选CS
  RC522_CS_ENABLE();
  RC522_Reset();
}

// 从RC522的某一寄存器读一个字节数据
unsigned char Read_RC522(unsigned char Address)
{
  unsigned char ucAddr;
  unsigned char ucResult=0;

  //地址移位
  ucAddr = ((Address<<1)&0x7E)|0x80;

  //发送寄存器地址
  HAL_SPI_Transmit(&hspi1,(uint8_t *)ucAddr,1,HAL_MAX_DELAY);

  //接收1字节数据
  HAL_SPI_Receive(&hspi1,(uint8_t *)ucResult,1,HAL_MAX_DELAY);
  return ucResult;
}

// 向RC522的某一寄存器写一个字节数据
void Write_RC522(unsigned char Address, unsigned char value)
{  
  unsigned char ucAddr;

  //地址移位
  ucAddr = ((Address<<1)&0x7E);

  //发送寄存器地址
  HAL_SPI_Transmit(&hspi1,(uint8_t *)ucAddr,1,HAL_MAX_DELAY);

  HAL_SPI_Transmit(&hspi1,(uint8_t *)value,1,HAL_MAX_DELAY);

}

// 复位RC522
char RC522_Reset(void) 
{
    RC522_RST_H();
    HAL_Delay(1);             
    RC522_RST_L();
		HAL_Delay(1);                          
    RC522_RST_H();
		HAL_Delay(1);         	  

    Write_RC522(CommandReg,0x0F); // 软复位
    while(Read_RC522(CommandReg) & 0x10); // 等待芯片启动完成

		HAL_Delay(1);             

    Write_RC522(ModeReg,0x3D);            // 和Mifare卡通信，CRC初始值0x6363
    Write_RC522(TReloadRegL,30);           
    Write_RC522(TReloadRegH,0);
    Write_RC522(TModeReg,0x8D);
    Write_RC522(TPrescalerReg,0x3E);
    Write_RC522(TxAutoReg,0x40);
    return MI_OK;
}

// 设置RC522寄存器位
void SetBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = Read_RC522(reg);
    Write_RC522(reg,tmp | mask);  // set bit mask
}

// 清除RC522寄存器位
void ClearBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = Read_RC522(reg);
    Write_RC522(reg, tmp & ~mask);  // clear bit mask
} 

// 通过RC522和ISO14443卡通信
char RC522_ToCard(unsigned char Command, 
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned int  *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)
    {
       case PCD_AUTHENT:
          irqEn   = 0x12;
          waitFor = 0x10;
          break;
       case PCD_TRANSCEIVE:
          irqEn   = 0x77;
          waitFor = 0x30;
          break;
       default:
         break;
    }
   
    Write_RC522(ComIEnReg,irqEn|0x80);
    ClearBitMask(ComIrqReg,0x80);
    Write_RC522(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    
    for (i=0; i<InLenByte; i++)
    {   
			Write_RC522(FIFODataReg, pInData[i]);
		}
    Write_RC522(CommandReg, Command);
   
    if (Command == PCD_TRANSCEIVE)
    {    
			SetBitMask(BitFramingReg,0x80);
		}
    
		n = Read_RC522(ComIrqReg);
    i = 1500;
    do 
    {
         n = Read_RC522(ComIrqReg);
         i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    ClearBitMask(BitFramingReg,0x80);
	      
    if (i!=0)
    {    
         if(!(Read_RC522(ErrorReg)&0x1B))
         {
             status = MI_OK;
             if (n & irqEn & 0x01)
             {   status = MI_NOTAGERR;   }
             if (Command == PCD_TRANSCEIVE)
             {
               	n = Read_RC522(FIFOLevelReg);
              	lastBits = Read_RC522(ControlReg) & 0x07;
                if (lastBits)
                {   *pOutLenBit = (n-1)*8 + lastBits;   }
                else
                {   *pOutLenBit = n*8;   }
                if (n == 0)
                {   n = 1;    }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i=0; i<n; i++)
                {   pOutData[i] = Read_RC522(FIFODataReg);    }
            }
         }
         else
         {   
				status = MI_ERR;   
			}
   }
   
   SetBitMask(ControlReg,0x80);
   Write_RC522(CommandReg,PCD_IDLE); 
   return status;
}

// 开启天线
void RC522_AntennaOn(void)
{
    unsigned char i;
    i = Read_RC522(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}

// 关闭天线
void RC522_AntennaOff(void)
{
    ClearBitMask(TxControlReg, 0x03);
}

// 计算CRC
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
    unsigned char i,n;
    ClearBitMask(DivIrqReg,0x04);
    Write_RC522(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {   Write_RC522(FIFODataReg, *(pIndata+i));   }
    Write_RC522(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = Read_RC522(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = Read_RC522(CRCResultRegL);
    pOutData[1] = Read_RC522(CRCResultRegM);
}

// 命令卡片进入休眠状态
char RC522_Halt(void)
{
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    RC522_ToCard(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    return MI_OK;
}

// 寻卡
char RC522_Request(unsigned char req_code,unsigned char *pTagType)
{
   char status;  
   unsigned int  unLen;
   unsigned char ucComMF522Buf[MAXRLEN]; 

   ClearBitMask(Status2Reg,0x08);
   Write_RC522(BitFramingReg,0x07);
   SetBitMask(TxControlReg,0x03);
 
   ucComMF522Buf[0] = req_code;

   status = RC522_ToCard(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);
   
   if ((status == MI_OK) && (unLen == 0x10))
   {    
       *pTagType     = ucComMF522Buf[0];
       *(pTagType+1) = ucComMF522Buf[1];
   }
   else
   {   status = MI_ERR;  }
   
   return status;
}

// 防冲突检测，读取选中卡片的序列号
char RC522_Anticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i,snr_check=0;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ClearBitMask(Status2Reg,0x08);
    Write_RC522(BitFramingReg,0x00);
    ClearBitMask(CollReg,0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = RC522_ToCard(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
    {
        for (i=0; i<4; i++)
        {   
            *(pSnr+i)  = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i])
        {   status = MI_ERR;    }
    }
    
    SetBitMask(CollReg,0x80);
    return status;
}

// 选中卡片
char RC522_SelectTag(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
        ucComMF522Buf[i+2] = *(pSnr+i);
        ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    ClearBitMask(Status2Reg,0x08);

    status = RC522_ToCard(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   status = MI_OK;  }
    else
    {   status = MI_ERR;    }

    return status;
}

// 验证卡片密钥
char RC522_AuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+2] = *(pKey+i);   }
    for (i=0; i<4; i++)
    {    ucComMF522Buf[i+8] = *(pSnr+i);   }
    
    status = RC522_ToCard(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(Read_RC522(Status2Reg) & 0x08)))
    {   status = MI_ERR;   }
    
    return status;
}

// 读取M1卡一个扇区数据
char RC522_Read(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
   
    status = RC522_ToCard(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90))
    {
        for (i=0; i<16; i++)
        {    *(pData+i) = ucComMF522Buf[i];   }
    }
    else
    {   status = MI_ERR;   }
    
    return status;
}
