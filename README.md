# 基于stm32制作的PAW3395鼠标开源介绍

## 一、项目简介

B站演示视频：

github开源地址：

本项目是一款基于stm32f103c8t6单片机制作的有线鼠标，使用的光电传感器为原相paw3395，使用HID协议与电脑通讯，尼龙外壳使用立创三维猴3D打印。

项目实现功能有：

- 正常鼠标功能：光标移动、左右键、中键、滚轮、DPI切换，DPI切换时LED闪烁3次
- 拓展功能：两个自定义按键（已预留接口），本例程未添加

<img src="https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409052004277.png" alt="image-20240905200432905" style="zoom:50%;" />

<img src="https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409052005416.png" alt="image-20240905200511964" style="zoom:50%;" />



## 二、制作动机

本人前段时间做数字图像处理时接触过一款adns3080光流传感器，能够近距离拍照生成黑白像素图像。深入了解后发现和鼠标使用的传感器原理一样，索性就想制作一款鼠标。本着要做就做最好的原则，选择了当时最流行的鼠标传感器paw3395。主控则选择手头最多的stm32f103c8t6。

## 三、软件具体实现

### 1. 总体架构

使用一种类RTOS操作系统的架构，其实也是传统前后台架构的一种，实现任务的时间片轮询，不加delay占用cpu资源

```c
#define TASKNUM_MAX	4

typedef struct{
	void (*pTask)(void);    //任务函数
	uint16_t TaskPeriod;    //多少毫秒调用一次任务函数
}TaskStruct;

/***************自定义一个定时器中断，我以定时器3为例，每次进入给每个任务减一个时钟节拍，中断进入时间可自定义，本例1ms******************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *hitm)
{
	for(i = 0; i < TASKNUM_MAX; i++){
		if(TaskTimer[i])
			TaskTimer[i]--;
	}	
}

/******************************任务调度机制*********************************/
void Task_Init(void)
{
	uint8_t NTask;
	for(NTask = 0; NTask < sizeof(Task)/sizeof(Task[0]); NTask++){
		TaskTimer[NTask] = Task[NTask].TaskPeriod;
	}
}	

void Task_Run(void)
{
	uint8_t NTask;
	for(NTask = 0; NTask < sizeof(Task)/sizeof(Task[0]); NTask++){
		if(TaskTimer[NTask] == 0)
		{
			TaskTimer[NTask] = Task[NTask].TaskPeriod;
			(Task[NTask].pTask)();
		}
	}
}

/******************************具体任务函数声明*********************************/
void Key_Task(void);			//按键任务
void Mouse_XY_Updata(void);		//HID协议发送电脑任务
void Mouse_wheel_Updata(void);	//鼠标滚轮更新任务
void LED_Task(void);			//LED任务

/******************************全局变量声明*********************************/
uint16_t TaskTimer[TASKNUM_MAX];
TaskStruct Task[] = {	// 添加任务函数
	{Key_Task, 20},
	{Mouse_XY_Updata, 1},
	{Mouse_wheel_Updata, 5},
    {LED_Task, 100}
};

/******************************主函数中调用*********************************/
int main()
{
    Task_Init();
    
    while(1)
    {
        Task_Run();
    }
}
```

网上有很多类似实现方法，也有原理的讲解，如：

[时间片轮询的任务调度方法（一）](https://blog.csdn.net/mirco_mcu/article/details/114157274)

### 2. 驱动PAW3395

PAW3395使用spi协议驱动，按照数据手册编写驱动程序（中文手册以放在开源文件夹中）

![image-20240905210513362](https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409052105519.png)

![image-20240905210553661](https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409052105739.png)

### 3. HID协议

使用STM32CbueMX自动生成初始HID协议：

勾选使用USB

![image-20240905211800127](https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409052118237.png)

选择Device Class(HID)后其他选项全部默认

![image-20240905212000765](https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409052120879.png)

由于STM32CubeMX配置HID协议时，只能配默认值，以下为在KEIL中需更改的自定义代码

```c
/*
 *更改HID_FS_BINTERVAL轮询时间，每1ms更新一次
 */

//更改usbd_config.h文件：

//原：
#define HID_FS_BINTERVAL     0xA			//10ms

//改为：
#define HID_FS_BINTERVAL     0x01			//1ms



/*
 *更改usbd_hid鼠标报文，使鼠标可以发送最大6字节数据，保证x,y轴移动数据不丢包
 *
 *全部复制粘贴
 */

更改usbd_hid.h文件：

#define HID_EPIN_ADDR                 0x81U
#define HID_EPIN_SIZE                 0x06U			//最大数据为6Byte

#define USB_HID_CONFIG_DESC_SIZ       34U
#define USB_HID_DESC_SIZ              9U
#define HID_MOUSE_REPORT_DESC_SIZE    69U

#define HID_DESCRIPTOR_TYPE           0x21U
#define HID_REPORT_DESC               0x22U

#ifndef HID_HS_BINTERVAL
#define HID_HS_BINTERVAL            0x04U
#endif /* HID_HS_BINTERVAL */

#ifndef HID_FS_BINTERVAL
#define HID_FS_BINTERVAL            0x01U
#endif /* HID_FS_BINTERVAL */

#define HID_REQ_SET_PROTOCOL          0x0BU
#define HID_REQ_GET_PROTOCOL          0x03U

#define HID_REQ_SET_IDLE              0x0AU
#define HID_REQ_GET_IDLE              0x02U

#define HID_REQ_SET_REPORT            0x09U
#define HID_REQ_GET_REPORT            0x01U



// 更改usbd_hid.c文件：
__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE]  __ALIGN_END =
{
    0x05, 0x01,     
    0x09, 0x02,     
    0xA1, 0x01,     
    0x05, 0x09,     
    0x19, 0x01,     
    0x29, 0x05,     
    0x15, 0x00,     
    0x25, 0x01,     
    0x95, 0x05,     
    0x75, 0x01,     
    0x81, 0x02,     
    0x95, 0x01,     
    0x75, 0x03,     
    0x81, 0x03,     

    0x05, 0x01,     
    0x09, 0x30,     
    0x09, 0x31,     
    0x16, 0x00, 0x80,
    0x26, 0xFF, 0x7F,
    0x36, 0x00, 0x80,
    0x46, 0xFF, 0x7F,
    0x75, 0x10,     
    0x95, 0x02,     
    0x81, 0x06,     

    0x09, 0x38,     
    0x15, 0x81,     
    0x25, 0x7F,     
    0x35, 0x81,     
    0x45, 0x7F,     
    0x75, 0x08,     
    0x95, 0x01,     
    0x81, 0x06,     

    0xC0            
};
```

具体为什么这样做就不在赘述了，想继续了解的可以百度“HID协议”

### 4. 鼠标信息结构体及报文发送

鼠标结构体

```c
struct mouseHID_t
{
	/*鼠标按键：
	 *	bit0: 左键	
	 *	bit1: 右键 
	 *	bit2: 中键
	 *	bit3: 自定义
	 *	...
	 *	bit7: 自定义
	 */
	
	uint8_t button_left:1;      //左键
	uint8_t button_right:1;     //右键
	uint8_t button_middel:1;    //中建
	
	int16_t x;                  //x轴相对位移
	int16_t y;                  //y轴相对位移
	
	int8_t wheel;               //滚轮位移大小
};
```

*注：声明顺序不能变，HID协议中已经规定好了*

自定义HID报文发送函数

```c
//更新鼠标报文
void myMouse_update(struct mouseHID_t* mouseHID)
{		
	mouseHID->button_left = Left_Key_Value;
	mouseHID->button_right = Right_Key_Value;
	mouseHID->button_middel = Middel_Key_Value;
	
	mouseHID->x = X_Speed;
	mouseHID->y = Y_Speed;	
	
	mouseHID->wheel = wheel_num;
	
    // stm32 hal库的HID发送函数
	USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t*)mouseHID,sizeof(struct mouseHID_t));
	
	wheel_num = 0x80;// 滚轮数据清零
}
```

最后，声明一个HID报文发送任务，每1ms更新一次位移数据并向电脑发送报文

![image-20240906002114286](https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409060021323.png)

```c
//更新移动数据，并向电脑发送报文
void Mouse_XY_Updata(void)
{
	Motion_Burst(motion_burst_data);   //读取PAW3395传回来的X和Y速度
	
	X_Speed = (int16_t)(motion_burst_data[2] + (motion_burst_data[3] << 8));
	Y_Speed = (int16_t)(motion_burst_data[4] + (motion_burst_data[5] << 8));
	
	myMouse_update(&mouseHID);
}
```

### 5. 滚轮数据更新

在CubeMX中使用TIM2的编码器模式，默认配置

![image-20240905221916241](https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409052219394.png)

在HID协议中，滚轮数据为0x80时代表滚轮未移动，0xff为向上移动一个单位，0x01为向下移动一个单位；而其他数值为向上或向下移动好几个单位，移动距离太大，我们把握不住。

所以我的解决方法是只要判断滚轮移动就以一个单位处理，及0xff或0x01；其他情况为滚轮未移动，及0x80。但如果滚了一次后编码器就会长时间保持一个值造成重复判断，一直向上/下滚，这就引出来一个新问题：什么时候将编码器的值清零？

很简单，判断完就清零，将判断出来的数值暂存在 wheel_num 中，通过上面刚自定义的报文发送函数 myMouse_update() 每1ms发送给电脑。当然这个编码器判断函数可以就放在 Mouse_XY_Updata() 中，但这样1ms做的事情就太多了，既要spi读鼠标位移值，又要发送HID报文给电脑，还要判断滚轮数据，容易时序紊乱。所以最终我测试出每5ms判断一次还是可以的，最终代码如下：

![image-20240905225901269](https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409052259315.png)

```c
//更新滚轮数据
void Mouse_wheel_Updata(void)
{
	if((int16_t)__HAL_TIM_GET_COUNTER(&htim2) > 0)// 返回16位数据，如果需要负值要强制数据类型转换
		wheel_num = 0xFF;   
	else if((int16_t)__HAL_TIM_GET_COUNTER(&htim2) < 0)
		wheel_num = 0x01;
	else
		wheel_num = 0x80;
	
	//清除编码器计数
	TIM2->CNT=0;  // x表示第几个定时器，例如TIM8->CNT=0;
}
```

### 6. 按键任务

按键对应引脚

| 名称       | 引脚 | 解释                                                         |
| ---------- | ---- | ------------------------------------------------------------ |
| DPI_Key    | PB12 | 理论最大DPI为26000，我设置最大为3000，初始为500，每次按下+500，大于3000时，DPI值变回500 |
| Middel_Key | PB14 | 鼠标中键，按下上下移动可快速滚动页面                         |
| Right_Key  | PB15 | 鼠标右键                                                     |
| Left_Key   | PB13 | 鼠标左键                                                     |

所有按键皆设置为上拉输入模式，低电平按下，减少元件，提高板子空间利用率

![image-20240905235246640](https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409052352682.png)

目前仅实现鼠标基本按键功能，侧键可自己添加，预留PH2.0接口，对应引脚 PB10、PB11

![image-20240905235024965](https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409052350002.png)

按键功能的实现不用纠结于长按短按双击这些问题，我们只需要告诉电脑什么时候时按下状态，什么时候是弹起状态即可，长按短按双击由电脑中的鼠标驱动程序判断即可，不需要我们操心

人如果连续按下按键，每次间隔的时间大约为20ms，因此按键任务我们设置为每20ms执行一次

![image-20240906001913732](https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409060019776.png)

```c
uint8_t Key_Read(void)
{
	uint8_t Key_Value = 0;
	
	if(HAL_GPIO_ReadPin(GPIOB, Left_Key_Pin) == 0)
		Key_Value = Left_Key;
	else if(HAL_GPIO_ReadPin(GPIOB, Right_Key_Pin) == 0)
		Key_Value = Right_Key;
	else if(HAL_GPIO_ReadPin(GPIOB, Middel_Key_Pin) == 0)
		Key_Value = Middel_Key;
	else if(HAL_GPIO_ReadPin(GPIOB, DPI_Key_Pin) == 0)
		Key_Value = DPI_Key;
	
	return Key_Value;
}	

//按键任务
void Key_Task(void)
{
    // 使用位操作将按键的上升沿和下降沿抽离出来，并对应到具体的键值
	Key_Value = Key_Read();
	Key_Down = Key_Value&(Key_Old^Key_Value);
	Key_UP = ~Key_Value&(Key_Old^Key_Value);
	Key_Old = Key_Value;
	
    //按键下降沿（按下瞬间）
	switch(Key_Down)
	{
		case Left_Key:
			Left_Key_Value = 1;
		break;
		
		case Right_Key:
			Right_Key_Value = 1;
		break;
		
		case Middel_Key:
			Middel_Key_Value = 1;
		break;
	}
	
    //按键上升沿（弹起瞬间）
	switch(Key_UP)
	{
		case Left_Key:
			Left_Key_Value = 0;
		break;
		
		case Right_Key:
			Right_Key_Value = 0;
		break;
		
		case Middel_Key:
			Middel_Key_Value = 0;
		break;
	}
	
    //DPI按键按下
	if(Key_Down == DPI_Key)
	{
		DPI += 500;
		if(DPI > 3000)
		{
			DPI = 500;
		}
		DPI_Config(DPI);
        led_flag = 1;
        Key_cnt++;
	}
}
```

### 7. LED任务

DPI改变时LED间隔500ms闪烁3次，但如果在闪烁期间DPI按键再次按下应该怎么办呢，我的做法是待本次3下的闪烁完成后再进行3次闪烁，若按下两次，则待本次3下的闪烁完成后再进行6次闪烁，以此类推

LED任务的时间需求不是很大，所以我将轮询时间设为100ms。上述功能的实现方法有好多种，我的不一定是最好的，仅供参考

![image-20240906004517022](https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409060045060.png)

```c
/******************************定时器3中断回调函数*********************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *hitm)
{
	uint8_t i;
	
    // 系统总运行时间，可连续计时49.7天
	SYS_tick_ms++;
	
	for(i = 0; i < TASKNUM_MAX; i++){
		if(TaskTimer[i])
			TaskTimer[i]--;
	}	
}

//按键任务
void Key_Task(void)
{
    ...
    ...
    ...
    
    //DPI按键按下
	if(Key_Down == DPI_Key)
	{
		DPI += 500;
		if(DPI > 3000)
		{
			DPI = 500;
		}
		DPI_Config(DPI);
        led_flag = 1;
        Key_cnt++;
	}
}

//LED任务，DPI改变时LED闪烁3次
//若闪烁期间DPI按键再次按下，则本次3次的闪烁完成后再进行3次闪烁，以此类推
void LED_Task(void)
{
    static uint32_t LED_tick_ms;
    static uint8_t blink_cnt;
    
    if(led_flag == 1)
    {
        if(SYS_tick_ms - LED_tick_ms >= 500)
        {
            LED_tick_ms = SYS_tick_ms;
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            blink_cnt++;
            if(blink_cnt == 6)
            {
                Key_cnt--;
                blink_cnt = 0;
                HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
            }
            if(Key_cnt == 0)
            {
                led_flag = 0;
            }
        }
    }   
}
```

## 四、硬件实现

硬件比较简单，本质就是stm32最小系统板 + PAW3395驱动电路 + 按键和滚轮电路，还有就是PCB和3D外壳的适配，文件工程里都有。具体BOM表及PCB、3D外壳详细信息请移步立创开源广场：

这里先放出原理图及PCB图片以供参考：

![image-20240906010017509](https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409060100576.png)



![image-20240906010219428](https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/image-20240906010219428.png)



## 五、开发过程中遇到的问题及解决方法

### 问题1：PAW3395驱动失败

找到PAW3395数据手册后，先尝试自己驱动，由于是英文而且篇幅很长，一直没有什么进展。后来在github找到一位大佬的开源驱动，并找到中文翻译的数据手册，成功驱动

大佬github主页：[Ghost-Girls (Jao) (github.com)](https://github.com/Ghost-Girls/)

### 问题2：HID报文格式

由于STM32CubeMX只能生成基本信息，详细配置需要自己修改。作为第一次使用HID协议的纯小白，我只得上网了解，最后也是勉强改出来了，usb协议这东西有多有杂，我真的不是很擅长...

这里推荐一个讲的比较清楚的博客：[USB鼠标HID报告描述符数据格式分析 - USB中文网 (usbzh.com)](https://www.usbzh.com/article/detail-327.html)

## 六、总结

鼠标看着容易，其实实现起来也挺复杂的，当然目前的效果仍存在一些缺陷：

1. PCB上晶振和芯片之间GND过孔和3.3V打得太近了，导致那里有一点干扰就容易短路，这也是最开头的图中将晶振和芯片之间用热熔胶打上的原因，就是为了防止信号干扰

	<img src="https://picture-note-1328988318.cos.ap-nanjing.myqcloud.com/Typora/202409061554906.png" alt="image-20240906155423745" style="zoom:50%;" />

2. HID协议研究的模棱两可，有些参数不是很理解，仅是把功能实现了

3. 总体架构不是很合理，1ms的时间既要spi读取鼠标位移数据又要发送报文，容易造成运行时间超出预设触发时间，实时性不够好

4. 3D打印外壳用的B站up主**[一浅垅一](https://space.bilibili.com/28885190)**的开源文件，但我PCB测量的尺寸对应的不是很好，各个键位能对应的上，就是有点丑...

总之，虽然缺点有不少，但好歹是能用的，我自己使用了两个星期，并用它打了2024年的电赛，除了手感有点拉跨，其他各功能都没什么问题，稳定性也不错，中途没有突然死机罢工

之后有空的话，我将对它进行一些优化：

1. 首先，外壳我准备换成厂家生产的现成鼠标外壳，虽然3D打印外壳一开始用会很新奇，但用久了手感还是不太好
2. 传感器准备换一种，虽然paw3395性能很强，但价格有点贵，一个要50人民币
3. HID协议准备换成南京沁恒的HID转串口芯片CH9328，这样就不用研究复杂的HID了，只需串口发送对应的命令即可实现HID的效果
4. 准备将裸机换成RT-Thread，以提高系统的稳定性及实时性

















