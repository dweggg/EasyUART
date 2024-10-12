# EasyUART

**EasyUART** is a portable library designed for MCU communication via UART. This library aims to simplify the process of sending and receiving real-time data between embedded systems and a PC interface.

## Features

- **User-Friendly GUI**: The library will feature an intuitive graphical user interface (GUI) for live data visualization and plotting of variables. This makes it easy to monitor and analyze real-time performance in embedded systems.
  
- **Custom Lightweight Protocol**: EasyUART will consist of a custom protocol that supports multiple speeds, allowing users to send certain messages faster than others. This flexibility ensures that critical data can be transmitted in a timely manner.

- **Database Generation Program**: Users will have the ability to create a database that defines the variables to be sent and their respective transmission speeds. The database generation tool will also:
  - Limit the number of variables based on the selected baud rate and data types.
  - Configure the target hardware (initially supporting STM32) while remaining hardware agnostic.
  - Generate accompanying `.c` and `.h` files that can be easily imported into your application to access public methods for sending and receiving messages.

- **Bidirectional Communication**: While the primary focus is on transmitting data from the MCU to the PC, EasyUART will also support bidirectional communication. This enables users to send commands, setpoints, and other control signals from the PC to the MCU.

## Usage Considerations

The amount of data and speed that can be logged or plotted will depend on the chosen baud rate, which is limited by the quality of the hardware setup, including the UART to USB converter and wiring. Careful consideration of the baud rate and data types will be necessary to optimize communication performance.

## Goals

- To provide a flexible and efficient UART communication library that can be adapted to various MCU platforms.
- To enhance real-time data monitoring and control in embedded systems through an easy-to-use GUI.
- To facilitate the development of robust embedded applications by providing a structured and dynamic approach to variable management and data transmission.

## Future Development

As development progresses, additional features and support for more hardware platforms may be added, ensuring that EasyUART remains a versatile solution for MCU communication.
