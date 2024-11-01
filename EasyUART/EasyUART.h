#ifndef EASYUART_H
#define EASYUART_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// Define variable types for transmission
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_ENUM,
    // Add more types as needed
} EasyUART_VariableType;

typedef struct {
    EasyUART_VariableType type;
    uint8_t id;
    void *data;  // Pointer to the variable data
} EasyUART_Variable;

void init_EasyUART(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim);
void send_EasyUART(void *data, uint8_t id, EasyUART_VariableType type);
void run_EasyUART(void);
void setTransmissionSpeed(uint32_t speed_ms);  // Set transmission speed

#endif // EASYUART_H
