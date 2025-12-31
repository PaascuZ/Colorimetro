
# Colorimeter on Basys MX3 (PIC32MX370 + TCS34725)

![Language](https://img.shields.io/badge/language-Embedded%20C-blue)
![MCU](https://img.shields.io/badge/MCU-PIC32MX370F512L-orange)
![Board](https://img.shields.io/badge/Board-Basys%20MX3-purple)
![IDE](https://img.shields.io/badge/IDE-MPLAB%20X-blue)
![Compiler](https://img.shields.io/badge/Compiler-XC32-red)
![Interfaces](https://img.shields.io/badge/Interfaces-I2C%20%7C%20SPI%20%7C%20UART-lightgrey)
![Project](https://img.shields.io/badge/Project-Academic-green)


**Microcontroller Programming Project** – Academic Year 2025/2026  
Development of an **embedded colorimeter** based on **Basys MX3** and the **TCS34725** color sensor, featuring UART interaction, LCD display, Flash memory storage, PWM audio feedback, and interrupt handling.

---

## 📌 Project Overview

This project implements a **colorimeter** capable of detecting the color of a target using the **TCS34725** sensor.  
The **Red, Green, and Blue (RGB)** values are:

- acquired via **I2C**
- displayed on the **LCD** using the **PMP** interface
- processed and classified
- stored in **SPI Flash memory**

At startup, the system presents a **UART-based menu** that allows the user to select the desired functionality.

The implementation fully complies with the official project requirements described in the assignment document :contentReference[oaicite:0]{index=0}.

---

## 🧩 Hardware Used

- **Board**: Basys MX3  
- **Microcontroller**: PIC32MX370F512L  
- **Color Sensor**: TCS34725 (with integrated white LED)
- **Display**: On-board LCD (PMP interface)
- **Speaker**: On-board speaker
- **Push Buttons**: BTNC
- **RGB LED**: On-board RGB LED
- **Interfaces**:
  - UART4
  - I2C
  - SPI
  - PWM (OC1)
  - GPIO
  - Timers

---

## 🔌 Main Connections

| Function | Pin / Interface |
|--------|-----------------|
| UART TX | RF12 (UART4TX) |
| UART RX | RF13 (UART4RX) |
| PWM Speaker | OC1 → RB14 |
| TCS34725 Sensor | I2C header (SDA, SCL, 3V3, GND) |
| Sensor LED | 3V3 (always on during measurements) |
| Push Button | BTNC → INT4 |
| LCD | PMP |
| Flash Memory | SPI |

---

## ⚙️ Clock Configuration

- **External oscillator**: 8 MHz  
- **PLL enabled**
- **SYSCLK** = 80 MHz  
- **PBCLK** = 40 MHz  

All configuration bits are defined in the dedicated `config_bits.h` file.

---

## 🖥️ Implemented Features

### Startup Menu (UART)

At power-up, the following menu is displayed on the serial terminal:
1. Start color scan
2. Show red color count
3. Reset stored colors

---

### 🔴 Function 1 – Start Color Scan

- Generates an audible **beep** on the speaker:
  - PWM frequency: **10 kHz**
  - Duty cycle: **50%**
- Turns on the **white LED** of the TCS34725
- Continuously reads **R, G, B** values from the sensor
- Displays on the LCD:
R: xxxx  
G: xxxx  
B: xxxx
- Performs basic color classification
- When **BTNC** is pressed:
- an **external interrupt (INT4)** is triggered
- the scan is stopped
- the number of times the **red color** was detected is saved to **SPI Flash memory**

---

### 🔁 Function 2 – Show Red Color Count

- Reads the stored value from **Flash memory**
- Prints the value on the **UART terminal**
- Turns on the **red channel** of the RGB LED
- The LED blinks:
- **0.5 s toggle time**
- **N times**, where N is the number of detected red targets

---

### ♻️ Function 3 – Reset Stored Colors

- Completely erases the **Flash memory**
- Resets the color detection counters

---

## 🧠 Software Architecture
````
Colorimeter-BasysMX3/
│
├── firmware/
│   │
│   ├── config/
│   │   └── config_bits.h
│   │
│   ├── inc/
│   │   ├── app.h
│   │   ├── i2c.h
│   │   ├── lcd.h
│   │   ├── pwm.h
│   │   ├── spi_flash.h
│   │   ├── tcs34725.h
│   │   ├── uart.h
│   │   └── utils.h
│   │
│   └── src/
│       ├── app.c
│       ├── i2c.c
│       ├── lcd.c
│       ├── main.c
│       ├── pwm.c
│       ├── spi_flash.c
│       ├── tcs34725.c
│       ├── uart.c
│       └── utils.c
│
├── docs/
│   ├── flowchart.pdf
│   └── report.pdf
│
├── .gitignore
├── README.md
└── LICENSE
````



### Main Modules

- **utils**  
  Core timer utilities, delays, `millis()`
- **uart**  
  Serial communication, menu handling, debugging
- **i2c**  
  I2C master driver
- **tcs34725**  
  Complete color sensor driver
- **lcd**  
  LCD control via PMP
- **pwm**  
  Speaker control using OC1 and Timer
- **spi_flash**  
  Flash read/write/erase routines
- **app**  
  Application logic and menu management
- **main**  
  Hardware initialization and main loop

---

## 🧪 Testing Methodology

- Target: printed sheet with **colored circles**
- White LED of the sensor enabled before each measurement
- System output verified through:
  - **UART terminal (PuTTY)**
  - LCD display
  - RGB LED behavior
  - Speaker audio feedback

---

## 📦 Software Requirements

- **MPLAB X IDE**
- **XC32 Compiler**
- Serial terminal software (e.g. PuTTY)

---

## 👤 Author

Developed for the **Microcontroller Programming** course  
Basys MX3 – PIC32MX370F512L

---

## 📜 License

MIT License.




