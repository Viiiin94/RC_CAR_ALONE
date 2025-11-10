#include "ultrasonic.h"

uint16_t IC_Value1[3] = {0};
uint16_t IC_Value2[3] = {0};
uint16_t echoTime[3] = {0};
uint8_t captureFlag[3] = {0};
uint8_t distance[3] = {0}; // 실제 거리

// ⭐ 이동 평균 큐
static uint8_t distanceQueue[3][QUEUE_SIZE] = {0};  // 각 센서별 5개 큐
static uint8_t queueIndex[3] = {0};                  // 큐 인덱스
static uint16_t queueSum[3] = {0};                   // 합계 (평균 계산용)
static uint8_t queueCount[3] = {0};                  // 데이터 개수

// ⭐ 큐에 새 값 추가하고 평균 계산
void addToQueue(uint8_t sensor_idx, uint8_t new_value)
{
	if(sensor_idx >= 3) return;

	// 큐가 가득 찬 경우 가장 오래된 값 제거
	if(queueCount[sensor_idx] >= QUEUE_SIZE)
	{
		queueSum[sensor_idx] -= distanceQueue[sensor_idx][queueIndex[sensor_idx]];
	}
	else
	{
		queueCount[sensor_idx]++;
	}

	// 새 값 추가
	distanceQueue[sensor_idx][queueIndex[sensor_idx]] = new_value;
	queueSum[sensor_idx] += new_value;

	// 인덱스 순환 (0 → 1 → 2 → 3 → 4 → 0)
	queueIndex[sensor_idx] = (queueIndex[sensor_idx] + 1) % QUEUE_SIZE;

	// 평균 계산
	if(queueCount[sensor_idx] > 0)
	{
		distance[sensor_idx] = queueSum[sensor_idx] / queueCount[sensor_idx];
	}
}

// Input Capture 콜백
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM1)
	{
		int sensor_idx = -1;
		uint32_t TIM_Channel = 0;

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
			if(captureFlag[sensor_idx] == 0)
			{
				// Rising edge - 에코 시작
				IC_Value1[sensor_idx] = HAL_TIM_ReadCapturedValue(htim, TIM_Channel);
				captureFlag[sensor_idx] = 1;
				__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_Channel, TIM_INPUTCHANNELPOLARITY_FALLING);
			}
			else if(captureFlag[sensor_idx] == 1)
			{
				// Falling edge - 에코 종료
				IC_Value2[sensor_idx] = HAL_TIM_ReadCapturedValue(htim, TIM_Channel);

				// 에코 시간 계산
				if(IC_Value2[sensor_idx] > IC_Value1[sensor_idx])
				{
					echoTime[sensor_idx] = IC_Value2[sensor_idx] - IC_Value1[sensor_idx];
				}
				else
				{
					echoTime[sensor_idx] = (0xffff - IC_Value1[sensor_idx]) + IC_Value2[sensor_idx];
				}

				// 거리 계산 (58us = 1cm)
				uint16_t measured_distance = echoTime[sensor_idx] / 58;

				// 최대값 제한
				if(measured_distance > MAX_DISTANCE)
				{
					measured_distance = MAX_DISTANCE;
				}

				// ⭐ 큐에 추가하고 평균 계산
				addToQueue(sensor_idx, measured_distance);

				// 플래그 초기화
				captureFlag[sensor_idx] = 0;
				__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_Channel, TIM_INPUTCHANNELPOLARITY_RISING);

				// 인터럽트 비활성화
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


// ⭐ 센서 0 트리거 (우측)
void getRightTrigger(void)
{
	__HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC1);

	HAL_GPIO_WritePin(TRIG0_PORT, TRIG0_PIN, GPIO_PIN_RESET);
	delay_us(1);
	HAL_GPIO_WritePin(TRIG0_PORT, TRIG0_PIN, GPIO_PIN_SET);
	delay_us(10);
	HAL_GPIO_WritePin(TRIG0_PORT, TRIG0_PIN, GPIO_PIN_RESET);
}

// ⭐ 센서 1 트리거 (정면)
void getMiddleTrigger(void)
{
	__HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC2);

	HAL_GPIO_WritePin(TRIG1_PORT, TRIG1_PIN, GPIO_PIN_RESET);
	delay_us(1);
	HAL_GPIO_WritePin(TRIG1_PORT, TRIG1_PIN, GPIO_PIN_SET);
	delay_us(10);
	HAL_GPIO_WritePin(TRIG1_PORT, TRIG1_PIN, GPIO_PIN_RESET);
}

// ⭐ 센서 2 트리거 (좌측)
void getLeftTrigger(void)
{
	__HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC3);

	HAL_GPIO_WritePin(TRIG2_PORT, TRIG2_PIN, GPIO_PIN_RESET);
	delay_us(1);
	HAL_GPIO_WritePin(TRIG2_PORT, TRIG2_PIN, GPIO_PIN_SET);
	delay_us(10);
	HAL_GPIO_WritePin(TRIG2_PORT, TRIG2_PIN, GPIO_PIN_RESET);
}
