#include "mode.h"
#include "stdio.h"

// 현재 모드 (기본값: 조이스틱)
Carmode current_mode = MODE_JOYSTICK;

// 모드 초기화
void mode_init(void)
{
    current_mode = MODE_JOYSTICK;
    printf("Mode initialized: JOYSTICK\n");
}

// 모드 전환
void mode_switch(void)
{
    if (current_mode == MODE_JOYSTICK)
    {
        current_mode = MODE_AUTOMOBILITY;
        printf("Mode switched to: AUTO\n");
    }
    else
    {
        current_mode = MODE_JOYSTICK;
        printf("Mode switched to: JOYSTICK\n");
    }
}

// 현재 모드 가져오기
Carmode mode_get(void)
{
    return current_mode;
}

// 모드 설정
void mode_set(Carmode new_mode)
{
    current_mode = new_mode;
    printf("Mode set to: %s\n",
           (new_mode == MODE_JOYSTICK) ? "JOYSTICK" : "AUTO");
}
