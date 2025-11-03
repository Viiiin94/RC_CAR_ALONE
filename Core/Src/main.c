/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "stdbool.h"
#include "string.h"
#include "rc_control.h"
#include "ultrasonic.h"
#include "delay_us.h"
#include "stdio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define PKT_LEN   8
#define SOF       0xFF

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

int _write(int file, unsigned char* p, int len)
{
	HAL_StatusTypeDef status = HAL_UART_Transmit(&huart2, p, len, 100);
	return (status == HAL_OK ? len : 0);
}

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

// 조이스틱 버퍼
static uint8_t rx1, rx2;
static uint8_t pkt[PKT_LEN];  // 프레임 버퍼
static uint8_t pidx = 0;      // 수신 인덱스

// 초음파센서 값
uint16_t IC_Value1[3] = {0};  // RISING EDGE 캡처 값
uint16_t IC_Value2[3] = {0};  // FALLING EDGE 캡처 값
uint16_t echoTime[3] = {0};   // 펄스 길이 (µs)
uint8_t captureFlag[3] = {0}; // 캡처 플래그 (0: RISING 대기, 1: FALLING 대기)
uint8_t distance[3] = {0};    // 최종 거리 (cm)

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart1)
	{
		uint8_t c = rx1;

		if (pidx == 0)
		{
			if (c != SOF)
			{
				HAL_UART_Receive_IT(&huart1, &rx1, 1);
				return;
			}
			pkt[pidx++] = c;
		}
		else
		{
			pkt[pidx++] = c;
			if (pidx == PKT_LEN)
			{
				uint16_t buttons = (uint16_t)pkt[6] | ((uint16_t)pkt[7] << 8); // LE
				onPressJoyStickKey(buttons);
				pidx = 0;
			}
		}
		HAL_UART_Receive_IT(&huart1, &rx1, 1);  // 재등록
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	// TIM1에서 인터럽트가 발생했을 때만 처리
	if(htim->Instance == TIM1)
	{
		int sensor_idx = -1;
		uint32_t TIM_Channel = 0;

		// 발생 채널에 따라 인덱스 결정
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			sensor_idx = 0;
			TIM_Channel = TIM_CHANNEL_1;
		}
		else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			sensor_idx = 1;
			TIM_Channel = TIM_CHANNEL_2;
		}
		else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
		{
			sensor_idx = 2;
			TIM_Channel = TIM_CHANNEL_3;
		}

		if (sensor_idx != -1)
		{
			if(captureFlag[sensor_idx] == 0) // 첫 번째 캡처 (RISING EDGE)
			{
				IC_Value1[sensor_idx] = HAL_TIM_ReadCapturedValue(htim, TIM_Channel);
				captureFlag[sensor_idx] = 1;

				// 다음 캡처를 위해 폴링을 FALLING EDGE로 변경
				__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_Channel, TIM_INPUTCHANNELPOLARITY_FALLING);
			}
			else if(captureFlag[sensor_idx] == 1) // 두 번째 캡처 (FALLING EDGE)
			{
				IC_Value2[sensor_idx] = HAL_TIM_ReadCapturedValue(htim, TIM_Channel);

				// 펄스 길이 계산 (오버플로우 처리 포함)
				if(IC_Value2[sensor_idx] > IC_Value1[sensor_idx])
				{
					echoTime[sensor_idx] = IC_Value2[sensor_idx] - IC_Value1[sensor_idx];
				}
				else
				{
					echoTime[sensor_idx] = (0xffff - IC_Value1[sensor_idx]) + IC_Value2[sensor_idx];
				}

				// 거리 계산 (cm)
				distance[sensor_idx] = echoTime[sensor_idx] / 58;
				captureFlag[sensor_idx] = 0; // 플래그 초기화

				// 다음 측정을 위해 폴링을 RISING EDGE로 복원
				__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_Channel, TIM_INPUTCHANNELPOLARITY_RISING);

				// ★★★ 핵심 수정: 측정이 완료되면 해당 채널의 캡처 인터럽트 비활성화 ★★★
				if (sensor_idx == 0) {
					__HAL_TIM_DISABLE_IT(htim, TIM_IT_CC1);
				} else if (sensor_idx == 1) {
					__HAL_TIM_DISABLE_IT(htim, TIM_IT_CC2);
				} else if (sensor_idx == 2) {
					__HAL_TIM_DISABLE_IT(htim, TIM_IT_CC3);
				}
			}
		}
	}
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
	MX_TIM3_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_TIM11_Init();
	MX_TIM1_Init();
	MX_USART6_UART_Init();
	/* USER CODE BEGIN 2 */

	// 바퀴 제어
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

	// 초음파 센서 제어
	HAL_TIM_Base_Start(&htim11);
	HAL_TIM_Base_Start(&htim1);

	HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);

	// 추후에 할 블루투스 모듈연결 AT command
	HAL_UART_Receive_IT(&huart1, &rx1, sizeof(rx1));
	HAL_UART_Receive_IT(&huart2, &rx2, sizeof(rx2));

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		getUltraSonicTrigger();
		HAL_Delay(60);

		uint8_t forward_distance = distance[SENSOR_FORWARD];

		if(forward_distance < 30 && forward_distance > 0)
		{
			// 너무 가까우면 정지
			stopMove();
			printf("[STOP]\n");
		}
		else if(forward_distance <= 100 && forward_distance > 0)
		{
			// 100cm 이하면 천천히 전진
			setSpeed(SPEED_SLOW);
//			moveForward();  // 👈 여기!
			printf("[SLOW FORWARD]\n");
		}
		else
		{
			// 100cm 초과면 빠르게 전진
			setSpeed(SPEED_NORMAL);
//			moveForward();  // 👈 여기!
			printf("[NORMAL FORWARD]\n");
		}

		printf("\n");
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

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 100;
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

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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
#ifdef USE_FULL_ASSERT
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
