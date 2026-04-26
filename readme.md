# Project 2

> This document explains clearly (hopefully) my version of the project_2 of Telemetry Recruitment for E-Agle TRT ASD.

## Setup 

This project was setup using the instruction from the projects readme.

### Github

The github repository was setup by initially making a branch 'dev' where all the project would be saved at before it going to the master bracnh, after that an issue was made for each important part of the projects to ensure its completion by doing small steps and having a better history of the project, for each issue a branch was made in order to resolve them in their respective on. After each issue got resolved a PR was created and if it was all OK a the branch would get merged into 'dev'.

## Project Architecture & File Structure

### 'main.cpp'

The entry point of the application. It implements the core Finite State Machine (FSM) to manage the telemetry session lifecycle (IDLE and RUN states). It handles real-time file I/O for raw logging and coordinates the statistical data aggregation.

### 'can_handler.cpp' & c'an_handler.h'

Encapsulates the multithreaded receiver logic. It manages a dedicated background thread that continuously polls the simulated CAN interface. It utilizes a 'mutex' and a 'condition_variable' to implement a thread-safe producer-consumer pattern, preventing data races when passing messages to the main thread.

### 'parser.cpp' & 'parser.h'

Provides the payload decoding logic. It parses the raw CAN strings (e.g., '<ID>#<PAYLOAD>') and converts the hexadecimal characters into a structured ParsedMessage struct, separating the 12-bit ID from the data bytes.

## Core Functions

### 'start_condition_met' & 'stop_condition_met'

Boolean helper functions that evaluate incoming parsed messages. They trigger the FSM state transitions by checking for specific hexadecimal IDs and payload sequences

### 'update_stat'

Updates a map tracking structure during the RUN state. It continuously increments the message counter for each unique CAN ID received in the current session.

### 'save_csv'

Triggered upon transitioning to the STOP state. It calculates the mean time interval (in milliseconds) between messages for each tracked ID. It then exports these stats to a formatted .csv file with hexadecimal IDs to be able to see the data from such fiel.