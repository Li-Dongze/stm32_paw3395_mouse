#ifndef _SPI_H_
#define _SPI_H_
#include "stdint.h"
#include "gpio.h"

#define CS_High HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET)
#define CS_Low	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET)

#define SPI_I2S_FLAG_RXNE               ((uint16_t)0x0001)
#define SPI_I2S_FLAG_TXE                ((uint16_t)0x0002)	
/**********************************************************************************************************/
	void SPI1_Init(void);
	uint8_t SPI_SendReceive(uint8_t data);
	uint8_t read_register(uint8_t adress);
	void writr_register(uint8_t adress,uint8_t vlue);


#endif
