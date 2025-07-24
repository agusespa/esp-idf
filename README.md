# ESP-IDF Project Collection

This repository contains a collection of ESP-IDF projects demonstrating the use of the ESP32 microcontroller. 
Each sub-project is a self-contained example that can be built and flashed independently.

## Project List

* **wifi_sta**: how to connect to a wifi network (sta mode).
* **http**: how to start up a basic http server.
* **mqtt**: how to set up a mqtt broker.
* **config**: how to read the microcontroller data.
* **pin_io**: collection of projects implementing io pins.
    * **button_led**: basic GPIO control with a button and an LED.
    * **gyro-accel**: how to read data from a gyroscope and accelerometer sensor.
    * **motor**: how to control a DC motor.
    * **motor-encoder**: expands on the motor example by adding an encoder for position feedback.
    * **ultrasonic**: implements a distance sensor using an ultrasonic module.

### Hardware Simulations
- button_led: https://wokwi.com/projects/421682705203208193
- ultrasonic: https://wokwi.com/projects/425941668822970369
- gyro-accel: https://wokwi.com/projects/428034659779638273

## How to Use
1. Install esp-idf: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/linux-macos-setup.html#.
2. Set up the environment variables for the terminal instance by running . $HOME/esp/esp-idf/export.sh.
3. Set up project target with idf.py set-target esp32 and configuration with idf.py menuconfig
4. Run idf.py clean to clean the previously built artifacts.
5. Build the project by running idf.py build.
6. Connect the device and get the port by running ls /dev/cu.*.
7. Flash the software into the device by running idf.py -p PORT flash.

