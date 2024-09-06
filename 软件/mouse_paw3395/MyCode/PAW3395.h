#ifndef PAW3395_H_
#define PAW3395_H_
#include "stdint.h"
//Register Address
#define PAW3395_SPIREGISTER_MOTION        0x02	

#define PAW3395_SPIREGISTER_MotionBurst 	0x16

#define PAW3395_SPIREGISTER_POWERUPRESET	0x3A

/*******************CPIÅäÖÃ¼Ä´æÆ÷********************/
#define SET_RESOLUTION										0x47

#define RESOLUTION_X_LOW									0x48
#define RESOLUTION_X_HIGH									0x49

#define RESOLUTION_Y_LOW									0x4A
#define RESOLUTION_Y_HIGH									0x4B

#define RIPPLE_CONTROL										0x5A

#define MOTION_CTRL												0x5C

//Register Value
#define PAW3395_POWERUPRESET_POWERON	0x5A

// Registers bits
#define PAW3395_OP_MODE0										 0
#define PAW3395_OP_MODE1										 1

#define PAW3395_PG_FIRST										 6
#define PAW3395_PG_VALID										 7

//Operation time 

/// SPI read - address delay
#define PAW3395_TIMINGS_SRAD 2 //2¦Ìs

/// SPI time between read and subsequent commands
#define PAW3395_TIMINGS_SRWSRR 2//2¦Ìs

/// SPI time between write commands
#define PAW3395_TIMINGS_SWW 5//5¦Ìs

/// SPI time between write and read commands
#define PAW3395_TIMINGS_SWR 5//5¦Ìs

/// SPI, NCS to SCLK active/inactive
#define PAW3395_TIMINGS_NCS_SCLK 1//120ns

/// Time to drive NCS high in order to exit burst mode
#define PAW3395_TIMINGS_BEXIT 4//500ns

#define PAW3395_SPI_WRITE	  0x80

void Power_up_sequence(void);
void Motion_Burst(uint8_t *buffer);
void Pixel_Burst_Read(uint8_t* pFrame);
void DPI_Config(uint16_t CPI_Num);
#endif
