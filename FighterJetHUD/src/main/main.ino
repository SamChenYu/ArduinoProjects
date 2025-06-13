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

#include <Adafruit_GFX.h> // Graphics library for displays
#include <Adafruit_ST7735.h>
#include <SPI.h>


#include <Adafruit_MLX90614.h> // Temperature
#include <TinyGPSPlus.h> // GPS
#include <MPU9250_asukiaaa.h> // Gyroscope


// Pins for LCD
#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     2

// SPI-based display (no MISO needed)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Temp
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// GPS
TinyGPSPlus gps;
HardwareSerial GPS_Serial(2);  // UART2

// Gyro
MPU9250_asukiaaa gyro;

// LCD
// Pins
#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     2




void drawCrosshair() {
  int cx = tft.width() / 2;
  int cy = tft.height() / 2;
  int r = 6;
  int gap = 0;
  int len = 10;

  tft.drawCircle(cx, cy, r, ST77XX_GREEN);
  tft.drawLine(cx, cy - r - gap - len, cx, cy - r - gap, ST77XX_GREEN);     // Top
  tft.drawLine(cx, cy + r + gap, cx, cy + r + gap + len, ST77XX_GREEN);     // Bottom
  tft.drawLine(cx - r - gap - len, cy, cx - r - gap, cy, ST77XX_GREEN);     // Left
  tft.drawLine(cx + r + gap, cy, cx + r + gap + len, cy, ST77XX_GREEN);     // Right
}






void setup() {


  delay(2000);
  Serial.begin(9600);

  // Configure this based on the current modules
  Wire.begin(21, 22);  // SDA = 21, SCL = 22
  
  // Initialize the display



  // Initialize with type of tab; most 1.8" ST7735S use BLACKTAB
  tft.initR(INITR_BLACKTAB);  // Try INITR_GREENTAB or INITR_REDTAB if display looks off

  tft.setRotation(1);         // 0–3, try different if upside down
  tft.fillScreen(ST77XX_BLACK);

  // Display text
  // tft.setTextColor(ST77XX_GREEN);
  // tft.setTextSize(2);
  // tft.setCursor(10, 30);
  // tft.println("Hello, World!");
  tft.setCursor(60,80);
  drawCrosshair();











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