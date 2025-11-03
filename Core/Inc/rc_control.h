#ifndef INC_RC_CONTROL_H_
#define INC_RC_CONTROL_H_

#include "stm32f4xx_hal.h"

#define UP      0x0001
#define DOWN    0x0002
#define LEFT    0x0004
#define RIGHT   0x0008
#define STOP	0x0000

#define SPEED_NORMAL  500
#define SPEED_SLOW    300

void setSpeed(uint16_t speed);

void stopMove();
void moveForward();
void moveBack();
void moveLeft();
void moveRight();
void onPressJoyStickKey(uint16_t button);

#endif /* INC_RC_CONTROL_H_ */
