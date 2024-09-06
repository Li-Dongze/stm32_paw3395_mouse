/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_hid.h"
#include "delay.h"
#include "PAW3395.h"
#include "myMouse.h"
#include "OLED.h"
//extern USBD_HandleTypeDef hUsbDeviceFS;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#define TASKNUM_MAX	4

typedef struct{
	void (*pTask)(void);    //任务函数
	uint16_t TaskPeriod;    //多少毫秒调用一次任务函数
}TaskStruct;

typedef enum KEY{
	Left_Key = 1,
	Right_Key,
	Middel_Key,
	DPI_Key
}KEY;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint8_t Key_Read(void);

void Task_Init(void);
void Task_Run(void);

void Key_Task(void);
void Mouse_XY_Updata(void);
void Mouse_wheel_Updata(void);
void Show_Task(void);
void LED_Task(void);
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t Key_Value;
uint8_t Key_Down;
uint8_t Key_UP;
uint8_t Key_Old;

uint8_t Left_Key_Value = 0;
uint8_t Right_Key_Value = 0;
uint8_t Middel_Key_Value = 0;

int8_t wheel_num;

struct mouseHID_t mouseHID;
uint8_t motion_burst_data[12] = {0};
int16_t X_Speed,Y_Speed;
uint16_t DPI = 1500;    //初始化DPI初值

uint32_t SYS_tick_ms;

uint8_t led_flag = 0;
uint8_t Key_cnt = 0;

uint16_t TaskTimer[TASKNUM_MAX];
TaskStruct Task[] = {
	{Key_Task, 20},
	{Mouse_XY_Updata, 1},
	{Mouse_wheel_Updata, 5},
    {LED_Task, 100}
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/******************************定时器3中断回调函数*********************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *hitm)
{
	uint8_t i;
	
    // 系统总运行时间，可连续计时49.7天
	SYS_tick_ms++;
	
	//HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
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

/******************************具体任务函数*********************************/
//按键任务
void Key_Task(void)
{
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

//更新移动数据，并向电脑发送报文
void Mouse_XY_Updata(void)
{
	Motion_Burst(motion_burst_data);   //读取PAW3395传回来的X和Y速度
	
	X_Speed = (int16_t)(motion_burst_data[2] + (motion_burst_data[3] << 8));
	Y_Speed = (int16_t)(motion_burst_data[4] + (motion_burst_data[5] << 8));
	
	myMouse_update(&mouseHID);
}

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

//显示各种数据参数，方便调试
void Show_Task(void)
{
	OLED_Printf(0, 0, OLED_6X8, "X_Speed:%06d", X_Speed);
	OLED_Printf(0, 8, OLED_6X8, "Y_Speed:%06d", Y_Speed);
	OLED_Printf(0, 16, OLED_6X8, "wheel:%05d", (int16_t)__HAL_TIM_GET_COUNTER(&htim2));
	OLED_Printf(0, 24, OLED_6X8, "DPI:%d", DPI);
	
	OLED_Update();
}
/*********************************其他操作函数*****************************************/

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

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	delay_init(72); //	精确延时初始化
	
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
	
	//开启spi
	__HAL_SPI_ENABLE(&hspi1);
    
    //以任何顺序为VDD和VDDIO供电，每次供电之间的延迟不超过100ms。确保所有供应稳定。
    HAL_Delay(100);
    //初始化PAW3395
	Power_up_sequence();
	myMouse_init(&mouseHID);
	DPI_Config(DPI);
	
	__HAL_TIM_CLEAR_IT(&htim3,TIM_IT_UPDATE ); //清除IT标志位
	HAL_TIM_Base_Start_IT(&htim3);//启动时基
	
    //任务初始化
	Task_Init();
	
    //开启定时器2编码器
	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
	
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
        //启动任务调度器
		Task_Run();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
