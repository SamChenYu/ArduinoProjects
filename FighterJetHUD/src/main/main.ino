/*
esp32:esp32doit-devkit-v1
Cabling:
  Power: red
  Ground: black
  Clock Line: blue
  Data Line: yellow
*/

#include <Wire.h>
#include <HardwareSerial.h>

#include <Adafruit_MLX90614.h> // Temperature
#include <TinyGPSPlus.h> // GPS
#include <MPU9250_asukiaaa.h> // Gyroscope


// Temp
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// GPS
TinyGPSPlus gps;
HardwareSerial GPS_Serial(2);  // UART2

// Gyro
MPU9250_asukiaaa gyro;

void setup() {


  delay(2000);
  Serial.begin(9600);

  // Configure this based on the current modules
  Wire.begin(21, 22);  // SDA = 21, SCL = 22
  

  //mlx.begin();

  // GPS
  //GPS_Serial.begin(9600, SERIAL_8N1, 16, 17); // We will override these next line


  gyro.setWire(&Wire);
  gyro.beginAccel();
  gyro.beginGyro();
  gyro.beginMag();

  Serial.println("MPU9250 ready");



  
}

void loop() {


  /*
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

  */
  // Gyro
  gyro.accelUpdate();
  gyro.gyroUpdate();
  gyro.magUpdate();

  // Heading from magnetometer
  float heading = atan2(gyro.magX(), gyro.magY()) * 180 / PI;
  if (heading < 0) {
    heading += 360;
  }
  Serial.print("Heading: "); Serial.print(heading); Serial.println("°");
  // G-force
  float gforce = sqrt(
    sq(gyro.accelX()) +
    sq(gyro.accelY()) +
    sq(gyro.accelZ()));
  Serial.print("G-force: "); Serial.println(gforce);

  // Gyro
  Serial.print("Gyro X: "); Serial.print(gyro.gyroX());
  Serial.print(" Y: "); Serial.print(gyro.gyroY());
  Serial.print(" Z: "); Serial.println(gyro.gyroZ());








  delay(1000);
}