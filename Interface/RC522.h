#ifndef __RC522_H
#define	__RC522_H
#include "spi.h"
#include "gpio.h"

/*********************************** RC522 引脚操作宏 *********************************************/
#define               RC522_CS_ENABLE()    	                HAL_GPIO_WritePin(RC522_CS_GPIO_Port,RC522_CS_Pin,GPIO_PIN_RESET)			   
#define               RC522_CS_DISABLE() 		                HAL_GPIO_WritePin(RC522_CS_GPIO_Port,RC522_CS_Pin,GPIO_PIN_SET)   // SDA → PA8

#define          RC522_RST_L()           HAL_GPIO_WritePin(RC522_RST_GPIO_Port,RC522_RST_Pin,GPIO_PIN_RESET)
#define          RC522_RST_H()           HAL_GPIO_WritePin(RC522_RST_GPIO_Port,RC522_RST_Pin,GPIO_PIN_SET)


/********************* RC522 命令/寄存器定义 **********************/
// MF522命令字
#define PCD_IDLE              0x00               // 取消当前命令
#define PCD_AUTHENT           0x0E               // 验证密钥
#define PCD_RECEIVE           0x08               // 接收数据
#define PCD_TRANSMIT          0x04               // 发送数据
#define PCD_TRANSCEIVE        0x0C               // 发送并接收数据
#define PCD_RESETPHASE        0x0F               // 复位
#define PCD_CALCCRC           0x03               // CRC计算

// Mifare_One卡片命令字
#define PICC_REQIDL           0x26               // 寻天线区内未进入休眠状态
#define PICC_REQALL           0x52               // 寻天线区内全部卡片
#define PICC_ANTICOLL1        0x93               // 防冲突
#define PICC_AUTHENT1A        0x60               // 验证A密钥
#define PICC_AUTHENT1B        0x61               // 验证B密钥
#define PICC_READ             0x30               // 读块
#define PICC_WRITE            0xA0               // 写块
#define PICC_HALT             0x50               // 休眠

// MF522寄存器定义
#define CommandReg            0x01    
#define ComIEnReg             0x02    
#define ComIrqReg             0x04    
#define ErrorReg              0x06    
#define Status2Reg            0x08    
#define FIFODataReg           0x09
#define FIFOLevelReg          0x0A
#define ControlReg            0x0C
#define BitFramingReg         0x0D
#define CollReg               0x0E
#define ModeReg               0x11
#define TxControlReg          0x14
#define TxAutoReg             0x15
#define CRCResultRegM         0x21
#define CRCResultRegL         0x22
#define TModeReg              0x2A
#define TPrescalerReg         0x2B
#define TReloadRegH           0x2C
#define TReloadRegL           0x2D
#define DivIrqReg             0x05 
// 通信返回码
#define MI_OK                 0x26
#define MI_NOTAGERR           0xCC
#define MI_ERR                0xBB

// 最大长度
#define MAXRLEN  18

/********************* 函数声明 **********************/
void RC522_Init(void);
char RC522_Reset(void);
void Write_RC522(unsigned char Address, unsigned char value);
unsigned char Read_RC522(unsigned char Address);  
void RC522_AntennaOn(void);
void RC522_AntennaOff(void);
char RC522_Request(unsigned char req_code,unsigned char *pTagType);
char RC522_Anticoll(unsigned char *pSnr);
char RC522_SelectTag(unsigned char *pSnr);
char RC522_AuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr);
char RC522_Read(unsigned char addr,unsigned char *pData);
char RC522_Halt(void);


#endif
