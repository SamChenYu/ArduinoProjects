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

#include <TFT_eSPI.h>
#include <SPI.h>
#include <math.h>

#include <Adafruit_MLX90614.h> // Temperature
#include <TinyGPSPlus.h> // GPS
#include <MPU9250_asukiaaa.h> // Gyroscope

// LCD PINS
#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     2

#define HUD_W 128
#define HUD_H 160

// LCD
TFT_eSPI tft = TFT_eSPI(); // Uses pins from User_Setup.h
// Temp
Adafruit_MLX90614 temp = Adafruit_MLX90614();
// GPS
TinyGPSPlus gps;
HardwareSerial GPS_Serial(2);  // UART2
// Gyro
MPU9250_asukiaaa gyro;



// Buffered Data
// Buffer holds a bitmap for the display -> constantly clearing screen and redrawing causes flickering
uint16_t hudBuffer[HUD_W * HUD_H]; 

float prevRoll = -1.0;
float currentRoll = 0.0;






void rotatePoint(float x, float y, float angleRad, float &outX, float &outY) {
  outX = x * cos(angleRad) - y * sin(angleRad);
  outY = x * sin(angleRad) + y * cos(angleRad);
}



void drawCrosshair(float rollDeg, uint16_t color) {
  int cx = tft.width() / 2;
  int cy = tft.height() / 2;

  // === Sizes ===
  int r = 5;
  int tailLen = 20;
  int shortTail = 12;

  float rollRad = radians(rollDeg);

  // === Central Ring ===
  tft.drawCircle(cx, cy, r, color);

  // === Horizontal Tails (rotated with roll) ===
  for (int i = 0; i < 2; i++) {
    int sign = (i == 0) ? -1 : 1;
    float dx = sign * (r + tailLen);
    
    float x0, y0, x1, y1;
    rotatePoint(dx, 0, rollRad, x0, y0);
    rotatePoint(dx + sign * tailLen, 0, rollRad, x1, y1);

    tft.drawLine(cx + x0, cy + y0, cx + x1, cy + y1, color);
  }

  // === Rotated short tails (90° apart) ===
  int angles[] = {0, 90, 180, 270};
  for (int i = 0; i < 4; i++) {
    float angleRad = radians(angles[i]) + rollRad;

    float x0 = cx + r * cos(angleRad);
    float y0 = cy + r * sin(angleRad);
    float x1 = cx + (r + shortTail) * cos(angleRad);
    float y1 = cy + (r + shortTail) * sin(angleRad);

    tft.drawLine(x0, y0, x1, y1, color);


    // Draw the outer circle and ticks
    int arc_radius = 30; // Radius of the outer circle
    int tick_len = 5;
    int tick_spacing_deg = 15;

    // === Full Circle ===
    tft.drawCircle(cx, cy, arc_radius, color);

    // === Arc Tick Marks — Top and Bottom semicircles ===
    for (int angle = 0; angle <= 360; angle += tick_spacing_deg) {
      float rad = radians(angle);

      // Outer tick position on circle
      float x_outer = cx + arc_radius * cos(rad);
      float y_outer = cy + arc_radius * sin(rad);

      // Vector from tick to center
      float dx = cx - x_outer;
      float dy = cy - y_outer;
      float dist = sqrt(dx * dx + dy * dy);

      // Normalize and scale to tick length
      float x_inner = x_outer + (dx / dist) * tick_len;
      float y_inner = y_outer + (dy / dist) * tick_len;

      // Draw tick mark
      tft.drawLine((int)x_outer, (int)y_outer, (int)x_inner, (int)y_inner, color);
    }


  }
}





void setup() {


  delay(2000);
  Serial.begin(9600);

  // Configure this based on the current modules
  Wire.begin(21, 22);  // SDA = 21, SCL = 22
  
  // Initialize the display
  // Initialize with type of tab; most 1.8" ST7735S use BLACKTAB
  tft.init();
  tft.fillScreen(ST7735_BLACK);
  // Display text
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(1);
  tft.setCursor(10, 30);
  tft.println("Initializing...");
  tft.println("");
  tft.println("  SamChenYu");
  tft.println("  Technologies");

  // Temp
  temp.begin();

  // GPS
  GPS_Serial.begin(9600, SERIAL_8N1, 16, 17); // We will override these next line

  // Gryo
  gyro.setWire(&Wire);
  gyro.beginAccel();
  gyro.beginGyro();
  gyro.beginMag();

  // Fake Loading Screen

  int x = 10;         // X position of the bar
  int y = 80;         // Y position
  int w = 100;        // Width of the full bar
  int h = 10;         // Height
  int maxSteps = 20;  // Total steps

  for (int i = 0; i <= maxSteps; i++) {
    // Draw border (static, can be moved outside the loop if desired)
    tft.drawRect(x, y, w, h, ST7735_WHITE);
    // Clear the inside of the bar
    tft.fillRect(x + 1, y + 1, w - 2, h - 2, ST7735_BLACK);
    // Calculate width to fill
    int filled = ((w - 2) * i) / maxSteps;
    // Draw the filled portion
    tft.fillRect(x + 1, y + 1, filled, h - 2, ST7735_GREEN);

    delay(200);
  }



  tft.fillScreen(ST7735_BLACK);
}

void loop() {


  /*
  // Temp
  Serial.print("Ambient = ");
  Serial.print(temp.readAmbientTempC());
  Serial.print(" °C\tObject = ");
  Serial.print(temp.readObjectTempC());
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

  float accX = gyro.accelX();
  float accY = gyro.accelY();
  float accZ = gyro.accelZ();
  Serial.print("Gyro X: "); Serial.print(accX);
  Serial.print(" Y: "); Serial.print(accY);
  Serial.print(" Z: "); Serial.println(accZ);
  
  prevRoll = currentRoll;
  currentRoll = atan2(accY, accZ) * 180.0 / PI;
  currentRoll /= 3.0;  // scale down sensitivity by 3×


  if (abs(currentRoll - prevRoll) > 0.05) { // Stability - only redraw when there are more changes
    drawCrosshair(prevRoll, ST7735_BLACK);
    drawCrosshair(currentRoll, ST7735_GREEN);
  }

  delay(50);
}