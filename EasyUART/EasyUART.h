// EasyUART.h
#ifndef EASYUART_H
#define EASYUART_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// UART Transmission Speed Configuration (in milliseconds)
#define EASYUART_VERY_FAST_INTERVAL  10000   // 10 ms
#define EASYUART_FAST_INTERVAL       100000  // 100 ms
#define EASYUART_SLOW_INTERVAL       1000000  // 1000 ms

// UART Frame Protocol Constants
#define EASYUART_START_BYTE          0x7E // Arbitrary start-of-frame byte
#define EASYUART_END_BYTE            0x7F // Arbitrary end-of-frame byte

#define EASYUART_BAUD_RATE			 115200
// Define EasyUART Packet Structure
typedef struct {
    uint8_t id;
    uint8_t *data;
    uint8_t data_size;
} EasyUART_Variable;

typedef enum {
    EASYUART_VERY_FAST = 0,
    EASYUART_FAST,
    EASYUART_SLOW,
    EASYUART_SPEED_COUNT
} EasyUART_Speed;

// Define the main structure for EasyUART
typedef struct {
    UART_HandleTypeDef *uart;
    TIM_HandleTypeDef *timer;
    uint32_t intervals[EASYUART_SPEED_COUNT];
    uint32_t last_sent_times[EASYUART_SPEED_COUNT];
    EasyUART_Variable *queues[EASYUART_SPEED_COUNT];
    uint8_t queue_sizes[EASYUART_SPEED_COUNT];
} EasyUART_HandleTypeDef;

// Library Functions
void init_EasyUART(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim);
void run_EasyUART();
void send_EasyUART(void *variable, uint8_t variable_id, uint8_t data_size, EasyUART_Speed speed);

#endif // EASYUART_H
