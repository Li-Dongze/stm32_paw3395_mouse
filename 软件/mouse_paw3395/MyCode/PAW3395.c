#include "PAW3395.h"
#include "Delay.h"
#include "usr_spi.h"
static void Power_Up_Initializaton_Register_Setting(void);


/*
 *严格按照开机顺序执行：
 *	虽然芯片执行内部上电自复位，但仍建议将Power_Up_Reset
 *	每次上电时都会写入寄存器。推荐的芯片上电顺序如下:
 *	1. 以任何顺序为VDD和VDDIO供电，每次供电之间的延迟不超过100ms。确保所有供应稳定。
 *	2. 等待至少50毫秒。
 *	3. 将NCS拉高，然后拉低以重置SPI端口。
 *	4. 将0x5A写入Power_Up_Reset寄存器（或者切换NRESET引脚）。
 *	5. 等待至少5ms。
 *	6. 加载上电初始化寄存器设置。
 *	7. 无论运动位状态如何，都读取寄存器0x02、0x03、0x04、0x05和0x06一次。
 *
 *
 *
 *
 *
 */
void Power_up_sequence(void)
{
	uint8_t reg_it;
	//Wait for at least 50ms（2.等待至少50毫秒。）
	delay_ms(50);
	// drive NCS high, then low to reset the SPI port（3.重置SPI端口）
	CS_Low;
	delay_125_ns(PAW3395_TIMINGS_NCS_SCLK);
	CS_High;
	delay_125_ns(PAW3395_TIMINGS_NCS_SCLK);
	CS_Low;
	delay_125_ns(PAW3395_TIMINGS_NCS_SCLK);
		
	// Write 0x5A to POWER_UP_Reset register（4.将0x5A写入Power_Up_Reset寄存器）
	writr_register(PAW3395_SPIREGISTER_POWERUPRESET
					,PAW3395_POWERUPRESET_POWERON);
	//Wait for at least 5ms（5.等待至少5ms）
	delay_ms(5);
	//Load Power-up initialization register setting（6.加载上电初始化寄存器设置）
	Power_Up_Initializaton_Register_Setting();
	CS_High;
	delay_125_ns(PAW3395_TIMINGS_NCS_SCLK);
	// read register from 0x02 to 0x06（7.无论运动位状态如何，都读取寄存器0x02、0x03、0x04、0x05和0x06一次）
	for(reg_it=0x02; reg_it<=0x06; reg_it++)
	{	
		read_register(reg_it);
		delay_us(PAW3395_TIMINGS_SRWSRR);	  
	}
	//片选拉高，本次SPI通讯结束
	CS_High;
	delay_125_ns(PAW3395_TIMINGS_BEXIT);
}


/*
 *严格执行启动Motion Burst的程序：
 *	1. 降低NCS。
 *	2. 等待tNCS-SCLK
 *	3. 发送Motion_Burst地址(0x16)。发送此地址后，MOSI应保持静态（高电平或低电平），直到突发传输完成。
 *	4. 等待tSRAD
 *	5. 开始连续读取最多12个字节的SPI数据。可以通过将NCS拉高持续at来终止运动突发至少tBEXIT。
 *	6. 要读取新的运动突发数据，请从步骤1开始重复。
 *
 *注意：
 *	即使在运行或静止模式下，也可以从Burst_Motion_Read寄存器读取运动突发数据。
 *
 */
void Motion_Burst(uint8_t *buffer)
{
	//Lower NCS
	CS_Low;
	//Wait for t(NCS-SCLK)
	delay_125_ns(PAW3395_TIMINGS_NCS_SCLK);
	//Send Motion_Brust address(0x16)
	SPI_SendReceive(PAW3395_SPIREGISTER_MotionBurst);	//读
	//Wait for tSRAD
	delay_us(PAW3395_TIMINGS_SRAD);
	//Start reading SPI data continuously up to 12 bytes.
	for(uint8_t i = 0;i < 12;i++)
	{
		buffer[i] = SPI_SendReceive(0x00);
	}
	CS_High;
	delay_125_ns(PAW3395_TIMINGS_BEXIT);
}

/*
 *上电初始化寄存器设置
 *
 */
static void Power_Up_Initializaton_Register_Setting(void)
{
	uint8_t read_tmp;
	uint8_t i ;
	writr_register(0x7F ,0x07);
	writr_register(0x40 ,0x41);
	writr_register(0x7F ,0x00);
	writr_register(0x40 ,0x80);
	writr_register(0x7F ,0x0E);
	writr_register(0x55 ,0x0D);
	writr_register(0x56 ,0x1B);
	writr_register(0x57 ,0xE8);
	writr_register(0x58 ,0xD5);
	writr_register(0x7F ,0x14);
	writr_register(0x42 ,0xBC);
	writr_register(0x43 ,0x74);
	writr_register(0x4B ,0x20);
	writr_register(0x4D ,0x00);
	writr_register(0x53 ,0x0E);
	writr_register(0x7F ,0x05);
	writr_register(0x44 ,0x04);
	writr_register(0x4D ,0x06);
	writr_register(0x51 ,0x40);
	writr_register(0x53 ,0x40);
	writr_register(0x55 ,0xCA);
	writr_register(0x5A ,0xE8);
	writr_register(0x5B ,0xEA);
	writr_register(0x61 ,0x31);
	writr_register(0x62 ,0x64);
	writr_register(0x6D ,0xB8);
	writr_register(0x6E ,0x0F);

	writr_register(0x70 ,0x02);
	writr_register(0x4A ,0x2A);
	writr_register(0x60 ,0x26);
	writr_register(0x7F ,0x06);
	writr_register(0x6D ,0x70);
	writr_register(0x6E ,0x60);
	writr_register(0x6F ,0x04);
	writr_register(0x53 ,0x02);
	writr_register(0x55 ,0x11);
	writr_register(0x7A ,0x01);
	writr_register(0x7D ,0x51);
	writr_register(0x7F ,0x07);
	writr_register(0x41 ,0x10);
	writr_register(0x42 ,0x32);
	writr_register(0x43 ,0x00);
	writr_register(0x7F ,0x08);
	writr_register(0x71 ,0x4F);
	writr_register(0x7F ,0x09);
	writr_register(0x62 ,0x1F);
	writr_register(0x63 ,0x1F);
	writr_register(0x65 ,0x03);
	writr_register(0x66 ,0x03);
	writr_register(0x67 ,0x1F);
	writr_register(0x68 ,0x1F);
	writr_register(0x69 ,0x03);
	writr_register(0x6A ,0x03);
	writr_register(0x6C ,0x1F);

	writr_register(0x6D ,0x1F);
	writr_register(0x51 ,0x04);
	writr_register(0x53 ,0x20);
	writr_register(0x54 ,0x20);
	writr_register(0x71 ,0x0C);
	writr_register(0x72 ,0x07);
	writr_register(0x73 ,0x07);
	writr_register(0x7F ,0x0A);
	writr_register(0x4A ,0x14);
	writr_register(0x4C ,0x14);
	writr_register(0x55 ,0x19);
	writr_register(0x7F ,0x14);
	writr_register(0x4B ,0x30);
	writr_register(0x4C ,0x03);
	writr_register(0x61 ,0x0B);
	writr_register(0x62 ,0x0A);
	writr_register(0x63 ,0x02);
	writr_register(0x7F ,0x15);
	writr_register(0x4C ,0x02);
	writr_register(0x56 ,0x02);
	writr_register(0x41 ,0x91);
	writr_register(0x4D ,0x0A);
	writr_register(0x7F ,0x0C);
	writr_register(0x4A ,0x10);
	writr_register(0x4B ,0x0C);
	writr_register(0x4C ,0x40);
	writr_register(0x41 ,0x25);
	writr_register(0x55 ,0x18);
	writr_register(0x56 ,0x14);
	writr_register(0x49 ,0x0A);
	writr_register(0x42 ,0x00);
	writr_register(0x43 ,0x2D);
	writr_register(0x44 ,0x0C);
	writr_register(0x54 ,0x1A);
	writr_register(0x5A ,0x0D);
	writr_register(0x5F ,0x1E);
	writr_register(0x5B ,0x05);
	writr_register(0x5E ,0x0F);
	writr_register(0x7F ,0x0D);
	writr_register(0x48 ,0xDD);
	writr_register(0x4F ,0x03);
	writr_register(0x52 ,0x49);
		
	writr_register(0x51 ,0x00);
	writr_register(0x54 ,0x5B);
	writr_register(0x53 ,0x00);
		
	writr_register(0x56 ,0x64);
	writr_register(0x55 ,0x00);
	writr_register(0x58 ,0xA5);
	writr_register(0x57 ,0x02);
	writr_register(0x5A ,0x29);
	writr_register(0x5B ,0x47);
	writr_register(0x5C ,0x81);
	writr_register(0x5D ,0x40);
	writr_register(0x71 ,0xDC);
	writr_register(0x70 ,0x07);
	writr_register(0x73 ,0x00);
	writr_register(0x72 ,0x08);
	writr_register(0x75 ,0xDC);
	writr_register(0x74 ,0x07);
	writr_register(0x77 ,0x00);
	writr_register(0x76 ,0x08);
	writr_register(0x7F ,0x10);
	writr_register(0x4C ,0xD0);
	writr_register(0x7F ,0x00);
	writr_register(0x4F ,0x63);
	writr_register(0x4E ,0x00);
	writr_register(0x52 ,0x63);
	writr_register(0x51 ,0x00);
	writr_register(0x54 ,0x54);
	writr_register(0x5A ,0x10);
	writr_register(0x77 ,0x4F);
	writr_register(0x47 ,0x01);
	writr_register(0x5B ,0x40);
	writr_register(0x64 ,0x60);
	writr_register(0x65 ,0x06);
	writr_register(0x66 ,0x13);
	writr_register(0x67 ,0x0F);
	writr_register(0x78 ,0x01);
	writr_register(0x79 ,0x9C);
	writr_register(0x40 ,0x00);
	writr_register(0x55 ,0x02);
	writr_register(0x23 ,0x70);
	writr_register(0x22 ,0x01);

	//Wait for 1ms
	delay_ms(1);
	
	for( i = 0 ;i < 60 ;i++)
	{
		read_tmp = read_register(0x6C);
		if(read_tmp == 0x80 )
			break;
		delay_ms(1);
	}
	if(i == 60)
	{
		writr_register(0x7F ,0x14);
		writr_register(0x6C ,0x00);
		writr_register(0x7F ,0x00);
	}
	writr_register(0x22 ,0x00);
	writr_register(0x55 ,0x00);
	writr_register(0x7F ,0x07);
	writr_register(0x40 ,0x40);
	writr_register(0x7F ,0x00);
}

/*
 *RawData输出程序
 *	读取当前鼠标摄像头拍摄到的接触面的像素阵列信息
 *	像素信息存入数组pFrame中
 *
 *注意：
 *	在RawData输出过程中，必须将鼠标置于静止位置
 *
 */
void Pixel_Burst_Read(uint8_t* pFrame)
{
	uint8_t reg_tmp;
	//Lower NCS
	CS_Low;
	//Wait for t(NCS-SCLK)
	delay_125_ns(PAW3395_TIMINGS_NCS_SCLK);
	writr_register(0x7F ,0x00);
	writr_register(0x40 ,0x80);
	
	do
	{
		reg_tmp = read_register(PAW3395_SPIREGISTER_MOTION);
		delay_us(PAW3395_TIMINGS_SRWSRR);
	}
	while((reg_tmp & ((1 << PAW3395_OP_MODE0) | (1 << PAW3395_OP_MODE1))) != 0);
		
	writr_register(0x50 ,0x01);
	writr_register(0x55 ,0x04);
	writr_register(0x58 ,0xFF);
	
	do
	{
		reg_tmp = read_register(0x59);
		delay_us(PAW3395_TIMINGS_SRWSRR);
	}
	while((reg_tmp & ((1 << PAW3395_PG_FIRST) | (1 << PAW3395_PG_VALID))) 
		!= ((1 << PAW3395_PG_FIRST) | (1 << PAW3395_PG_VALID)));
	
	pFrame[35*2]=read_register(0x58);//Read the first rawdata from register 0x58
	delay_us(PAW3395_TIMINGS_SRWSRR);
	for(uint8_t width = 0;width < 36;width++)
	{
		for(uint8_t height = 0;height < 36;height++)
		{
			if((width == 0)&&(height == 0))
				continue;
			do
			{
				reg_tmp = read_register(0x59);
				delay_us(PAW3395_TIMINGS_SRWSRR);
			}
			while(!((reg_tmp >> PAW3395_PG_VALID) & 0x01));
			pFrame[(height * 36 + (35-width)) * 2] = read_register(0x58);
			delay_us(PAW3395_TIMINGS_SRWSRR);
		}
	}	
	
	writr_register(0x40 ,0x00);
	writr_register(0x50 ,0x00);
	writr_register(0x55 ,0x00);	
	CS_High;
	delay_125_ns(PAW3395_TIMINGS_BEXIT);
}


/*
 *
 *更改DPI
 *
 */
void DPI_Config(uint16_t CPI_Num)
{
	uint8_t temp;
	
	//Lower NCS
	CS_Low;
	//Wait for t(NCS-SCLK)
	delay_125_ns(PAW3395_TIMINGS_NCS_SCLK);
	
	//设置分辨率模式：X轴和Y轴分辨率均由RESOLUTION_X_LOW和RESOLUTION_X_HIGH定义
	writr_register(MOTION_CTRL, 0x00);
	
	//两个8位寄存器设置X轴分辨率
	temp = (uint8_t)(((CPI_Num/50) << 8) >> 8);
	writr_register(RESOLUTION_X_LOW, temp);
	temp = (uint8_t)((CPI_Num/50) >> 8);
	writr_register(RESOLUTION_X_HIGH, temp);
	
	//更新分辨率
	writr_register(SET_RESOLUTION, 0x01);
	
	CS_High;
	delay_125_ns(PAW3395_TIMINGS_BEXIT);
}
