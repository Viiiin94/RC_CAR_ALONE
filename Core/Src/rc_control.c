#include "rc_control.h"

static uint16_t current_speed = SPEED_NORMAL;

void setSpeed(uint16_t speed)
{
	current_speed = speed;
}

void stopMove(void){
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 0);

	TIM3->CCR1 = 0;
	TIM3->CCR2 = 0;
}

void moveForward()
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 0);

	TIM3->CCR1 = current_speed;
	TIM3->CCR2 = current_speed;
}

void moveBack()
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 1);

	TIM3->CCR1 = current_speed;
	TIM3->CCR2 = current_speed;
}

void moveLeft()
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 1);

	TIM3->CCR1 = current_speed;
	TIM3->CCR2 = current_speed;
}

void moveRight()
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 0);

	TIM3->CCR1 = current_speed;
	TIM3->CCR2 = current_speed;
}

void turnRightForward()
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 0);

	TIM3->CCR1 = 0;
	TIM3->CCR2 = current_speed;
}

void turnLeftForward()
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 0);

	TIM3->CCR1 = current_speed;
	TIM3->CCR2 = 0;
}

void onPressJoyStickKey(uint16_t button)
{
	uint16_t direction = button & (UP | DOWN | LEFT | RIGHT);

	if (direction == STOP)
	{
		stopMove();
		return;
	}

	if (button & UP)    { moveForward(); }
	if (button & DOWN)  { moveBack();    }
	if (button & LEFT)  { moveLeft();    }   // 제자리 좌회전
	if (button & RIGHT) { moveRight();   }   // 제자리 우회전
	if (button & STOP)  { stopMove();    }
}
