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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//////////////////////////////////////////////////////  步进电机驱动  /////////////////////////////////////////////////////////

// 定义步进电机的引脚
#define IN1_PIN_1 GPIO_PIN_5
#define IN2_PIN_1 GPIO_PIN_6
#define IN3_PIN_1 GPIO_PIN_7
#define IN4_PIN_1 GPIO_PIN_4
#define IN1_PORT_1 GPIOA
#define IN2_PORT_1 GPIOA
#define IN3_PORT_1 GPIOA
#define IN4_PORT_1 GPIOC

#define IN1_PIN_2 GPIO_PIN_5
#define IN2_PIN_2 GPIO_PIN_0
#define IN3_PIN_2 GPIO_PIN_1
#define IN4_PIN_2 GPIO_PIN_2
#define IN1_PORT_2 GPIOC
#define IN2_PORT_2 GPIOB
#define IN3_PORT_2 GPIOB
#define IN4_PORT_2 GPIOB

// 步进电机的步骤序列
const uint8_t step_sequence[8] = {0x1,0x2,0x4,0x8,0x1,0x2,0x4,0x8};

// 步进电机正转函数
void motor_forward(int motor, uint32_t delay_ms) {
	if(motor == 0){
    for (int i = 0; i < 8; i++) {
        if (step_sequence[i] & 0x01) HAL_GPIO_WritePin(IN1_PORT_1, IN1_PIN_1, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN1_PORT_1, IN1_PIN_1, GPIO_PIN_RESET);
        
			  if (step_sequence[i] & 0x02) HAL_GPIO_WritePin(IN2_PORT_1, IN2_PIN_1, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN2_PORT_1, IN2_PIN_1, GPIO_PIN_RESET);
        
			  if (step_sequence[i] & 0x04) HAL_GPIO_WritePin(IN3_PORT_1, IN3_PIN_1, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN3_PORT_1, IN3_PIN_1, GPIO_PIN_RESET);
        
			  if (step_sequence[i] & 0x08) HAL_GPIO_WritePin(IN4_PORT_1, IN4_PIN_1, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN4_PORT_1, IN4_PIN_1, GPIO_PIN_RESET);
        
			  HAL_Delay(delay_ms);
    }
	}
	else if(motor == 1){
		for (int i = 0; i < 8; i++) {
        if (step_sequence[i] & 0x01) HAL_GPIO_WritePin(IN1_PORT_2, IN1_PIN_2, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN1_PORT_2, IN1_PIN_2, GPIO_PIN_RESET);
        
			  if (step_sequence[i] & 0x02) HAL_GPIO_WritePin(IN2_PORT_2, IN2_PIN_2, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN2_PORT_2, IN2_PIN_2, GPIO_PIN_RESET);
        
			  if (step_sequence[i] & 0x04) HAL_GPIO_WritePin(IN3_PORT_2, IN3_PIN_2, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN3_PORT_2, IN3_PIN_2, GPIO_PIN_RESET);
        
			  if (step_sequence[i] & 0x08) HAL_GPIO_WritePin(IN4_PORT_2, IN4_PIN_2, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN4_PORT_2, IN4_PIN_2, GPIO_PIN_RESET);
        
			  HAL_Delay(delay_ms);
    }
	}
}

// 步进电机反转函数
void motor_backward(int motor, uint32_t delay_ms) {
	if(motor == 0){
    for (int i = 7; i >= 0; i--) {
        if (step_sequence[i] & 0x01) HAL_GPIO_WritePin(IN1_PORT_1, IN1_PIN_1, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN1_PORT_1, IN1_PIN_1, GPIO_PIN_RESET);

        if (step_sequence[i] & 0x02) HAL_GPIO_WritePin(IN2_PORT_1, IN2_PIN_1, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN2_PORT_1, IN2_PIN_1, GPIO_PIN_RESET);

        if (step_sequence[i] & 0x04) HAL_GPIO_WritePin(IN3_PORT_1, IN3_PIN_1, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN3_PORT_1, IN3_PIN_1, GPIO_PIN_RESET);

        if (step_sequence[i] & 0x08) HAL_GPIO_WritePin(IN4_PORT_1, IN4_PIN_1, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN4_PORT_1, IN4_PIN_1, GPIO_PIN_RESET);

        HAL_Delay(delay_ms);
    }
	}
	else if(motor == 1){
		for (int i = 7; i >= 0; i--) {
        if (step_sequence[i] & 0x01) HAL_GPIO_WritePin(IN1_PORT_2, IN1_PIN_2, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN1_PORT_2, IN1_PIN_2, GPIO_PIN_RESET);

        if (step_sequence[i] & 0x02) HAL_GPIO_WritePin(IN2_PORT_2, IN2_PIN_2, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN2_PORT_2, IN2_PIN_2, GPIO_PIN_RESET);

        if (step_sequence[i] & 0x04) HAL_GPIO_WritePin(IN3_PORT_2, IN3_PIN_2, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN3_PORT_2, IN3_PIN_2, GPIO_PIN_RESET);

        if (step_sequence[i] & 0x08) HAL_GPIO_WritePin(IN4_PORT_2, IN4_PIN_2, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(IN4_PORT_2, IN4_PIN_2, GPIO_PIN_RESET);

        HAL_Delay(delay_ms);
    }
	}
}

///////////////////////////////////////////////////////  PCA9685 驱动  /////////////////////////////////////////////////////////////////

#ifndef _PCA9685_H
#define _PCA9685_H
 
void PCA9685_Init();
 
// channel: 0 ~ 15
//      on: 0 ~ 4095
//     off: 0 ~ 4095
void PCA9685_SetChannelDuty(uint8_t channel, uint32_t on, uint32_t off);
 
#endif


#define PCA9685_I2C_ADDR 0x40
 
#define MODE1       0x00
#define MODE2       0x01
#define SUBADR1     0x02
#define SUBADR2     0x03
#define SUBADR3     0x04
#define ALLCALLADR  0x05
#define LED0_ON_L   0x06
#define LED0_ON_H   0x07
#define LED0_OFF_L  0x08
#define LED0_OFF_H  0x09
#define LED1_ON_L   0x0A
#define LED1_ON_H   0x0B
#define LED1_OFF_L  0x0C
#define LED1_OFF_H  0x0D
#define LED2_ON_L   0x0E
#define LED2_ON_H   0x0F
#define LED2_OFF_L  0x10
#define LED2_OFF_H  0x11
#define LED3_ON_L   0x12
#define LED3_ON_H   0x13
#define LED3_OFF_L  0x14
#define LED3_OFF_H  0x15
#define LED4_ON_L   0x16
#define LED4_ON_H   0x17
#define LED4_OFF_L  0x18
#define LED4_OFF_H  0x19
#define LED5_ON_L   0x1A
#define LED5_ON_H   0x1B
#define LED5_OFF_L  0x1C
#define LED5_OFF_H  0x1D
#define LED6_ON_L   0x1E
#define LED6_ON_H   0x1F
#define LED6_OFF_L  0x20
#define LED6_OFF_H  0x21
#define LED7_ON_L   0x22
#define LED7_ON_H   0x23
#define LED7_OFF_L  0x24
#define LED7_OFF_H  0x25
#define LED8_ON_L   0x26
#define LED8_ON_H   0x27
#define LED8_OFF_L  0x28
#define LED8_OFF_H  0x29
#define LED9_ON_L   0x2A
#define LED9_ON_H   0x2B
#define LED9_OFF_L  0x2C
#define LED9_OFF_H  0x2D
#define LED10_ON_L  0x2E
#define LED10_ON_H  0x2F
#define LED10_OFF_L 0x30
#define LED10_OFF_H 0x31
#define LED11_ON_L  0x32
#define LED11_ON_H  0x33
#define LED11_OFF_L 0x34
#define LED11_OFF_H 0x35
#define LED12_ON_L  0x36
#define LED12_ON_H  0x37
#define LED12_OFF_L 0x38
#define LED12_OFF_H 0x39
#define LED13_ON_L  0x3A
#define LED13_ON_H  0x3B
#define LED13_OFF_L 0x3C
#define LED13_OFF_H 0x3D
#define LED14_ON_L  0x3E
#define LED14_ON_H  0x3F
#define LED14_OFF_L 0x40
#define LED14_OFF_H 0x41
#define LED15_ON_L  0x42
#define LED15_ON_H  0x43
#define LED15_OFF_L 0x44
#define LED15_OFF_H 0x45
 
#define PRE_SCALE  0xFE
 
#define MODE1_RESTART 0x80
#define MODE1_EXTCLK  0x40
#define MODE1_AI      0x20
#define MODE1_SLEEP   0x10
#define MODE1_ALLCALL 0x01
 

void PCA9685_WriteReg(uint8_t addr, uint8_t value) {	
	uint8_t data[2] = {addr, value};
	HAL_I2C_Master_Transmit(&hi2c1, (PCA9685_I2C_ADDR<<1)&0xfe, data, 2, HAL_MAX_DELAY);
}
 
void PCA9685_ReadReg(uint8_t addr, uint8_t *value) {
	HAL_I2C_Master_Transmit(&hi2c1, (PCA9685_I2C_ADDR<<1)&0xfe, &addr, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&hi2c1, (PCA9685_I2C_ADDR<<1)|0x01, value, 1, HAL_MAX_DELAY);
}

void PCA9685_SetChannelDuty(uint8_t channel, uint32_t on, uint32_t off) {
    uint8_t data[5];
    
    data[0] = LED0_ON_L + 4*channel;
    data[1] = on & 0xff;
    data[2] = (on >> 8) & 0xff;
    data[3] = off & 0xff;
    data[4] = (off >> 8) & 0xff;
    
    HAL_I2C_Master_Transmit(&hi2c1, (PCA9685_I2C_ADDR << 1) & 0xfe, data, 5, HAL_MAX_DELAY);
		HAL_Delay(100);
}
 
void PCA9685_Init() {
	uint8_t value;
	
	PCA9685_WriteReg(MODE1, MODE1_SLEEP);
	PCA9685_WriteReg(PRE_SCALE, 121);
	PCA9685_WriteReg(MODE1, 0);
	PCA9685_WriteReg(MODE1, MODE1_RESTART | MODE1_AI | MODE1_ALLCALL);
 
	while (1) {
		PCA9685_ReadReg(MODE1, &value);
		if ((value & MODE1_RESTART)==0) {
			break;
		}
		HAL_Delay(100);
	}
}


////////////////////////////////////////////////////////////////////  SG90 驱动  ////////////////////////////////////////////////////

#ifndef _SG90_H
#define _SG90_H
 
#define TIMER htim1

void SG90_Init();
void SG90_SetAngle(uint8_t id, uint8_t angle);
void SG90_TurnToDegree(int angle);
 
#endif

void wError_Handler(void)
{
	// 错误处理代码
	while(1){
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7);
		HAL_Delay(500);
	}
}

// 管脚是 PA0
void SG90_TurnToDegree(int angle){
	unsigned dutyCycle = 0;
	dutyCycle = 1000 + (unsigned)angle * 1000 / 180;
  
  // 设置PWM的占空比
  TIM_OC_InitTypeDef sConfigOC = {0};
	
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = dutyCycle;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&TIMER, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    wError_Handler();
  }
	
  HAL_TIM_PWM_Start(&TIMER, TIM_CHANNEL_1);
}

void SG90_Init() {
	PCA9685_Init();
}
 
void SG90_SetAngle(uint8_t id, uint8_t angle) {
	//if(id != 0x00){
		PCA9685_SetChannelDuty(id, 0, (uint32_t)(4095*((angle/180.0)*2.0+0.5)/20.0));
	/*}
	else{
		int a = angle;
		char str[50];
		
		sprintf(str, "Angle: %d\r\n", a);
		printf("%s", str);
		
		SG90_TurnToDegree(a);
	}*/
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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	
	
	HAL_TIM_Base_Start(&TIMER);
	
	SG90_Init();
	
	int i = 0;
	
	// 初始旋转角度调试
	SG90_SetAngle(0, 0);
	SG90_SetAngle(1, 180);
	
	// 信息接收符号
	uint8_t psuc = 0x2a, pfal = 0x54, dsuc = 0xff, dfal = 0x00;
	uint8_t *pdata;
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	#define COUNT 7
	
	// 舵机编号队列
	uint8_t motor_id[COUNT] = {4, 5, 6, 0, 1, 7, 8};
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// 舵机的旋转角度和舵机的 STEP 计数值
	int32_t sg90s_angel_byjs_step[COUNT];
	memset(sg90s_angel_byjs_step, 0, COUNT);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	/*
	 * 测试 CODE:
	 * 
	 * 01 04 01 00 06
   * 00 02 01 5A
	 */
  while (1)
  {
		uint8_t prepare[5];
		memset(prepare, 0, 5);
		
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		HAL_UART_AbortReceive(&huart1);
		__HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_RXNE);


		// 接收报告描述符头
		HAL_UART_Receive(&huart1, prepare, 5, HAL_MAX_DELAY);
		
		// 进行校验
		uint16_t crc = (((uint16_t)(prepare[3])) << 8) + prepare[4];
		
		// 校验成功
		if((uint16_t)(prepare[0]) + prepare[1] + prepare[2] == crc){
			// 返回成功接收
			HAL_UART_Transmit(&huart1, &psuc, 1, HAL_MAX_DELAY);
			
			// 开始分配必要内存
			pdata = (uint8_t*)malloc((unsigned)prepare[1] * sizeof(uint8_t));
			memset(pdata, 0, prepare[1]);
			
			// 接收 16 位数据
			HAL_UART_Receive(&huart1, pdata, (uint16_t)prepare[1], HAL_MAX_DELAY);
			// 关闭 USART1
			HAL_UART_DeInit(&huart1);
			
			// 依次解析包内容
			uint8_t i = 0;
			for(; i < prepare[0]; ++i){
				// 如果要驱动的是步进电机
				if(pdata[4 * i] == 0x00){
					uint8_t id = motor_id[pdata[4 * i + 1]];
					
					// 绝对值
					if(pdata[4 * i + 2] == 0x00){
						int32_t current_pos = sg90s_angel_byjs_step[pdata[4 * i + 1]];
						
						// 如果电机位置在正向
						if (current_pos < (int32_t)pdata[4 * i + 3]){
							// 需要逆向旋转的度数
							int32_t pos = (int32_t)pdata[4 * i + 3] - current_pos, i = 0;
							
							for(; i < pos; ++i){
								motor_backward(id, 2);
							}
						}
						else if (current_pos > (int32_t)pdata[4 * i + 3]){
							// 需要正向旋转的度数
							int32_t pos = current_pos - (int32_t)pdata[4 * i + 3], i = 0;
							
							for(; i < pos; ++i){
								motor_forward(id, 2);
							}
						}
						else;
						
						// 统计步进电机的 STEP
						sg90s_angel_byjs_step[pdata[4 * i + 1]] = pdata[4 * i + 3];
					}
					// 相对值, 以0x80为界, 高的反转, 低的正转, 0x80正好是180度
					else if(pdata[4 * i + 2] == 0x01){
						// 逆时针旋转
						if(pdata[4 * i + 3] < 0x80){
							// 旋转 STEP
							int8_t angel = 0x80 - pdata[4 * i + 3], j = 0;
							
							for (; j < angel; ++j){
								motor_forward(id, 2);
							}
							// 统计步进电机的 STEP
							sg90s_angel_byjs_step[pdata[4 * i + 1]] -= angel;
						}
						else if(pdata[4 * i + 3] > 0x80){
							// 旋转 STEP, 0x00不能使用, 范围为: 0x01 到 0xff
							int8_t angel = pdata[4 * i + 3] - 0x80, j = 0;
							
							for (; j < angel; ++j){
								motor_backward(id, 2);
							}
							// 统计步进电机的 STEP
							sg90s_angel_byjs_step[pdata[4 * i + 1]] += angel;
						}
						else;
					}
					else{
						++dfal;
						break;
					}
				}
				// 如果要驱动的是舵机
				else if(pdata[4 * i] == 0x01){
					uint8_t id = motor_id[pdata[4 * i + 1]];
					
					// 绝对值
					if(pdata[4 * i + 2] == 0x00){
						// 旋转 STEP, 范围 0x00 到 0xb4
						int8_t angel = pdata[4 * i + 3];
							
						SG90_SetAngle(id, angel);
						sg90s_angel_byjs_step[pdata[4 * i + 1]] = angel;
					}
					// 相对值, 以0x80为界, 高的反转, 低的正转
					else if(pdata[4 * i + 2] == 0x01){
						// 如果要逆时针旋转
						if(pdata[4 * i + 1] < 0x80){
							int8_t angel = 0x80 - pdata[4 * i + 3];
							
							// 如果逆时针越界
							if(angel > sg90s_angel_byjs_step[pdata[4 * i + 1]]){
								SG90_SetAngle(id, 0);
								sg90s_angel_byjs_step[pdata[4 * i + 1]] = 0;
							}
							else{
								// 先类型转换, 再进行赋值
								sg90s_angel_byjs_step[pdata[4 * i + 1]] -= angel;
								SG90_SetAngle(id, (int)sg90s_angel_byjs_step[pdata[4 * i + 1]]);
							}
						}
						// 如果要顺时针旋转
						else if(pdata[4 * i + 1] > 0x80){
							int8_t angel = pdata[4 * i + 3] - 0x80;
							
							// 如果逆时针越界
							if(angel > 0x0100 - (uint16_t)sg90s_angel_byjs_step[pdata[4 * i + 1]]){
								SG90_SetAngle(id, 180);
								// 这是 int32_t 不需要考虑容量
								sg90s_angel_byjs_step[pdata[4 * i + 1]] = 0xb4;
							}
							else{
								// 先类型转换, 再进行赋值
								sg90s_angel_byjs_step[pdata[4 * i + 1]] -= angel;
								SG90_SetAngle(id, (int)sg90s_angel_byjs_step[pdata[4 * i + 1]]);
							}
						}
						else;
					}
					else{
						++dfal;
						break;
					}
				}
				// 如果出现了字节错误
				else{
					++dfal;
				}
			}
			// 释放动态内存
			free(pdata);
			// 重启 USART1
			HAL_UART_Init(&huart1);
			// 检查错误
			if(dfal){
				memset(prepare, 0, 5);
				HAL_UART_Transmit(&huart1, &dfal, 1, HAL_MAX_DELAY);
				dfal = 0x00;
			}
			else{
				HAL_UART_Transmit(&huart1, &dsuc, 1, HAL_MAX_DELAY);
				memset(prepare, 0, 5);
			}
		}
		// 校验失败
		else{
			// memset(prepare, 0, 5);
			int i = 0;
			for (; i < 5; ++i){
				prepare[i] = 0x00;
			}
			
			dfal = 0x00;
			// 返回错误值
			HAL_UART_Transmit(&huart1, &pfal, 1, HAL_MAX_DELAY);
		}
		
		
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
