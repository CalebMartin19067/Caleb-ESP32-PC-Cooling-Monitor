#19067 Caleb Martin, Project: PC Case Ambient Monitor (ESP32-S3-REVERSE TFT)

A compact ambient system using the ESP32-S3 to monitor temperature and humidity inside a PC case. Information is shown on the built-in display, and also seen on a timeline chart via a local web server interface over Wi-Fi.

## Features:

- Real-time temperature and humidity monitoring
- LCD screen display (can be added later)
- Web-based dashboard over local network
- 3D printable housing (optional)
- Designed to help maintain safe PC operating conditions

## Hardware Used:

- ESP32-S3 microcontroller (with built-in screen)
- AHT20+BMP280 temp/humidity sensor
- Optional: 3D printed 120mm fan mount
- Power source via USB C

## How to Run the Project:
- Connect the combined AHT/BMP sensor module to power, SDA and SCL 
- Connect the ESP32-S3 to your PC via USB-C.
- Change the WiFi SSID and password to match your network
- Open the project in PlatformIO
- Upload the code to the board.
- Open the Serial Monitor at 115200 baud to view the IP address.
- Connect to the same WiFi network as the ESP32.
- Open a browser, enter the IP address shown in the Serial Monitor.
- Interact with the web interface to view sensor readings and toggle outputs.

## Attribution
This project uses the following open‑source libraries:
- Arduino Core (Arduino.h) – LGPL 2.1
- ESP32 WiFi (WiFi.h) – LGPL 2.1
- Arduino_JSON – MIT License
- Adafruit BMP280 / Adafruit Sensor – BSD License
- Adafruit AHTX0 – BSD License
- TFT_eSPI – MIT License
All libraries are open source and freely available. They are used here under their respective permissive licenses, which allow free use and modification provided credit is given to the original authors.





