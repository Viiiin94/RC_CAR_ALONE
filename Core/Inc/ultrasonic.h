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

#define SENSOR_RIGHT    0
#define SENSOR_FORWARD  1
#define SENSOR_LEFT     2

#define MAX_DISTANCE  100

void getRightTrigger(void);  // 우측
void getMiddleTrigger(void);  // 정면
void getLeftTrigger(void);  // 좌측

extern uint16_t IC_Value1[3];
extern uint16_t IC_Value2[3];
extern uint16_t echoTime[3];
extern uint8_t captureFlag[3];
extern uint8_t distance[3];       // 원본 거리

#endif /* INC_ULTRASONIC_H_ */
