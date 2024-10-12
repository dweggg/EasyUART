#include "EasyUART.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Static variable database
static VariableMetaData variable_database[MAX_VARIABLES] = {
    { VARIABLE_1_ID, TYPE_INT, 100, 0 },  // Transmission every 100us
    { VARIABLE_2_ID, TYPE_FLOAT, 200, 0 }, // Transmission every 200us
    // Add more variables
};

// Send queue (FIFO)
static QueueEntry send_queue[MAX_QUEUE_SIZE];
static int queue_head = 0;
static int queue_tail = 0;

// Buffer for received variables (most recent value received per variable)
static void* received_values[MAX_VARIABLES];

// Pointers to UART and Timer handles
static UART_HandleTypeDef* huart_global;
static TIM_HandleTypeDef* htim_global;

// Function to get the current time in microseconds
uint32_t get_current_time_us(void) {
    return __HAL_TIM_GET_COUNTER(htim_global);
}

// Initialize UART
static void setup_UART(UART_HandleTypeDef* huart) {
    huart->Init.BaudRate = EASYUART_BAUD_RATE; // Set baud rate from defined macro
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(huart) != HAL_OK) {
        // Handle error (e.g., trigger an error LED or a system reset)
    }
}

// Initialize Timer for microsecond counting
static void setup_Timer(TIM_HandleTypeDef* htim) {
    // Enable the clock for the timer (user should ensure the correct clock is enabled)
    // e.g., if using TIM2, then __HAL_RCC_TIM2_CLK_ENABLE();
    
    // Configure timer to count in microseconds
    htim->Init.Prescaler = (HAL_RCC_GetPCLK1Freq() / 1000000) - 1; // Prescaler for 1us
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = 0xFFFFFFFF;  // Max period (32-bit timer overflow)
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(htim) != HAL_OK) {
        // Handle error (e.g., trigger an error LED or system reset)
    }

    HAL_TIM_Base_Start(htim);  // Start the timer
}

// Initialize UART and Timer peripherals, allowing user-defined configurations
void init_EasyUART(UART_HandleTypeDef* huart, TIM_HandleTypeDef* htim) {
    // Store the UART and Timer handles globally
    huart_global = huart;
    htim_global = htim;

    // Reinitialize UART peripheral to ensure correct configuration
    setup_UART(huart_global);

    // Reinitialize Timer peripheral to ensure it counts in microseconds
    setup_Timer(htim_global);

    // Initialize memory
    memset(send_queue, 0, sizeof(send_queue));
    memset(received_values, 0, sizeof(received_values));
}

// Add a variable to the send queue
int send_EasyUART(void* variable, VariableID id) {
    if (id < 0 || id >= VARIABLE_ID_COUNT) {
        return -1; // Invalid variable ID
    }
    
    // Enqueue the variable for transmission
    if ((queue_tail + 1) % MAX_QUEUE_SIZE == queue_head) {
        return -2; // Error: Queue is full
    }

    QueueEntry* entry = &send_queue[queue_tail];
    entry->id = id;

    // Allocate memory and copy the variable data
    VariableMetaData meta = variable_database[id];
    switch (meta.type) {
        case TYPE_INT:
            entry->data = malloc(sizeof(int));
            memcpy(entry->data, variable, sizeof(int));
            break;

        case TYPE_FLOAT:
            entry->data = malloc(sizeof(float));
            memcpy(entry->data, variable, sizeof(float));
            break;

        case TYPE_STRING:
            entry->data = malloc(strlen((char*)variable) + 1);
            strcpy((char*)entry->data, (char*)variable);
            break;

        default:
            return -3; // Unsupported data type
    }

    queue_tail = (queue_tail + 1) % MAX_QUEUE_SIZE;
    return 0; // Success
}

// Run the EasyUART main loop, handling transmission and reception
void run_EasyUART(void) {
    uint32_t current_time = get_current_time_us();

    // 1. Process send queue
    for (int i = queue_head; i != queue_tail; i = (i + 1) % MAX_QUEUE_SIZE) {
        QueueEntry* entry = &send_queue[i];
        VariableMetaData* meta = &variable_database[entry->id];

        // Check if it's time to send this variable
        if (current_time - meta->last_transmission_time >= meta->transmission_speed_us) {
            // Send the variable over UART
            switch (meta->type) {
                case TYPE_INT:
                    HAL_UART_Transmit(huart_global, (uint8_t*)entry->data, sizeof(int), HAL_MAX_DELAY);
                    break;
                
                case TYPE_FLOAT:
                    HAL_UART_Transmit(huart_global, (uint8_t*)entry->data, sizeof(float), HAL_MAX_DELAY);
                    break;

                case TYPE_STRING:
                    HAL_UART_Transmit(huart_global, (uint8_t*)entry->data, strlen((char*)entry->data), HAL_MAX_DELAY);
                    break;

                default:
                    break; // Unsupported type
            }

            // Update the last transmission time
            meta->last_transmission_time = current_time;

            // Remove from queue
            free(entry->data); // Free allocated memory
            queue_head = (queue_head + 1) % MAX_QUEUE_SIZE;
        }
    }

    // 2. Handle reception (Example code, modify based on platform)
    uint8_t received_data[4]; // Adjust size based on expected data types
    if (HAL_UART_Receive(huart_global, received_data, sizeof(received_data), 0) == HAL_OK) {
        // Here we would typically parse the received data
        // For simplicity, assume we are receiving data in the format of an integer
        int receivedInt = *(int*)received_data;
        received_values[VARIABLE_1_ID] = malloc(sizeof(int));
        *(int*)received_values[VARIABLE_1_ID] = receivedInt; // Store the received integer

        // If we had more variable types, we would parse accordingly
    }
}


// Read the last received value of a variable
void* read_EasyUART(VariableID id) {
    if (id < 0 || id >= VARIABLE_ID_COUNT) {
        return NULL; // Invalid variable ID
    }
    
    // Return a pointer to the most recently received value
    void* received_value = received_values[id];
    if (received_value != NULL) {
        // Allocate memory for return value based on type
        void* return_value = malloc(sizeof(received_value)); // Allocate memory for return
        if (return_value == NULL) {
            return NULL; // Memory allocation failed
        }

        // Copy the received value to the allocated memory
        memcpy(return_value, received_value, sizeof(received_value));
        return return_value; // Return pointer to allocated memory
    }

    return NULL; // No value received
}
