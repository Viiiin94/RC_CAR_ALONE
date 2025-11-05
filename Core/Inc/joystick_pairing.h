
#ifndef INC_JOYSTICK_PAIRING_H_
#define INC_JOYSTICK_PAIRING_H_

#include "stm32f4xx_hal.h"

void HM10_SendCommand(UART_HandleTypeDef *huart, char* command);
void HM10_SendCommand_WithResponse(UART_HandleTypeDef *huart, char* command);

#endif /* INC_JOYSTICK_PAIRING_H_ */
