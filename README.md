# EasyUART

**DISCLAIMER**: I'm just starting out. This was an idea that came to mind and seems like a fun project.

**EasyUART** will be a portable library designed for MCU communication via UART. This library aims to simplify the process of sending and receiving real-time data between embedded systems and a PC interface.

## Features

- **User-Friendly GUI**: EasyUART will feature an intuitive graphical user interface (GUI) for live data visualization and plotting of variables, making it easy to monitor and analyze variables in real time.
  
- **Custom Lightweight Protocol**: The library will includesa custom protocol that supports multiple speeds, allowing users to send certain messages faster than others.

- **Database Generation Program**: Users will be able to create a database defining the variables to be sent and their respective transmission speeds. The database generation tool shall:
  - Limit the number of variables based on the selected baud rate and data types.
  - Configure the target hardware (initially supporting STM32) while remaining hardware agnostic.
  - Generate the `EasyUART.c` and `EasyUART.h` files for easy integration into your application, providing the user with access to public methods for sending and receiving messages.

- **Bidirectional Communication**: While the primary focus is on transmitting data from the MCU to the PC, EasyUART shall allow for bidirectional communication, enabling users to send commands, setpoints, and other control signals from the PC to the MCU.

## Usage Considerations

The amount of data and speed that can be logged or plotted will depend on the chosen baud rate, which is limited by the quality of the hardware setup, including the MCU chosen, the UART to USB converter and wiring. Setting the baud rate too high may result in message loss, so choose wisely.

## Goals

- To provide a flexible, lightweight, and fast UART communication library that can be adapted to various MCU platforms, so it should not consume excessive RAM or FLASH
- To enable dirt cheap debugging through an easy-to-use GUI, eliminating the need for an expensive debugger. However, if you have one, check out [klonyyy's MCUViewer](https://github.com/klonyyy/MCUViewer).
- It should be initialized with a single line of code such as `init_EasyUART();`, and run with somehting like a single `run_EasyUART();` call in your code.
- It should be as easy to use as `send_EasyUART(variable, VARIABLE_ID);` or `variable = read_EasyUART(VARIABLE_ID);`

## Future Development

As development progresses, additional features and support for more hardware platforms will be added.
