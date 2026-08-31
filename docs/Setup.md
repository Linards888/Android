# Folkrace Setup

## 1. Get the project

Open cmd, go to folder where you keep your projects, and clone the repo:

```powershell
git clone https://github.com/Linards888/Android
cd Android
```

The main sketch is `Android.ino`. Project code is grouped under `src/`(hopefully there is no need to go there). and the robot configuration is `config.h`.

## Install ESP32 development tools

### Arduino IDE

1. Install[Arduino IDE](https://github.com/Linards888/Android).
2. Open **File > Preferences** and add this URL to **Additional Boards Manager
   URLs**:

   ```text
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. Open **Tools > board > Boards Manager**, search for **esp32**, and install **esp32 by Espressif Systems**.
4. Connect the EPS32 by USB. Install its USB-to-serial driver if no serial port appiers in **Tools > Ports**.

> [!CAUTION]
> Use an ESP32 board, not an Arduino Uno/Nano: this firmware uses ESP32 features such as Preferences and Bluetooth Low Energy.

### Arduino CLI
> [!NOTE]
> Coming Soon!

### Arduino Community Edition - VScode
> [!NOTE]
> Useful information that users should know, even when skimming content.