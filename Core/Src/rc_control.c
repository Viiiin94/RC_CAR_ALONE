
#include "rc_control.h"

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

	TIM3->CCR1 = 1000;
	TIM3->CCR2 = 1000;
}

void moveBack()
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 1);

	TIM3->CCR1 = 1000;
	TIM3->CCR2 = 1000;
}

void moveLeft()
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 1);

	TIM3->CCR1 = 1000;
	TIM3->CCR2 = 1000;
}

void moveRight()
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 0);

	TIM3->CCR1 = 1000;
	TIM3->CCR2 = 1000;
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
