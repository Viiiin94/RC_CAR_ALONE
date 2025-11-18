#ifndef INC_AUTOMOBILITY_H_
#define INC_AUTOMOBILITY_H_

#include "rc_control.h"
#include "ultrasonic.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"

#define STOP_DISTANCE    	30              // 정면 정지 거리
#define DIFF_SIDE_DISTANCE	30

#define STOP_TIME			200
#define BASE_SPEED 			500

typedef enum {
	STATE_NORMAL,              // 정상 주행
	STATE_STOPPING,            // 정지 중
	STATE_TURNING,             // 회전 중
} DriveState;

void autonomobility_init(void);
void autonomobility_run(void);
void autonomobility_reset(void);
DriveState autonomobility_get_state(void);

#endif /* INC_AUTOMOBILITY_H_ */
