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
#include "stdio.h"

#include "rc_control.h"
#include "ultrasonic.h"
#include "joystick_pairing.h"
#include "delay_us.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum {
	STATE_NORMAL,           // 정상 주행
	STATE_MEASURING,        // 센서 측정 중
	STATE_STOPPING,         // 정지 중
	STATE_TURNING           // 회전 중
} DriveState;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define PKT_LEN   8						// 아이폰 조이스틱 버튼 헥사코드 패킷
#define SOF       0xFF					// 첫 번째 패킷 값이 FF로 고정

#define SLOW_DISTANCE	 40
#define STOP_DISTANCE    35
#define TURN_DISTANCE    18

#define ULTRASONIC_DETECTING_TIME 60

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
static uint8_t rx1, rx2;	  	// rx1 USART1 / rx2 USART2
static uint8_t pkt[PKT_LEN];  	// 프레임 버퍼
static uint8_t pidx = 0;      	// 수신 인덱스

// 초음파센서 값
uint16_t IC_Value1[3] = {0};
uint16_t IC_Value2[3] = {0};
uint16_t echoTime[3] = {0};
uint8_t captureFlag[3] = {0};
uint8_t distance[3] = {0};

static uint32_t lastSensorTime = 0;
static uint32_t measureStartTime = 0;
static uint32_t stateStartTime = 0;
static DriveState currentState = STATE_NORMAL;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

// 아이폰 블루투스 감지된 값 처리 콜백함수
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
				uint16_t buttons = (uint16_t)pkt[6] | ((uint16_t)pkt[7] << 8); // 0x0001, 0x0002, 0x0004, 0x0008 << 이런식으로 나옴
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

				if(IC_Value2[sensor_idx] > IC_Value1[sensor_idx])
				{
					echoTime[sensor_idx] = IC_Value2[sensor_idx] - IC_Value1[sensor_idx];
				}
				else
				{
					echoTime[sensor_idx] = (0xffff - IC_Value1[sensor_idx]) + IC_Value2[sensor_idx];
				}

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

	// 초기 속도 설정
	setSpeed(400);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		uint32_t currentTime = HAL_GetTick();

		switch(currentState)
			{
				case STATE_NORMAL:
				{
					if(currentTime - lastSensorTime >= 60)
					{
						getUltraSonicTrigger();
						measureStartTime = currentTime;
						currentState = STATE_MEASURING;
					}

					// 주행 (이전 센서 값으로)
					uint8_t right = distance[SENSOR_RIGHT];
					uint8_t forward = distance[SENSOR_FORWARD];
					uint8_t left = distance[SENSOR_LEFT];

					if(forward < SLOW_DISTANCE && forward > 0)
					{
						setSpeed(400);

						if(forward < STOP_DISTANCE && forward > 0)
						{
							stopMove();
							currentState = STATE_STOPPING;
							stateStartTime = currentTime;
						}
					}
					else if(left < TURN_DISTANCE && left > 0)
					{
						turnRightForward();
					}
					else if(right < TURN_DISTANCE && right > 0)
					{
						turnLeftForward();
					}
					else
					{
						setSpeed(500);
						moveForward();
					}
					break;
				}

				case STATE_MEASURING:
				{
					// 60ms 후 측정 완료
					if(currentTime - measureStartTime >= 60)
					{
						uint8_t right = distance[SENSOR_RIGHT];
						uint8_t forward = distance[SENSOR_FORWARD];
						uint8_t left = distance[SENSOR_LEFT];

						printf("R:%3d | F:%3d | L:%3d\n", right, forward, left);

						lastSensorTime = currentTime;
						currentState = STATE_NORMAL;
					}

					// 측정 중에도 기존 동작 유지
					// (이전 센서 값으로 계속 주행)
					break;
				}

				case STATE_STOPPING:
				{
					uint32_t elapsed = currentTime - stateStartTime;

					if(elapsed < 150)
					{
						stopMove();
					}
					else
					{
						uint8_t left = distance[SENSOR_LEFT];
						uint8_t right = distance[SENSOR_RIGHT];

						setSpeed(400);
						if(left > right)
						{
							turnLeftForward();
						}
						else
						{
							turnRightForward();
						}

						currentState = STATE_TURNING;
						stateStartTime = currentTime;
					}
					break;
				}

				case STATE_TURNING:
				{
					uint32_t elapsed = currentTime - stateStartTime;

					uint8_t left = distance[SENSOR_LEFT];
					uint8_t right = distance[SENSOR_RIGHT];

					if(elapsed < 300)
					{
						if(left > right)
						{
							turnLeftForward();
						}
						else
						{
							turnRightForward();
						}
					}
					else  // 회전 완료, 정상 주행 복귀
					{
						currentState = STATE_NORMAL;
					}
					break;
				}
			}
		HAL_Delay(5);
	}
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
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
