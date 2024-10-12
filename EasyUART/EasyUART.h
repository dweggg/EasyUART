#ifndef EASYUART_H
#define EASYUART_H

#include <stdint.h>
#include "stm32f4xx_hal.h"  // Adjust this header based on your STM32 family

#define MAX_VARIABLES 10
#define MAX_QUEUE_SIZE 10

// Define the baud rate for UART communication
#define EASYUART_BAUD_RATE 115200  // Adjust as necessary

// Enumeration for variable IDs
typedef enum {
    VARIABLE_1_ID,
    VARIABLE_2_ID,
    // Add more IDs as necessary
    VARIABLE_N_ID,
    VARIABLE_ID_COUNT
} VariableID;

// Enumeration for data types
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    // Add more as necessary
} DataType;

// Struct for variable metadata
typedef struct {
    VariableID id;
    DataType type;
    uint32_t transmission_speed_us; // Custom transmission speed in microseconds
    uint32_t last_transmission_time; // Last transmission timestamp in microseconds
} VariableMetaData;

// Queue entry struct for scheduling sends
typedef struct {
    VariableID id;
    void* data; // Pointer to the variable data
} QueueEntry;

// Public function prototypes
void init_EasyUART(UART_HandleTypeDef* huart, TIM_HandleTypeDef* htim);
void run_EasyUART(void);
int send_EasyUART(void* variable, VariableID id);
void* read_EasyUART(VariableID id);

#endif // EASYUART_H
