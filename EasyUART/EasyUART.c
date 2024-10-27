// EasyUART.c
#include "EasyUART.h"
#include <string.h> // For memcpy
#include <stdlib.h> // For malloc/free

// Static instance of the EasyUART handle
static EasyUART_HandleTypeDef easyuart_handle;

// Helper functions
static void EasyUART_TransmitPacket(EasyUART_Speed speed);
static uint8_t EasyUART_CalculateChecksum(uint8_t *data, uint16_t length);
static uint32_t EasyUART_GetTimestamp();

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
static void setup_timer(TIM_HandleTypeDef* htim) {

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

// Initialize EasyUART with UART and Timer handles
void init_EasyUART(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim) {
    easyuart_handle.uart = huart;
    easyuart_handle.timer = htim;

    setup_UART(huart);
    setup_timer(htim);

    easyuart_handle.intervals[EASYUART_VERY_FAST] = EASYUART_VERY_FAST_INTERVAL;
    easyuart_handle.intervals[EASYUART_FAST] = EASYUART_FAST_INTERVAL;
    easyuart_handle.intervals[EASYUART_SLOW] = EASYUART_SLOW_INTERVAL;
    memset(easyuart_handle.last_sent_times, 0, sizeof(easyuart_handle.last_sent_times));
    memset(easyuart_handle.queues, 0, sizeof(easyuart_handle.queues));
    memset(easyuart_handle.queue_sizes, 0, sizeof(easyuart_handle.queue_sizes));
}

// Queue a variable for transmission
void send_EasyUART(void *variable, uint8_t variable_id, uint8_t data_size, EasyUART_Speed speed) {
    EasyUART_Variable var = { variable_id, malloc(data_size), data_size };
    memcpy(var.data, variable, data_size);
    easyuart_handle.queues[speed][easyuart_handle.queue_sizes[speed]++] = var;
}

// Main function to manage transmission based on intervals
void run_EasyUART() {
    uint32_t current_time = EasyUART_GetTimestamp();
    for (EasyUART_Speed speed = EASYUART_VERY_FAST; speed < EASYUART_SPEED_COUNT; speed++) {
        if ((current_time - easyuart_handle.last_sent_times[speed]) >= easyuart_handle.intervals[speed]) {
            EasyUART_TransmitPacket(speed);
            easyuart_handle.last_sent_times[speed] = current_time;
        }
    }
}

// Transmit a packet for a given speed
static void EasyUART_TransmitPacket(EasyUART_Speed speed) {
    uint8_t packet[256];
    uint8_t idx = 0;
    uint16_t packet_size = 2 + 4 + (easyuart_handle.queue_sizes[speed] * 2); // Start + End + Timestamp + data

    // Start of frame
    packet[idx++] = EASYUART_START_BYTE;
    packet[idx++] = packet_size;

    // Timestamp
    uint32_t timestamp = EasyUART_GetTimestamp();
    memcpy(&packet[idx], &timestamp, sizeof(timestamp));
    idx += sizeof(timestamp);

    // Variable data
    for (int i = 0; i < easyuart_handle.queue_sizes[speed]; i++) {
        EasyUART_Variable var = easyuart_handle.queues[speed][i];
        packet[idx++] = var.id;
        memcpy(&packet[idx], var.data, var.data_size);
        idx += var.data_size;
        free(var.data);  // Free memory after transmission
    }

    // Checksum
    packet[idx++] = EasyUART_CalculateChecksum(packet, idx);

    // End of frame
    packet[idx++] = EASYUART_END_BYTE;

    // Send the packet over UART
    HAL_UART_Transmit(easyuart_handle.uart, packet, idx, HAL_MAX_DELAY);
    easyuart_handle.queue_sizes[speed] = 0; // Clear queue
}

// Calculate checksum
static uint8_t EasyUART_CalculateChecksum(uint8_t *data, uint16_t length) {
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < length; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

// Get current timestamp from timer
static uint32_t EasyUART_GetTimestamp() {
    return __HAL_TIM_GET_COUNTER(easyuart_handle.timer);
}
