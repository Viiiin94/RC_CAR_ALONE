
#ifndef INC_JOYSTICK_PAIRING_H_
#define INC_JOYSTICK_PAIRING_H_

#include "stm32f4xx_hal.h"

#define AT_RESPONSE_BUFFER_SIZE  100
#define AT_COMMAND_TIMEOUT       1000  // ms

void HM10_SendCommand(UART_HandleTypeDef *huart, char* command);
void HM10_SendCommand_WithResponse(UART_HandleTypeDef *huart, char* command, char* response, uint16_t timeout);

void HM10_UART1_SLAVE_SETTING(void);
void HM10_UART6_MASTER_SETTING(void);

HAL_StatusTypeDef HM10_WaitResponse(UART_HandleTypeDef *huart, char* response, uint16_t timeout);

#endif /* INC_JOYSTICK_PAIRING_H_ */
