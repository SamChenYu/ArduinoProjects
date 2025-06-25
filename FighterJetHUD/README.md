# FighterJetHUD

Building a Fighter Jet Style HUD Attachment For My Mountain Bike

<img width="1118" alt="pcb" src="https://github.com/user-attachments/assets/a219efe6-fb62-463f-be7a-58d3aea8bd10" />

## HUD Specifications
- Orientation: Gyroscope-based Artificial Horizon, Pitch Ladders, and Compass Heading
- Temperature: Real-time Ambient and Object Readings
- Acceleration: G-force Measurement
- GPS: Latitude / Longitude, Altitude and Satellite Count


https://github.com/user-attachments/assets/80f926de-409b-4d04-a4e0-312d349b4325





## Sensor Specifications  
- Infrared Temperature Sensor MLX90614  
- 9DOF 9-Axis Accelerometer Gyroscope Magnetic Field Sensor MPU-9250
- GPS Module NEO-M8
- LCD TFT 1.8" ST7735
- DOIT ESP32 DevKit V1

## Sensor Libraries  
- Adafruit_MLX90614  
- MPU9250_asukiaaa 
- TinyGPSPlus  
- TFT_eSPI  (User_Setup.h in /src)

## Sensor & Display Wiring

| Component                         | Signal     | ESP32 Pin | Notes                               |
| --------------------------------- | ---------- | --------- | ----------------------------------- |
| **LCD Module (ST7735)**           | SCLK (SCL) | D18       | SPI Clock                           |
|                                   | MOSI (SDA) | D23       | SPI Data (Master Out Slave In)      |
|                                   | RST        | D4        | Hardware Reset                      |
|                                   | DC (OC)    | D2        | Data/Command Select                 |
|                                   | CS (DS)    | D5        | Chip Select                         |
| **Temperature Sensor (MLX90614)** | SCL        | D22       | I²C Clock                           |
|                                   | SDA        | D21       | I²C Data                            |
| **Gyroscope Sensor (MPU9250)**    | SCL        | D22       | I²C Clock (shared with temp sensor) |
|                                   | SDA        | D21       | I²C Data  (shared with temp sensor) |
| **GPS Module (NEO-M8)**           | TXD        | RX2 (16)  | GPS TX → ESP32 RX2 (receive)        |
|                                   | RXD        | TX2 (17)  | GPS RX ← ESP32 TX2 (transmit)       |

