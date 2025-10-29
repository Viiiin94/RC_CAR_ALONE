
#include "rc_control.h"

void moveForward()
{
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 1);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 1);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 0);

	  TIM3->CCR1 = 900;
	  TIM3->CCR2 = 900;
}

void moveBack()
{
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 1);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 0);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 1);

	  TIM3->CCR1 = 700;
	  TIM3->CCR2 = 700;
}

void moveLeft()
{
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 1);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 0);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 1);

	  TIM3->CCR1 = 700;
	  TIM3->CCR2 = 700;
}

void moveRight()
{
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 1);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 1);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 0);

	  TIM3->CCR1 = 700;
	  TIM3->CCR2 = 700;
}
