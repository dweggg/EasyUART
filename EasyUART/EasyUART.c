#include "EasyUART.h"
#include <string.h>

#define MAX_VARIABLES 10
#define PACKET_SIZE 256  // Define maximum packet size
#define START_OF_FRAME 0xAA
#define END_OF_FRAME 0x55

// UART and Timer handler
static UART_HandleTypeDef *huart_EasyUART;
static TIM_HandleTypeDef *htim_EasyUART;

// Fixed slot arrays for variable data
static EasyUART_Variable slow_variables[MAX_VARIABLES] = {0};
static EasyUART_Variable fast_variables[MAX_VARIABLES] = {0};
static EasyUART_Variable very_fast_variables[MAX_VARIABLES] = {0};

volatile static uint32_t last_send_time_slow = 0;
volatile static uint32_t last_send_time_fast = 0;
volatile static uint32_t last_send_time_very_fast = 0;
volatile static uint32_t current_time = 0;
// Variable dictionary that associates IDs with types and speeds

// 32-bit time tracking variable to extend the timer
volatile static uint32_t extended_time = 0;
volatile static uint16_t last_timer_count = 0;


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

void configureMicrosecondTimer(TIM_HandleTypeDef *htim) {
    // Assume the timer clock frequency is 84 MHz for APB1 timers on STM32F4 series
    uint32_t timer_clock = HAL_RCC_GetPCLK1Freq();  // Get the APB1 clock frequency
    uint32_t prescaler_value = (timer_clock / 1000000) - 1;  // Prescaler for 1 µs tick

    htim->Instance = TIM4;  // Replace TIMx with the specific timer instance, e.g., TIM2, TIM3
    htim->Init.Prescaler = prescaler_value;  // Set prescaler for 1 µs resolution
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;  // Count up mode
    htim->Init.Period = 0xFFFF;  // Max period for 32-bit counter (or 0xFFFF for 16-bit)
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;  // No clock division
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;  // Disable preload

    // Initialize the timer with the configuration above
    if (HAL_TIM_Base_Init(htim) != HAL_OK) {
        // Initialization Error
    }

    // Start the timer in basic mode (no interrupts)
    if (HAL_TIM_Base_Start(htim) != HAL_OK) {
        // Starting Error
    }
}

void init_EasyUART(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim) {
    huart_EasyUART = huart;
    huart_EasyUART->Init.BaudRate = EasyUART_BaudRate;
    HAL_UART_Init(huart_EasyUART);

    htim_EasyUART = htim;
    configureMicrosecondTimer(htim_EasyUART);
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

    packet[index++] = END_OF_FRAME;  // End of frame

    // Send the packet via UART
    HAL_UART_Transmit_IT(huart_EasyUART, packet, index);
}


// Function to update extended time based on 16-bit timer overflow
void updateExtendedTime() {
    uint16_t current_timer_count = __HAL_TIM_GET_COUNTER(htim_EasyUART);

    // Check for overflow
    if (current_timer_count < last_timer_count) {
        extended_time += 0x10000;  // Increment by 2^16 (65536) on overflow
    }

    last_timer_count = current_timer_count;

    // Calculate the full 32-bit time in microseconds
    current_time = extended_time + current_timer_count;
}


void run_EasyUART(void) {
    // Update current_time by extending 16-bit timer to 32-bit
    updateExtendedTime();

    static uint8_t count = 0;

    if ((current_time - last_send_time_slow) >= SPEED_SLOW) {
//        buildAndSendPacket(slow_variables, &count, SPEED_SLOW);
        last_send_time_slow = current_time;
    }

    if ((current_time - last_send_time_fast) >= SPEED_FAST) {
//        buildAndSendPacket(fast_variables, &count, SPEED_FAST);
        last_send_time_fast = current_time;
    }

    if ((current_time - last_send_time_very_fast) >= SPEED_VERY_FAST) {
        buildAndSendPacket(very_fast_variables, &count, SPEED_VERY_FAST);
        last_send_time_very_fast = current_time;
    }
}
