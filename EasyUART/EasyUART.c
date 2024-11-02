#include "EasyUART.h"
#include <string.h>

#define MAX_VARIABLES 10
#define PACKET_SIZE 256  // Define maximum packet size
#define START_OF_FRAME 0xAA
#define END_OF_FRAME 0x55

static uint32_t CLOCK_FREQUENCY = 0;
static uint8_t last_sent_packet[PACKET_SIZE] = {0};  // Store the last sent packet for debugging

// Declare the UART handler
static UART_HandleTypeDef *huart_EasyUART;  // UART handler

// Fixed slot arrays for variable data
static EasyUART_Variable slow_variables[MAX_VARIABLES] = {0};
static EasyUART_Variable fast_variables[MAX_VARIABLES] = {0};
static EasyUART_Variable very_fast_variables[MAX_VARIABLES] = {0};

volatile static uint32_t current_time;

// Time tracking for sending packets
static uint32_t last_send_time_slow = 0;
static uint32_t last_send_time_fast = 0;
static uint32_t last_send_time_very_fast = 0;

// Variable dictionary that associates IDs with types and speeds
static const struct {
    uint8_t id;
    EasyUART_VariableType type;
    EasyUART_TransmissionSpeed speed;
} variable_dictionary[] = {
    {0, TYPE_INT, SPEED_SLOW},
    {1, TYPE_FLOAT, SPEED_FAST},
    {2, TYPE_FLOAT, SPEED_VERY_FAST},
    {3, TYPE_FLOAT, SPEED_VERY_FAST},
    {4, TYPE_FLOAT, SPEED_FAST},
    // Add more entries as needed
};

void init_EasyUART(UART_HandleTypeDef *huart) {
    huart_EasyUART = huart;
    CLOCK_FREQUENCY = HAL_RCC_GetHCLKFreq();
}

void send_EasyUART(uint8_t id, void *data) {
    // Check if ID is valid and fetch from dictionary
    for (uint8_t i = 0; i < sizeof(variable_dictionary) / sizeof(variable_dictionary[0]); i++) {
        if (variable_dictionary[i].id == id) {
            // Select the appropriate array based on transmission speed
            EasyUART_Variable *variable_array;
            switch (variable_dictionary[i].speed) {
                case SPEED_SLOW:
                    variable_array = slow_variables;
                    break;
                case SPEED_FAST:
                    variable_array = fast_variables;
                    break;
                case SPEED_VERY_FAST:
                    variable_array = very_fast_variables;
                    break;
                default:
                    return;  // Unknown speed, exit
            }

            // Use ID as index, ensuring it is within bounds
            if (id < MAX_VARIABLES) {
                variable_array[id].type = variable_dictionary[i].type;
                variable_array[id].id = id;
                memcpy(variable_array[id].data, data, MAX_DATA_SIZE);  // Copy actual data to slot
            }
            break; // Exit loop once the ID is found
        }
    }
}

static uint8_t calculateChecksum(uint8_t *data, uint8_t size) {
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < size; i++) {
        checksum ^= data[i];  // Simple XOR checksum
    }
    return checksum;
}

void buildAndSendPacket(EasyUART_Variable *variables, uint8_t *count, EasyUART_TransmissionSpeed speed) {
    uint8_t packet[PACKET_SIZE];
    uint8_t index = 0;

    packet[index] = START_OF_FRAME;  // Start of frame

    index += 2; // skip 1 byte to allocate data length

    // Loop through the variables and add their data to the packet
    uint8_t data_length = 0;
    for (uint8_t i = 0; i < MAX_VARIABLES; i++) {
        if (variables[i].id != 0) { // Ensure ID slot is not empty
            packet[index++] = variables[i].id;  // Variable ID in the packet
            data_length += sizeof(variables[i].id);
            switch (variables[i].type) {
                case TYPE_INT:
                    memcpy(&packet[index], variables[i].data, sizeof(int));
                    index += sizeof(int);
                    data_length += sizeof(int);
                    break;
                case TYPE_FLOAT:
                    memcpy(&packet[index], variables[i].data, sizeof(float));
                    index += sizeof(float);
                    data_length += sizeof(float);
                    break;
                case TYPE_BOOL:
                    memcpy(&packet[index], variables[i].data, sizeof(bool));
                    index += sizeof(bool);
                    data_length += sizeof(bool);
                    break;
                case TYPE_ENUM:
                    memcpy(&packet[index], variables[i].data, sizeof(int)); // Assuming enum size is int
                    index += sizeof(int);
                    data_length += sizeof(int);
                    break;
                // Add cases for other types as needed
            }
        }
    }

    *count = data_length;  // Set count based on actual data length
    packet[1] = *count;    // Set the count byte in the packet

    // Calculate checksum over the entire packet excluding the checksum byte
//    uint8_t checksum = calculateChecksum(packet, index);
//    packet[index++] = checksum;  // Append checksum to the packet

    packet[index++] = END_OF_FRAME;  // End of frame

    // Store the last sent packet in the debug buffer
    memcpy(last_sent_packet, packet, index);

    // Send the packet via UART
    HAL_UART_Transmit(huart_EasyUART, packet, index, HAL_MAX_DELAY);
}

void run_EasyUART(void) {
    // Calculate current time based on SysTick and system clock
    current_time = SysTick->VAL;
    static uint8_t count = 0;

    // Send SLOW packet
    if (current_time - last_send_time_slow >= (SPEED_SLOW * CLOCK_FREQUENCY / 1000000)) {
        // buildAndSendPacket(slow_variables, &count, SPEED_SLOW);
        last_send_time_slow = current_time;
        memset(slow_variables, 0, sizeof(slow_variables)); // Clear variable data
    }

    // Send FAST packet
    if (current_time - last_send_time_fast >= (SPEED_FAST * CLOCK_FREQUENCY / 1000000)) {
        // buildAndSendPacket(fast_variables, &count, SPEED_FAST);
        last_send_time_fast = current_time;
        memset(fast_variables, 0, sizeof(fast_variables)); // Clear variable data
    }

    // Send VERY FAST packet
    if (current_time - last_send_time_very_fast >= (SPEED_VERY_FAST * CLOCK_FREQUENCY / 1000000)) {
        buildAndSendPacket(very_fast_variables, &count, SPEED_VERY_FAST);
        last_send_time_very_fast = current_time;
        memset(very_fast_variables, 0, sizeof(very_fast_variables)); // Clear variable data
    }
}

// Global for debugging purposes
volatile uint8_t *dbg_packet_ptr = last_sent_packet;  // Pointer to the last sent packet for debugging
