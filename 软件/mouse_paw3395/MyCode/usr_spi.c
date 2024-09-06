#include "stm32f1xx_hal.h"
#include "Delay.h"
#include "usr_spi.h"
FlagStatus SPI_I2S_GetFlagStatus(SPI_TypeDef* SPIx, uint16_t SPI_I2S_FLAG);
static void SPI_I2S_SendData(SPI_TypeDef* SPIx, uint16_t Data);
static uint16_t SPI_I2S_ReceiveData(SPI_TypeDef* SPIx);
extern SPI_HandleTypeDef hspi1;
uint8_t SPI_SendReceive(uint8_t data)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET){;}
      SPI_I2S_SendData(SPI1, data);	
	  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET){;}
	  return  SPI_I2S_ReceiveData(SPI1);

}

uint8_t read_register(uint8_t adress)
{
unsigned char temp;
	CS_Low;
	delay_125_ns(1);
	temp=SPI_SendReceive(adress+0x00);	//读
	delay_us(5);
	temp=SPI_SendReceive(0xff);	//提供时钟信号_读
//	CS_High;
	return temp;
}

void writr_register(uint8_t adress,uint8_t vlue)
{
	CS_Low;
	delay_125_ns(1);
	SPI_SendReceive(adress+0x80);
	SPI_SendReceive(vlue); 	
	CS_High;
	delay_us(5);
}

FlagStatus SPI_I2S_GetFlagStatus(SPI_TypeDef* SPIx, uint16_t SPI_I2S_FLAG)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_SPI_ALL_PERIPH(SPIx));
  assert_param(IS_SPI_I2S_GET_FLAG(SPI_I2S_FLAG));
  /* Check the status of the specified SPI/I2S flag */
  if ((SPIx->SR & SPI_I2S_FLAG) != (uint16_t)RESET)
  {
    /* SPI_I2S_FLAG is set */
    bitstatus = SET;
  }
  else
  {
    /* SPI_I2S_FLAG is reset */
    bitstatus = RESET;
  }
  /* Return the SPI_I2S_FLAG status */
  return  bitstatus;
}

static void SPI_I2S_SendData(SPI_TypeDef* SPIx, uint16_t Data)
{
  /* Check the parameters */
  assert_param(IS_SPI_ALL_PERIPH(SPIx));
  
  /* Write in the DR register the data to be sent */
  SPIx->DR = Data;
}
static uint16_t SPI_I2S_ReceiveData(SPI_TypeDef* SPIx)
{
  /* Check the parameters */
  assert_param(IS_SPI_ALL_PERIPH(SPIx));
  
  /* Return the data in the DR register */
  return SPIx->DR;
}
