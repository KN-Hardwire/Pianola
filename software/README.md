# Software

This directory contains the firmware and software source code for the 16-key USB-OTG MIDI keyboard matrix. The build system is managed via PlatformIO, which handles the ESP32-S3 toolchain, Arduino framework, and library dependencies automatically.

## Prerequisites

To build and flash this project, you only need PlatformIO. 

* **PlatformIO Core (CLI):** Recommended for headless, CI, or custom terminal environments.
```bash
    pip install -U platformio
    ```
    *(Alternatively, you can use the PlatformIO IDE extension for VSCode).*

## Building

PlatformIO abstracts away the compiler flags and paths. To compile the project for the target architecture (`esp32-s3`), simply run:

```bash
pio run
```

To clean the build directory:
```bash
pio run -t clean
```

## Flashing & Monitoring

Connect your ESP32-S3 via USB. PlatformIO will typically auto-detect the serial port. 

To build and upload the firmware in one step:
```bash
pio run -t upload
```

To open the serial monitor (configured for 115200 baud):
```bash
pio device monitor
```

*Note: If you need to force a specific port, ensure `upload_port = COM13` (or `/dev/ttyACM0`, etc.) is set in `platformio.ini`.*

## Structure

* `src/`: Main source files (e.g., `main.cpp`).
* `include/`: Custom header files.
* `lib/`: Project-specific (private) libraries.
* `platformio.ini`: The master build configuration file. Defines the environment, board (`esp32-s3-devkitc-1`), memory partitions, and custom build flags.