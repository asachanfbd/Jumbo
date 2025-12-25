# Jumbo - IoT Smart Assistant Display

Jumbo is an IoT-enabled smart display based on the ESP8266 (NodeMCU). It brings a personality to your desk by displaying animated facial expressions ("Eyes") and text messages fetched from a remote API. It also features sound feedback using an active buzzer and a power-saving standby mode.

## Features

- **Animated Eyes**: Expressive eye animations (Happy, Sad, Shocked, Sleep, etc.) displayed on an OLED screen.
- **Remote Control**: Fetches command sequences (expressions, text, beeps) from a configured API endpoint.
- **Standby Mode**: Toggle display and activity on/off using the built-in Flash button (GPIO 0).
- **Audio Feedback**: Simple beeps and tones using an active buzzer.
- **WiFi Connected**: Automatically connects to configured WiFi and performs network tasks.

## Hardware Requirements

To build Jumbo, you will need the following components:

1.  **NodeMCU v2 (ESP8266)**: The main microcontroller.
2.  **SSD1306 OLED Display**: 0.96" 128x64 Pixel I2C OLED Module.
3.  **Active Buzzer**: For sound effects (supports simple HIGH/LOW or PWM sequences).
4.  **Jumper Wires & Breadboard**: For connections.
5.  **Micro-USB Cable**: For power and programming.

## Pin Configuration / Wiring

Connect the components as follows:

### OLED Display (I2C)

| OLED Pin | NodeMCU Pin | GPIO | Function |
| :--- | :--- | :--- | :--- |
| **VCC** | 3.3V (or Vin) | - | Power |
| **GND** | GND | - | Ground |
| **SCL** | **D1** | GPIO 5 | I2C Clock |
| **SDA** | **D2** | GPIO 4 | I2C Data |

### Active Buzzer

| Buzzer Pin | NodeMCU Pin | GPIO | Function |
| :--- | :--- | :--- | :--- |
| **VCC** | 3.3V (or Vin) | - | Power |
| **GND** | GND | - | Ground |
| **I/O (Signal)** | **D5** | GPIO 14 | Signal (PWM) |

### Input Control

| Component | NodeMCU Pin | GPIO | Function |
| :--- | :--- | :--- | :--- |
| **Flash Button** | **D3** / Flash | GPIO 0 | Toggle Sleep/Resume |

*Note: The **Flash Button** is usually the built-in button on the NodeMCU board near the USB port (often labeled "FLASH"). You can use this directly without external wiring.*

## Setup & Configuration

1.  **Clone the Repository**:
    ```bash
    git clone <repository-url>
    cd Jumbo
    ```

2.  **Configure Credentials**:
    - Copy `src/Config.h-Sample` to `src/Config.h`.
    - Open `src/Config.h` and update the settings with your own WiFi and API details.
    
    *Note: `src/Config.h` is ignored by git to keep your credentials safe.*

## Flashing the Code

This project uses **PlatformIO** for dependency management and building.

### using PlatformIO Core (CLI)

1.  **Install PlatformIO**: If you haven't already, install PlatformIO Core.
2.  **Connect Device**: Plug your NodeMCU into your computer via USB.
3.  **Build and Upload**:
    Run the following command in the project root:
    ```bash
    pio run -t upload
    ```
4.  **Monitor Output** (Optional):
    To see debug logs and connection status:
    ```bash
    pio device monitor
    ```

### using VS Code (PlatformIO IDE)

1.  Open the project folder in VS Code.
2.  Wait for PlatformIO to initialize.
3.  Click the **PlatformIO Alien Head** icon in the sidebar.
4.  Under **Project Tasks** > **nodemcuv2** > **General**, click **Upload**.

## Usage Instructions

1.  **Power On**: Connect the device to power.
2.  **Boot Sequence**: The screen will show "Booting...", connect to WiFi, and then fetch initial data from the API.
3.  **Active Mode**:
    - The device will display expressions and text messages as received from the API.
    - It periodically polls the API for new "Sequences".
4.  **Standby (Sleep) Mode**:
    - **Enter Sleep**: Press the **Flash Button** once. The eyes will close (Sleep expression), and "Sleeping..." will appear.
    - **Resume / Wake**: Press the **Flash Button** again. The device will wake up and resume normal operation.

## API Integration

The device expects a JSON response from the API containing a list of steps. Each step defines the expression to show, text to display, buzzer duration, and how long to hold the step.

**Example JSON Response:**
```json
[
  {
    "Expression": "happy",
    "Text": "Hello World!",
    "BuzzerDuration": 0.1,
    "DisplayDuration": 3.0
  },
  {
    "Expression": "shocked",
    "Text": "I am alive?",
    "BuzzerDuration": 0.5,
    "DisplayDuration": 2.0
  }
]
```
