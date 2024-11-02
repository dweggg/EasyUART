#ifndef EASYUART_H
#define EASYUART_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_DATA_SIZE  sizeof(float) // Adjust if you plan to support larger data types

typedef enum {
    TYPE_NONE,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_ENUM,
    // Add more types as needed
} EasyUART_VariableType;

typedef enum {
    SPEED_SLOW = 2000000,    // 2 seconds in microseconds
    SPEED_FAST = 1000000,    // 1 second in microseconds
    SPEED_VERY_FAST = 500000, // 500 ms in microseconds
} EasyUART_TransmissionSpeed;

typedef struct {
    EasyUART_VariableType type;
    uint8_t id;
    uint8_t data[MAX_DATA_SIZE];  // Buffer to hold actual data
} EasyUART_Variable;

void init_EasyUART(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim);
void send_EasyUART(uint8_t id, void *data);
void run_EasyUART(void);

#endif // EASYUART_H
