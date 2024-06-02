# Calculator using RTOS

## Project Overview

This project is a simple calculator application built using Real-Time Operating System (RTOS) principles. 
The calculator performs basic arithmetic operations and demonstrates RTOS concepts like task scheduling, internal timers, queues and semaohores.

## Features

- **User Input**: Enter operations via a keypad. Inputs are displayed on an LCD in real-time.
- **Calculation**: The `CalcTask` processes the equation and calculates the result.
- **Timeout Handling**: If the user delays more than 5 seconds between inputs, the `ClearTask` clears the equation from LCD and frees up the queue.
- **LCD Display**: The LCD shows the equation on the first line and the current time on the second line.


### Prerequisites

- A development environment with RTOS support.
- Basic knowledge of C programming.
- good knowledge at Microcontroller Peritherals Interface of ATMEGA32

