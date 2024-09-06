#include "delay.h"
#define SCS_BASE            (0xE000E000UL)                            /*!< System Control Space Base Address */
#define SysTick_BASE        (SCS_BASE +  0x0010UL)                    /*!< SysTick Base Address */

/* following defines should be used for structure members */
#define     __IM     volatile const      /*! Defines 'read only' structure member permissions */
#define     __OM     volatile            /*! Defines 'write only' structure member permissions */
#define     __IOM    volatile            /*! Defines 'read / write' structure member permissions */

typedef struct
{
  __IOM uint32_t CTRL;                   /*!< Offset: 0x000 (R/W)  SysTick Control and Status Register */
  __IOM uint32_t LOAD;                   /*!< Offset: 0x004 (R/W)  SysTick Reload Value Register */
  __IOM uint32_t VAL;                    /*!< Offset: 0x008 (R/W)  SysTick Current Value Register */
  __IM  uint32_t CALIB;                  /*!< Offset: 0x00C (R/ )  SysTick Calibration Register */
} SysTick_Type;

#define SysTick             ((SysTick_Type   *)     SysTick_BASE  )   /*!< SysTick configuration struct */

//void delay_ms(unsigned int t)
//{
//	int i;
//	for( i=0;i<t;i++)
//	{
//		int a=10300;//10300
// 		while(a--);
//	}
//}

//void delay_us(unsigned int t)
//{
//	int i;
//	for( i=0;i<t;i++)
//	{
//		int a=9;//9
//		while(a--);
//	}
//}

/******************************************************************************/
                                 //用于3.0或者以上库
/******************************************************************************/

uint8_t   fac_us;  //延时基数（延时一微秒所要设置的时间）    
uint16_t  fac_ms;
void delay_init(uint8_t SYSCLK)
{
	SysTick->CTRL &= 0xfffffffb;// HCLK/8   bit2 = CLKSOURCE    选择外部时钟  0=外部时钟源 1=内核时钟源
	fac_us=SYSCLK/8;      
	fac_ms=(uint16_t)fac_us*1000; 	
}
void delay_us(uint32_t nus)
{
	uint32_t temp;
	SysTick->LOAD = nus*fac_us;//加载的时间
	SysTick->VAL = 0x00; //清空计数器
	SysTick->CTRL = 0x01; //开始倒数
	temp = SysTick->CTRL;
	while((temp&0x01)&&(!(temp&(1<<16))))temp = SysTick->CTRL;
	SysTick->CTRL=0x00;       //关闭计数器 
	SysTick->VAL =0X00;       //清空计数器 
				
}
void delay_ms(uint16_t nms)
{
	uint32_t temp;
	SysTick->LOAD = (uint32_t)nms*fac_ms;//加载的时间
	SysTick->VAL = 0x00; //清空计数器
	SysTick->CTRL = 0x01; //开始倒数
	temp = SysTick->CTRL;
	while((temp&0x01)&&(!(temp&(1<<16)))) temp = SysTick->CTRL;
	SysTick->CTRL=0x00;       //关闭计数器 
	SysTick->VAL =0X00;       //清空计数器 		
}

void delay_125_ns(uint8_t nns)
{
	uint32_t temp;
	SysTick->LOAD = (uint32_t)nns;//加载的时间
	SysTick->VAL = 0x00; //清空计数器
	SysTick->CTRL = 0x01; //开始倒数
	temp = SysTick->CTRL;
	while((temp&0x01)&&(!(temp&(1<<16)))) temp = SysTick->CTRL;
	SysTick->CTRL=0x00;       //关闭计数器 
	SysTick->VAL =0X00;       //清空计数器 
}
