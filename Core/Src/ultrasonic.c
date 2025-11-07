#include "ultrasonic.h"

extern uint8_t distance[3];

void getUltraSonicTrigger()
{
    static uint8_t sensor_idx = 0;

    __HAL_TIM_ENABLE_IT(&htim1, (TIM_IT_CC1 << sensor_idx));

    GPIO_TypeDef* ports[3] = {TRIG0_PORT, TRIG1_PORT, TRIG2_PORT};
    uint16_t pins[3] = {TRIG0_PIN, TRIG1_PIN, TRIG2_PIN};
    HAL_GPIO_WritePin(ports[sensor_idx], pins[sensor_idx], 0);
    delay_us(2);
    HAL_GPIO_WritePin(ports[sensor_idx], pins[sensor_idx], 1);
    delay_us(10);
    HAL_GPIO_WritePin(ports[sensor_idx], pins[sensor_idx], 0);

    sensor_idx = (sensor_idx + 1) % 3;
}

uint8_t getDistance(uint8_t sensor_idx)
{
	if(sensor_idx < 3)
	{
		return distance[sensor_idx];
	}
	return 255;
}
