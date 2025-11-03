#ifndef INC_ULTRASONIC_H_
#define INC_ULTRASONIC_H_

#include "stm32f4xx_hal.h"
#include "delay_us.h"

// 우측
#define TRIG0_PORT	GPIOA
#define TRIG0_PIN	GPIO_PIN_5

// 정면
#define TRIG1_PORT	GPIOB
#define TRIG1_PIN	GPIO_PIN_2

// 좌측
#define TRIG2_PORT	GPIOB
#define TRIG2_PIN	GPIO_PIN_12

void HCSR04_TRIGGER();

#endif /* INC_ULTRASONIC_H_ */
