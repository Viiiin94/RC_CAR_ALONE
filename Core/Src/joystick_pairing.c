#include "joystick_pairing.h"
#include "usart.h"

// AT 명령어 응답 버퍼
static char at_response_buffer[AT_RESPONSE_BUFFER_SIZE];
static uint8_t rx_byte;

/**
 * @brief AT 명령어 전송 (응답 대기 없음)
 * @param huart: UART 핸들
 * @param command: 전송할 AT 명령어 문자열
 */
void HM10_SendCommand(UART_HandleTypeDef *huart, char* command)
{
    // 명령어 길이 계산
    uint16_t cmd_len = strlen(command);

    // UART로 전송
    HAL_UART_Transmit(huart, (uint8_t*)command, cmd_len, 1000);

    // 디버깅용 출력 (USART2로)
    printf("Send: %s\r\n", command);

    // 명령어 처리 대기
    HAL_Delay(100);
}

/**
 * @brief AT 명령어 전송 및 응답 대기
 * @param huart: UART 핸들
 * @param command: 전송할 AT 명령어
 * @param response: 응답 저장 버퍼
 * @param timeout: 타임아웃 (ms)
 */
void HM10_SendCommand_WithResponse(UART_HandleTypeDef *huart, char* command, char* response, uint16_t timeout)
{
    // 응답 버퍼 초기화
    memset(response, 0, AT_RESPONSE_BUFFER_SIZE);

    // 명령어 전송
    uint16_t cmd_len = strlen(command);
    HAL_UART_Transmit(huart, (uint8_t*)command, cmd_len, 1000);

    // 디버깅 출력
    printf("Send: %s\r\n", command);

    // 응답 수신
    HAL_StatusTypeDef status = HM10_WaitResponse(huart, response, timeout);

    if (status == HAL_OK)
    {
        printf("Response: %s\r\n", response);
    }
    else
    {
        printf("Response: TIMEOUT\r\n");
    }

    // 다음 명령어를 위한 대기
    HAL_Delay(100);
}

/**
 * @brief 응답 대기 함수
 * @param huart: UART 핸들
 * @param response: 응답 저장 버퍼
 * @param timeout: 타임아웃 (ms)
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef HM10_WaitResponse(UART_HandleTypeDef *huart, char* response, uint16_t timeout)
{
    uint32_t start_tick = HAL_GetTick();
    uint16_t idx = 0;

    // 타임아웃까지 응답 수신
    while ((HAL_GetTick() - start_tick) < timeout)
    {
        // 1바이트 수신 시도
        if (HAL_UART_Receive(huart, &rx_byte, 1, 10) == HAL_OK)
        {
            // 버퍼에 저장
            if (idx < AT_RESPONSE_BUFFER_SIZE - 1)
            {
                response[idx++] = rx_byte;
                response[idx] = '\0';  // NULL 종료
            }

            // "OK" 또는 개행 문자 확인 (응답 완료)
            if (strstr(response, "OK") != NULL ||
                strstr(response, "\r\n") != NULL)
            {
                return HAL_OK;
            }
        }
    }

    return HAL_TIMEOUT;
}

/**
 * @brief UART1 Slave 설정
 */
void HM10_UART1_SLAVE_SETTING(void)
{
    char response[AT_RESPONSE_BUFFER_SIZE];

    printf("\n=== UART1 (Slave) Setting Start ===\n");

    // AT 테스트
    HM10_SendCommand_WithResponse(&huart1, "AT", response, 1000);

    // 이름 설정
    HM10_SendCommand_WithResponse(&huart1, "AT+NAMEHM23SLV", response, 1000);

    // Role: Slave (0)
    HM10_SendCommand_WithResponse(&huart1, "AT+ROLE0", response, 1000);

    // 주소 확인
    HM10_SendCommand_WithResponse(&huart1, "AT+ADDR?", response, 1000);

    // UART 설정 (9600, N, 8, 1)
    HM10_SendCommand_WithResponse(&huart1, "AT+BAUD0", response, 1000);  // 9600

    // 리셋
    HM10_SendCommand(&huart1, "AT+RESET");

    printf("=== UART1 (Slave) Setting Complete ===\n\n");

    // 리셋 후 안정화 대기
    HAL_Delay(1000);
}

/**
 * @brief UART6 Master 설정
 */
void HM10_UART6_MASTER_SETTING(void)
{
    char response[AT_RESPONSE_BUFFER_SIZE];

    printf("\n=== UART6 (Master) Setting Start ===\n");

    // AT 테스트
    HM10_SendCommand_WithResponse(&huart6, "AT", response, 1000);

    // 이름 설정
    HM10_SendCommand_WithResponse(&huart6, "AT+NAMEHM23MASTER", response, 1000);

    // Role: Master (1)
    HM10_SendCommand_WithResponse(&huart6, "AT+ROLE1", response, 1000);

    // 연결 모드: 특정 주소로만 연결
    HM10_SendCommand_WithResponse(&huart6, "AT+IMME1", response, 1000);

    // UART 설정 (9600, N, 8, 1)
    HM10_SendCommand_WithResponse(&huart6, "AT+BAUD0", response, 1000);

    // Slave 주소로 연결 (주소는 Slave 설정 후 확인 필요)
    // 예: HM10_SendCommand_WithResponse(&huart6, "AT+CON001122334455", response, 1000);

    // 리셋
    HM10_SendCommand(&huart6, "AT+RESET");

    printf("=== UART6 (Master) Setting Complete ===\n\n");

    // 리셋 후 안정화 대기
    HAL_Delay(1000);
}
