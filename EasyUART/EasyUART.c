#include "EasyUART.h"

#define MAX_VARIABLES 10
#define PACKET_SIZE 256  // Define maximum packet size
#define START_OF_FRAME 0x7E
#define END_OF_FRAME 0x7F
#define MAX_PACKET_COUNT 3  // Number of different speeds

static UART_HandleTypeDef *huart2;
static TIM_HandleTypeDef *htim5;
static EasyUART_Variable variables[MAX_VARIABLES];
static uint8_t variable_count = 0;
static uint32_t transmission_speeds[MAX_PACKET_COUNT] = {1000, 500, 100}; // Transmission speeds in ms
static uint32_t current_speed_index = 0;
static uint32_t last_send_time = 0;

void init_EasyUART(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim) {
    huart2 = huart;
    htim5 = htim;

    // Additional initialization if necessary
}

void send_EasyUART(void *data, uint8_t id, EasyUART_VariableType type) {
    if (variable_count >= MAX_VARIABLES) {
        return;  // Queue full
    }

    variables[variable_count].data = data;
    variables[variable_count].id = id;
    variables[variable_count].type = type;
    variable_count++;
}

static uint8_t calculateChecksum(uint8_t *data, uint8_t size) {
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < size; i++) {
        checksum ^= data[i];  // Simple XOR checksum
    }
    return checksum;
}

static void buildAndSendPacket() {
    uint8_t packet[PACKET_SIZE];
    uint8_t index = 0;

    packet[index++] = START_OF_FRAME;  // Start of frame
    packet[index++] = variable_count;   // Count of variables
    packet[index++] = HAL_GetTick();     // Timestamp

    for (uint8_t i = 0; i < variable_count; i++) {
        packet[index++] = variables[i].id;  // Variable ID
        switch (variables[i].type) {
            case TYPE_INT:
                memcpy(&packet[index], variables[i].data, sizeof(int));
                index += sizeof(int);
                break;
            case TYPE_FLOAT:
                memcpy(&packet[index], variables[i].data, sizeof(float));
                index += sizeof(float);
                break;
            case TYPE_BOOL:
                memcpy(&packet[index], variables[i].data, sizeof(bool));
                index += sizeof(bool);
                break;
            // Add cases for other types as needed
        }
    }

    // Calculate and add checksum
    uint8_t checksum = calculateChecksum(packet, index);
    packet[index++] = checksum;

    packet[index++] = END_OF_FRAME;  // End of frame

    // Send the packet via UART
    HAL_UART_Transmit(huart2, packet, index, HAL_MAX_DELAY);
}

void run_EasyUART(void) {
    uint32_t current_time = HAL_GetTick();

    if (current_time - last_send_time >= transmission_speeds[current_speed_index]) {
        buildAndSendPacket();
        last_send_time = current_time;

        // Reset variable count after sending
        variable_count = 0;
    }
}

void setTransmissionSpeed(uint32_t speed_ms) {
    for (uint8_t i = 0; i < MAX_PACKET_COUNT; i++) {
        if (transmission_speeds[i] == speed_ms) {
            current_speed_index = i;  // Set to corresponding index
            return;
        }
    }

    // If not found, you can add new speed or handle it accordingly
}
