# ⏱️ TimerCube

A creative and physical approach to time management using an ESP32, MPU6050 accelerometer, and a capacitive touch sensor. This Pomodoro Timer supports intuitive mode switching by rotating the cube, manual mode selection via touch, and visual feedback through LEDs with a breathing effect.

## 🎯 Features

- 🧭 **Tangible UI with MPU6050**: Tilt the cube to switch modes (Pomodoro, Short Break, Long Break).
- 👆 **Capacitive Touch Control**: Start, pause/resume, reset, or cycle modes without needing MPU6050.
- 🔊 **Buzzer Alerts**: Beeps when the timer completes or responds to user actions.
- 💡 **LED Breathing Effect**: Blinking LEDs while timer is running, solid light when paused or complete.
- 🔁 **Fallback Mode Selection**: Operates fully in touch-only mode if MPU6050 is disconnected.
- 📟 **Serial Output**: Debug and state updates via Serial Monitor.

## 🔧 Mode Mapping (Based on Cube Orientation)

| Cube Face Direction   | Mode         | Duration      |
|-----------------------|--------------|---------------|
| Z-axis Up             | Pomodoro     | 2 minutes     |
| X-axis Positive       | Short Break  | 30 seconds    |
| X-axis Negative       | Long Break   | 1 minute      |

> You can easily change the durations in the `checkMPUOrientation()` or `cycleModeManually()` functions.

## 📲 Touch Controls

| Gesture                | Action                           |
|------------------------|----------------------------------|
| Short Tap (while running) | Pause/Resume Timer            |
| Short Tap (after complete) | Acknowledge Completion      |
| Long Press             | Reset Timer                      |
| Short Tap (Idle without MPU) | Cycle Modes Manually       |

## ⚙️ Hardware Requirements

- ESP32 (any variant, tested with ESP32 Dev Board)
- MPU6050 Accelerometer (I2C)
- 3x LEDs (for Pomodoro, Short Break, Long Break indicators)
- 1x Capacitive Touch Sensor (or touch-capable GPIO)
- 1x Buzzer (optional for audio feedback)
- Resistors as needed

## 🪛 Pin Configuration

| Component         | GPIO Pin |
|------------------|----------|
| Touch Sensor     | GPIO 4   |
| Buzzer           | GPIO 5   |
| Pomodoro LED     | GPIO 18  |
| Short Break LED  | GPIO 19  |
| Long Break LED   | GPIO 23  |
| MPU6050 SDA      | GPIO 21  |
| MPU6050 SCL      | GPIO 22  |

> You can change these values in the `#define` section of the code.

## 🚀 Getting Started

1. **Install Libraries**
    - [`MPU6050`](https://github.com/jrowberg/i2cdevlib) by Jeff Rowberg
    - Wire library (built-in with ESP32 core)

2. **Upload the Sketch**
    - Use [Arduino IDE](https://www.arduino.cc/en/software) with ESP32 board support installed.
    - Select your ESP32 board and port.
    - Upload the code provided in this repository.

3. **Monitor Output**
    - Open Serial Monitor at 115200 baud to see logs and debug info.

## 🧠 Inspiration

This project combines **Human-Computer Interaction (HCI)** principles with **physical computing** to help users stay productive using physical interaction instead of screens or buttons.

## 🤝 Contribution

Feel free to fork this project, add features like:
- BLE Keyboard shortcuts for integration with productivity apps
- OLED display showing time remaining
- Web configuration via WiFi

## 📝 License

This project is open-source under the [MIT License](LICENSE).

---

**Made with ❤️ by Tanzim Ikram Sheikh**
