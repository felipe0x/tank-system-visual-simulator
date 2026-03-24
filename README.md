# SupTanques - SCADA Tank System Simulator

A networked industrial monitoring and supervisory system (SCADA) developed in C++ and the Qt Framework. This project simulates a cyber-physical system consisting of a two-tank plant, pumps, valves, and sensors, controllable via a Client-Server architecture over TCP Sockets.

## Architecture
* Language: C++ (C++11/14)
* GUI Framework: Qt 5 / Qt 6
* Networking: TCP/IP Sockets (custom byte-stream protocol)
* Concurrency: std::thread, std::mutex, Qt signals/slots

## Implementation Details

This project was developed for an academic assignment. The mathematical simulation of the physical plant (fluid dynamics) and the base socket wrappers were provided by the professor. My work focused on implementing the distributed systems logic, concurrency, and GUI integration.

My specific contributions include:

* Server (SupServidor): Implemented the multi-client connection loop using `select()` and socket queues for I/O multiplexing. Handled the parsing of client commands (login, telemetry requests, actuation) and dispatching the plant's state over the network.
* Base Client (SupCliente): Implemented the background thread responsible for asynchronous data polling. Integrated `std::mutex` to protect shared socket resources, preventing race conditions during concurrent read/write operations when the user triggers an actuator.
* Qt Client (SupClienteQt): Integrated the core client logic with the Qt event loop. Used signals and slots to safely trigger UI updates from the background thread and wired the interactive UI components to the network transmission functions.

## Build Instructions and Project Structure

Note on repository structure: For version control and readability, the files in this repository have been organized into `/src`, `/include`, `/ui`, and `/assets` directories. 

However, the native project files (`SupCliente.pro` and `.cbp` files) and the internal `#include` directives expect a flat directory structure. 

To compile and run this project locally:
1. Move all `.cpp`, `.h`, `.ui`, and visual assets (`.png`, `.svg`) into a single root directory.
2. Server: Open `SupServidor.cbp` in Code::Blocks (link the `Ws2_32` library on Windows) or compile the `supservidor*.cpp` files directly.
3. Client: Open the `SupCliente.pro` file in Qt Creator to build the GUI application.
