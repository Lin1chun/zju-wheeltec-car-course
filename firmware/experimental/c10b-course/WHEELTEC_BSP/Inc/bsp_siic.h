#ifndef __BSP_sIIC_H
#define __BSP_sIIC_H

#include "main.h"

typedef enum 
{
	IIC_OK       = 0x00U,
	IIC_ERR      = 0x01U,
	IIC_BUSY     = 0x02U,
	IIC_TIMEOUT  = 0x03U,
	I2C_BUS_ERROR,
	I2C_ARBITRATION_LOST,
	I2C_ADDR_NACK
} IIC_Status_t;


typedef struct {
	// IIC外设初始化
	void (*init)(void);
	
	//参数：设备地址，要写入的数据，要写入的数据长度，超时时间
	// 基础IIC发送：无寄存器，直接向设备写数据
  IIC_Status_t (*write)(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
  IIC_Status_t (*read)(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
	
	//参数：设备地址，要访问的寄存器地址，要写入的数据，要写入的数据长度，超时时间
	// 带寄存器写入：先发送寄存器地址，再写入数据（绝大多数传感器使用）
  IIC_Status_t (*write_reg)(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
	IIC_Status_t (*read_reg)(uint16_t DevAddress, uint16_t MemAddress,  uint8_t *pData, uint16_t Size, uint32_t Timeout);
	
	// 毫秒延时，模拟IIC时序/等待器件稳定
	void (*delay_ms)(uint16_t ms);
}IICInterface_t,*pIICInterface_t;

extern IICInterface_t User_sIICDev;


#endif /* __BSP_IIC_H */

