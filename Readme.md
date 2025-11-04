# Smart Lighting System using RD03_v2 and MQTT on Ai-Thinker WB2

## Overview
This project implements an **automatic lighting system** for **Smart Homes** using the **RD03_v2 radar motion sensor**, the **Ai-Thinker WB2 board** and the **MQTT protocol**. 

It provides **real-time motion detection**, **light control**, and **status feedback** over Wi-Fi via MQTT communication.

The system runs on **FreeRTOS**, enabling concurrent tasks for UART handling, MQTT communication, and timer-based automation.

---

## System Architecture
+----------------------+       +----------------------+  
|    RD03_v2 Sensor    | ----> |     Ai-Thinker WB2   |  
|  (Motion Detection)  |       |  (FreeRTOS + MQTT)   |  
+----------------------+       +----------------------+  
            |  
            | MQTT over Wi-Fi  
            v  
       +----------------------+  
       |     MQTT Broker      |  
       |   (Mosquitto Server) |  
       +----------------------+  
            ^  
            |  
       +----------------------+  
       |  MQTT Dashboard/App  |  
       |   (MQTTX / Node-RED) |  
       +----------------------+  



---

## Key Features
-  **Motion-based light control** — LED automatically turns ON/OFF when motion is detected or lost.  
-  **MQTT integration** — Publishes presence and system status to an MQTT broker.  
-  **UART communication** — Handles RD03 data frames and control commands.  
-  **FreeRTOS timers** — Real-time scheduling for smooth and responsive control.  
-  **Command acknowledgment** — Publishes feedback for open/close/config commands.

---

##  Hardware Components
| Component | Description |
|------------|-------------|
| **Ai-Thinker WB2** | Wi-Fi MCU module based on BL602 |
| **RD03_v2 Sensor** | Radar-based motion detector (UART interface) |
| **LED** | Output control for lighting |
| **Power Supply (5V)** | System power input |

---

## Hardware Connections

| RD03_v2 Pin | WB2 Pin | Function |
|--------------|----------|----------|
| TX | GPIO3 | UART RX0 |
| RX | GPIO4 | UART TX0 |
| VCC | 3.3V | Power |
| GND | GND | Ground |
| LED | GPIO14 | Output control |

---

##  Firmware Logic

### 1. **UART Task**
- Initializes two UARTs:
  - UART0 for RD03 sensor
  - UART1 for debugging logs  
- Parses RD03 frames and triggers light control logic.
- Uses FreeRTOS timer for periodic UART data checking (every 10 ms).

### 2. **MQTT Communication**
- Connects to:
mqtt:"your mqtt"


- Username: `Your username`  
- Password: `Your password`

| Topic | Direction | Purpose |
|--------|------------|----------|
| `/topic/pre/status/` | Publish | Send presence/light status |
| `/topic/pre/ack/` | Publish | Send acknowledgment or sensor events |
| `/topic/pre_test/qos0` | Subscribe | Receive configuration commands |

### 3. **Smart Light Logic**
| Event | Action | MQTT Message |
|--------|---------|--------------|
| Motion detected | LED ON | `"status: Have Presence"` |
| No motion | LED OFF | `"status: OFF"` |
| Command: `open sensor` | Enable detection | `"ack: open success"` |
| Command: `close sensor` | Disable detection | `"ack: close success"` |
| Config success | Update settings | `"ack: config success"` |

---

##  MQTT Example Messages

### WB2 → Broker
{
"topic": "/topic/pre/status/",
"message": "status: Have Presence"
}

{
  "topic": "/topic/pre/ack/",
  "message": "Trigger F1:45.20"
}
### Broker → WB2
FD FF 01 00 02 00 20 ...
(hex command for configuration or control) 

---

## Project Structure

martlight-wb2-mqtt/  
│  
├── include  
│   ├── mqtt_demo.h  
│   ├── wifi_demo.h
├── main.c              
├── mqtt_demo.c  
├── wifi_demo.c       
├── Makefile              
├── bouffalo.mk          
├── proj_config.mk        
└── README.md
        


---

## Prerequisites
This firmware requires the Ai-Thinker WB2 SDK, which includes:

Bouffalo Lab toolchain

FreeRTOS and Wi-Fi drivers

MQTT, UART, and GPIO libraries

---

## Download SDK
Please clone the official SDK from Ai-Thinker to build and flash the project:  

GitHub Repository: [Ai-Thinker-Open/Ai-Thinker-WB2](https://github.com/Ai-Thinker-Open/Ai-Thinker-WB2)  

Documentation: [Ai-WB2-12F Kit Specification (PDF)](https://docs.rs-online.com/f055/A700000010207959.pdf)  

Reference Manual: [Bouffalo Lab BL602/BL604 Reference Manual (PDF)](https://files.pine64.org/doc/datasheet/Pinenut/Bouffalo%20Lab%20BL602_Reference_Manual_en_1.1.pdf)  

Sensor Protocol: [RD-03_v2 UART Communication Protocol (PDF)](https://vdoc.ai-thinker.com/_media/rd-03_v2_serial_communication_en_1_.pdf)  



---

## Future Improvements
Add LDR sensor for adaptive brightness.

Enable web dashboard via MQTT WebSocket.

Support OTA firmware update.

Extend for multi-room lighting network.

---

👨‍💻 Author
Son Truong.
Embedded Systems & IoT Developer  
📧 [truongcongsonbh@gmail.com]  
🏷️ Project: Smart Lighting System using RD03_v2 and MQTT on Ai-Thinker WB2