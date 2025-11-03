#include "ultrasonic.h"

extern uint8_t distance[3];

void getUltraSonicTrigger()
{
	__HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3);

	// Trig 펄스 발생
	HAL_GPIO_WritePin(TRIG0_PORT, TRIG0_PIN, 0);
	HAL_GPIO_WritePin(TRIG1_PORT, TRIG1_PIN, 0);
	HAL_GPIO_WritePin(TRIG2_PORT, TRIG2_PIN, 0);
	delay_us(2);

	HAL_GPIO_WritePin(TRIG0_PORT, TRIG0_PIN, 1);
	HAL_GPIO_WritePin(TRIG1_PORT, TRIG1_PIN, 1);
	HAL_GPIO_WritePin(TRIG2_PORT, TRIG2_PIN, 1);
	delay_us(10);

	HAL_GPIO_WritePin(TRIG0_PORT, TRIG0_PIN, 0);
	HAL_GPIO_WritePin(TRIG1_PORT, TRIG1_PIN, 0);
	HAL_GPIO_WritePin(TRIG2_PORT, TRIG2_PIN, 0);
}

uint8_t getDistance(uint8_t sensor_idx)
{
	if(sensor_idx < 3)
	{
		return distance[sensor_idx];
	}
	return 255;
}
