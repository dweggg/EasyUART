#ifndef EASYUART_H
#define EASYUART_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_DATA_SIZE  sizeof(float) // Adjust if you plan to support larger data types

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_ENUM,
    // Add more types as needed
} EasyUART_VariableType;

typedef enum {
    SPEED_SLOW = 2,    // 2 seconds
    SPEED_FAST = 10,    // 1 second
    SPEED_VERY_FAST = 500, // 500 ms
} EasyUART_TransmissionSpeed;

typedef struct {
    EasyUART_VariableType type;
    uint8_t id;
    uint8_t data[MAX_DATA_SIZE];  // Buffer to hold actual data
} EasyUART_Variable;

void init_EasyUART(UART_HandleTypeDef *huart);
void send_EasyUART(uint8_t id, void *data);
void run_EasyUART(void);

#endif // EASYUART_H
