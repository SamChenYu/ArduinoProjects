/*
esp32:esp32doit-devkit-v1
Cabling:
  Power: red
  Ground: black
  Clock Line: blue
  Data Line: yellow
*/

#include <Wire.h>
#include <Adafruit_MLX90614.h>

#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// Temp
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// GPS
TinyGPSPlus gps;
HardwareSerial GPS_Serial(2);  // UART2


void setup() {
  Serial.begin(9600);

  // Temp
  Wire.begin(21, 22);  // SDA = 21, SCL = 22

  // GPS
  GPS_Serial.begin(9600, SERIAL_8N1, 16, 17); // We will override these next line



  mlx.begin();
}

void loop() {
  // Temp
  Serial.print("Ambient = ");
  Serial.print(mlx.readAmbientTempC());
  Serial.print(" °C\tObject = ");
  Serial.print(mlx.readObjectTempC());
  Serial.println(" °C");
  
  // GPS
  while (GPS_Serial.available()) {
    gps.encode(GPS_Serial.read());
  }

  if (!gps.location.isValid()) {
    if (gps.satellites.isValid()) {
      Serial.print("Searching for GPS fix... Satellites in view: ");
      Serial.println(gps.satellites.value());
    } else {
      Serial.println("Waiting for satellite data...");
    }
  } else if (gps.location.isUpdated()) {
    Serial.print("✅ GPS Fix Acquired!\nLat: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print("  Lon: ");
    Serial.println(gps.location.lng(), 6);

    Serial.print("Satellites used: ");
    Serial.println(gps.satellites.value());

    if (gps.altitude.isValid()) {
      Serial.print("Altitude (m): ");
      Serial.println(gps.altitude.meters());
    }
  }


  delay(1000);
}
