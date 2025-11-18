#include "automobility.h"

// 내부 상태 변수
static DriveState current_state = STATE_NORMAL;
static uint32_t stateStartTime = 0;

// 자율주행 초기화
void autonomobility_init(void)
{
	current_state = STATE_NORMAL;
	stateStartTime = 0;
	setSpeed(BASE_SPEED);
}

// 자율주행 상태 초기화
void autonomobility_reset(void)
{
	current_state = STATE_NORMAL;
	stateStartTime = 0;
	stopMove();
}

// 현재 상태 가져오기
DriveState autonomobility_get_state(void)
{
	return current_state;
}

// 자율주행 메인 로직
void autonomobility_run(void)
{
	uint32_t currentTime = HAL_GetTick();

	// 센서 값 읽기
	uint8_t right = distance[SENSOR_RIGHT];
	uint8_t forward = distance[SENSOR_FORWARD];
	uint8_t left = distance[SENSOR_LEFT];

	int diff = (int)left - (int)right;

	// 디버깅 (필요시)
	// printf("R:%2d F:%2d L:%2d DIFF:%2d\n", right, forward, left, diff);

	switch(current_state)
	{
	case STATE_NORMAL:
	{
		// 전방 장애물 감지 시 속도 감소
		if (forward < STOP_DISTANCE + 5)
		{
			setSpeed(BASE_SPEED - 50);
		}
		else
		{
			setSpeed(BASE_SPEED);
		}

		// 정지 필요 확인
		if (forward < STOP_DISTANCE && forward > 0)
		{
			current_state = STATE_STOPPING;
			stateStartTime = currentTime;
		}
		// 좌우 벽 감지 - 조정
		else if (abs(diff) > DIFF_SIDE_DISTANCE)
		{
			setSpeed(BASE_SPEED - 100);
			if(diff > 0)  // 좌측이 넓음 → 우측으로
					{
				moveRight();
					}
			else if(diff < 0)  // 우측이 넓음 → 좌측으로
			{
				moveLeft();
			}
			else
			{
				setSpeed(BASE_SPEED);
				moveForward();
			}
			current_state = STATE_NORMAL;
		}
		// 정상 주행
		else
		{
			setSpeed(BASE_SPEED);
			moveForward();
			current_state = STATE_NORMAL;
		}
		break;
	}

	case STATE_STOPPING:
	{
		uint32_t stopTime = currentTime - stateStartTime;

		if (stopTime < STOP_TIME)
		{
			stopMove();
		}
		else
		{
			current_state = STATE_TURNING;
			stateStartTime = currentTime;
		}
		break;
	}

	case STATE_TURNING:
	{
		static uint8_t clearCount = 0;
		static uint8_t prevForward = 0;

		// 정면 거리의 급격한 증가 감지 (탈출 감지)
		if (forward > prevForward + 30 || forward > STOP_DISTANCE + 20)
		{
			clearCount++;
			if (clearCount >= 2)
			{
				current_state = STATE_NORMAL;
				setSpeed(BASE_SPEED);
				clearCount = 0;
				prevForward = forward;
				break;
			}
		}
		else
		{
			clearCount = 0;
		}

		prevForward = forward;

		// 회전 방향 결정
		bool leftOrRight = (left >= right);

		setSpeed(BASE_SPEED - 100);
		if (leftOrRight)
		{
			turnLeft();
		}
		else
		{
			turnRight();
		}
		break;
	}
	}
}
