# ESP Folkrace Control System

<!-- Improved compatibility of back to top link -->
<a id="readme-top"></a>

<!-- PROJECT SHIELDS -->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![Unlicense License][license-shield]][license-url]

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/your_username/esp-folkrace">
    <img src="images/logo.png" alt="Logo" width="80" height="80">
  </a>

<h3 align="center">ESP Folkrace Control System</h3>

  <p align="center">
    Real-time PID control, BLE tuning, and wireless telemetry for ESP-based Folkrace robots.
    <br />
    <a href="https://github.com/your_username/esp-folkrace"><strong>Explore the docs »</strong></a>
    <br /><br />
    <a href="https://github.com/your_username/esp-folkrace">View Demo</a>
    ·
    <a href="https://github.com/your_username/esp-folkrace/issues">Report Bug</a>
    ·
    <a href="https://github.com/your_username/esp-folkrace/issues">Request Feature</a>
  </p>
</div>

---

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#key-features">Key Features</a></li>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#system-overview">System Overview</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>

---

<!-- ABOUT THE PROJECT -->
## About The Project

[![Product Screenshot][product-screenshot]](https://example.com)

This project is a complete control and telemetry system for ESP-based Folkrace robots. It combines real-time PID control, wireless communication, and live BLE tuning into one cohesive setup — so instead of reflashing firmware 50 times to tweak a gain value, you just dial it in from your phone and watch the robot either nail the corner or redecorate the wall (but now scientifically 📈).

The system uses two ESPs: one on the robot running the control logic, and one connected to your PC acting as a telemetry receiver — giving you real-time data logging, plotting, and analysis without any wires trailing behind your robot.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

---

### Key Features

- **PID Control System**  
  Stable, tunable control loop for motor management and line/wall following.

- **Live BLE Tuning**  
  Adjust Kp, Ki, and Kd in real time over Bluetooth Low Energy using your phone or PC — no reflashing needed.

- **Wireless Telemetry (Robot ESP → Receiver ESP32)**  
  Streams sensor data, PID state, and runtime info over radio (ESP-NOW or similar) to a stationary receiver.

- **PC Logging & Plotting**  
  The receiver ESP32 forwards all telemetry to your PC via Serial — store it, plot it, analyze it with whatever tool you prefer.

- **Broad ESP Compatibility**  
  Designed for ESP32. Adaptable to other ESP boards with minor changes.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

---

### Built With

* [ESP32](https://www.espressif.com/en/products/socs/esp32) / ESP platform
* [Arduino Framework](https://www.arduino.cc/) / [PlatformIO](https://platformio.org/)
* Bluetooth Low Energy (BLE)
* [ESP-NOW](https://www.espressif.com/en/products/software/esp-now/overview) wireless communication
* Serial interface for PC data forwarding

<p align="right">(<a href="#readme-top">back to top</a>)</p>

---

<!-- GETTING STARTED -->
## Getting Started

Get your robot running and slightly less chaotic.

### Prerequisites

* [Arduino IDE](https://www.arduino.cc/en/software) or [PlatformIO](https://platformio.org/)
* ESP32 board support installed ([guide here](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html))
* Two ESP32 boards — one for the robot, one for the receiver
* A phone or PC with a BLE app (e.g. [nRF Connect](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-mobile)) for live PID tuning
* Basic understanding of wiring (if not, good luck soldier 🫡)

### Installation

1. Clone the repo
   ```sh
   git clone https://github.com/your_username/esp-folkrace.git
   ```

2. Open the project in Arduino IDE or PlatformIO

3. Flash `robot/` firmware to your **robot ESP32**

4. Flash `receiver/` firmware to your **receiver ESP32** connected to PC

5. Update MAC addresses and config values in `config.h` to match your hardware

6. Power up, connect via BLE, and start tuning

<p align="right">(<a href="#readme-top">back to top</a>)</p>

---

<!-- USAGE -->
## Usage

### BLE PID Tuning

1. Power on the robot
2. Open a BLE app on your phone or PC and connect to `ESP-Folkrace`
3. Write new Kp, Ki, Kd values to the corresponding BLE characteristics
4. Changes apply in real time — no restart required

### Telemetry & Logging

1. Connect the receiver ESP32 to your PC via USB
2. Open a Serial monitor or logging script (e.g. Python with `pyserial`)
3. Run the robot — all sensor and PID data streams live to your PC
4. Plot and analyze with your tool of choice (Serial Plotter, matplotlib, etc.)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

---

<!-- SYSTEM OVERVIEW -->
## System Overview

```
┌─────────────────────────────────┐        ┌──────────────────────────┐
│         Robot ESP32             │        │     Receiver ESP32       │
│                                 │        │                          │
│  ┌──────────┐  ┌─────────────┐  │        │  ┌────────────────────┐  │
│  │  Sensors │→ │ PID Control │  │        │  │  Serial → PC       │  │
│  └──────────┘  └──────┬──────┘  │        │  └────────────────────┘  │
│                       ↓         │        │           ↑              │
│               ┌───────────────┐ │        │  ┌────────────────────┐  │
│               │  ESP-NOW TX   │─┼──────→─┼─ │    ESP-NOW RX      │  │
│               └───────────────┘ │        │  └────────────────────┘  │
│                                 │        └──────────────────────────┘
│  ┌─────────────────────────┐    │                    ↓
│  │  BLE Server (tuning)    │←───┼── Phone / PC   [ PC Logging,
│  │  Kp, Ki, Kd             │    │                   Plotting,
│  └─────────────────────────┘    │                   Analysis ]
└─────────────────────────────────┘
```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

---

<!-- ROADMAP -->
## Roadmap

- [x] PID control loop
- [x] BLE live tuning
- [x] ESP-NOW telemetry to receiver
- [x] PC serial data forwarding
- [ ] Web dashboard for real-time plotting
- [ ] PID preset save/load over BLE
- [ ] OTA firmware updates
- [ ] Support for additional ESP boards

See the [open issues](https://github.com/your_username/esp-folkrace/issues) for the full list of proposed features and known bugs.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

---

<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such a great place to learn, build, and break things responsibly. Any contributions you make are **greatly appreciated**.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#readme-top">back to top</a>)</p>

---

<!-- LICENSE -->
## License

Distributed under the Unlicense License. See `LICENSE` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

---

<!-- CONTACT -->
## Contact

Your Name — [@your_twitter](https://twitter.com/your_twitter) — your@email.com

Project Link: [https://github.com/your_username/esp-folkrace](https://github.com/your_username/esp-folkrace)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

---

<!-- MARKDOWN LINKS & IMAGES -->
[contributors-shield]: https://img.shields.io/github/contributors/your_username/esp-folkrace.svg?style=for-the-badge
[contributors-url]: https://github.com/your_username/esp-folkrace/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/your_username/esp-folkrace.svg?style=for-the-badge
[forks-url]: https://github.com/your_username/esp-folkrace/network/members
[stars-shield]: https://img.shields.io/github/stars/your_username/esp-folkrace.svg?style=for-the-badge
[stars-url]: https://github.com/your_username/esp-folkrace/stargazers
[issues-shield]: https://img.shields.io/github/issues/your_username/esp-folkrace.svg?style=for-the-badge
[issues-url]: https://github.com/your_username/esp-folkrace/issues
[license-shield]: https://img.shields.io/github/license/your_username/esp-folkrace.svg?style=for-the-badge
[license-url]: https://github.com/your_username/esp-folkrace/blob/master/LICENSE
[product-screenshot]: images/screenshot.png
