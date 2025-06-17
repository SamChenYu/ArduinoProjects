#include <Wire.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <math.h>

#include <TFT_eSPI.h> // LCD
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
TFT_eSPI hud = TFT_eSPI(); // Uses pins from User_Setup.h
TFT_eSprite staticHud = TFT_eSprite(&hud); // Contains the static elements so no redraw is needed
TFT_eSprite hudSprite = TFT_eSprite(&hud);  // Sprite tied to display (Kind of like a framebuffer - otherwise would have too much flickering)

// Temp
Adafruit_MLX90614 temp = Adafruit_MLX90614();
// GPS
TinyGPSPlus gps;
HardwareSerial GPS_Serial(2);  // UART2
// Gyro
MPU9250_asukiaaa gyro;
const char* cardinalMap[360] {nullptr}; // Cardinal directions for compass

// Buffered Sensor Data
float currentRoll = 0.0; const float rollNoiseThreshold = 0.1;
float currentPitch = 0.0; const float pitchNoiseThreshold = 3.0;
float currentHeading = 0.0; const float headingNoiseThreshold = 0.5;
float gForce = 0.0;

float ambientTemp = 0.0;
float objectTemp = 0.0;

String gpsStatus = "[--] Searching";
bool gpsSignal = false;
bool gpsHasFix = false; 
double gpsLatitude = 0.0;
double gpsLongitude = 0.0;
double gpsAltitude = 0.0;
int gpsSatellites = 0;

void initStaticHud() {
  staticHud.fillSprite(TFT_BLACK);

  // === Sizes === (Refer to drawCrosshair for more details)
  int cx = staticHud.width() / 2;
  int cy = staticHud.height() / 2;
  int r = 5;
  int color = TFT_GREEN;

  // === Central Ring ===
  staticHud.drawCircle(cx, cy, r, color);

  // Draw the outer circle and ticks
  int arc_radius = 30; // Radius of the outer circle
  int tick_len = 5;
  int tick_spacing_deg = 15;
  // === Full Circle ===
  staticHud.drawCircle(cx, cy, arc_radius, color);
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
    staticHud.drawLine((int)x_outer, (int)y_outer, (int)x_inner, (int)y_inner, color);
  }

  // === Bar Labels for Temp / G Force ===
  int barHeight = 100;
  int barTop = cy - barHeight / 2;
  int tempBarX = 0; // Left Side
  staticHud.setTextSize(1);
  staticHud.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  // Temperature bar ticks and labels (Left side)
  for(int i=0; i<10; i++) {
    int value = 20 + i * 2;
    int y = barTop + barHeight - (i * barHeight) / 10;
    // Tick
    staticHud.drawLine(tempBarX, y, tempBarX + 5, y, TFT_LIGHTGREY);
  }

  int gBarX = 128 - tempBarX; // Right side
  float gMin = -0.5;
  float gMax = 2.0;
  for(int i=0; i<10; i++) {
    float value = gMin + (i * (gMax - gMin) / 10);
    int y = barTop + barHeight - (i * barHeight) / 10;
    staticHud.drawLine(gBarX, y, gBarX - 5, y, TFT_LIGHTGREY);
  }


}

void rotatePoint(float x, float y, float angleRad, float &outX, float &outY) {
  outX = x * cos(angleRad) - y * sin(angleRad);
  outY = x * sin(angleRad) + y * cos(angleRad);
}

void drawCrosshair(float rollDeg, uint16_t color) {
  int cx = hudSprite.width() / 2;
  int cy = hudSprite.height() / 2;

  // === Sizes ===
  int r = 5;
  int tailLen = 20;
  int shortTail = 12;

  float rollRad = radians(rollDeg);

  // === Horizontal Tails (rotated with roll) ===
  for (int i = 0; i < 2; i++) {
    int sign = (i == 0) ? -1 : 1;
    float dx = sign * (r + tailLen);
    
    float x0, y0, x1, y1;
    rotatePoint(dx, 0, rollRad, x0, y0);
    rotatePoint(dx + sign * tailLen, 0, rollRad, x1, y1);

    hudSprite.drawLine(cx + x0, cy + y0, cx + x1, cy + y1, color);
  }

  // === Rotated short tails (90° apart) ===
  int angles[] = {0, 90, 180, 270};
  for (int i = 0; i < 4; i++) {
    float angleRad = radians(angles[i]) + rollRad;

    float x0 = cx + r * cos(angleRad);
    float y0 = cy + r * sin(angleRad);
    float x1 = cx + (r + shortTail) * cos(angleRad);
    float y1 = cy + (r + shortTail) * sin(angleRad);

    hudSprite.drawLine(x0, y0, x1, y1, color);

  }
}

void drawPitchLadders(float pitch, float roll, uint16_t color) {

  int cx = hudSprite.width() / 2;
  int cy = hudSprite.height() / 2;

  const int safeTop = 65;
  const int safeBottom = HUD_H - 40;
  const int rungWidth = 40;
  const int rungSpacing = 10; // Degrees between rungs

  float rollRad = radians(roll);

  for (int angle = -90; angle <= 90; angle += rungSpacing) {
    float yOffset = (angle - pitch) * -2;  // Negative for natural movement
    int yScreen = (int)(cy + yOffset);

    if (yScreen > safeTop && yScreen < safeBottom) {
      // Rung endpoints (relative to center)
      float x0 = -rungWidth / 2, y0 = yOffset;
      float x1 =  rungWidth / 2, y1 = yOffset;

      // Rotate based on roll
      float x0r, y0r, x1r, y1r;
      rotatePoint(x0, y0, rollRad, x0r, y0r);
      rotatePoint(x1, y1, rollRad, x1r, y1r);

      // Translate and draw
      hudSprite.drawLine(cx + x0r, cy + y0r, cx + x1r, cy + y1r, color);

      // Label near right end
      int labelX = cx + x1r + 5;
      int labelY = cy + y1r - 5;
      hudSprite.setTextDatum(TL_DATUM); // Top-left corner for label
      hudSprite.setTextColor(color, TFT_BLACK);
      hudSprite.drawString(String(-angle), labelX, labelY);
    }
  }
}

void drawCompassSlider(float heading) {
  int cx = hudSprite.width() / 2;
  int canvasWidth = 128;

  int spacing = 12;
  int tickInterval = 5;
  int labelInterval = 15;
  int yPos = 10;
  int baseLineY = yPos + 10;
  int tickHeight = 5;

  int edgePadding = 10;
  int leftLimit = edgePadding;
  int rightLimit = canvasWidth - edgePadding;

  hudSprite.fillRect(0, yPos - 15, canvasWidth, 40, TFT_BLACK); // Clear previous area
  // Compass baseline
  hudSprite.drawLine(leftLimit, baseLineY, rightLimit, baseLineY, TFT_GREEN);
  // End Caps
  hudSprite.drawLine(leftLimit, baseLineY - 5, leftLimit, baseLineY + 15, TFT_GREEN);
  hudSprite.drawLine(rightLimit, baseLineY - 5, rightLimit, baseLineY + 15, TFT_GREEN);
  // Degree and alignment Logic
  int headingStep = int(heading) / tickInterval;
  float offsetWithinStep = (int(heading) % tickInterval) / (float) tickInterval;
  float pixelOffset = offsetWithinStep * spacing;
  int centerDeg = headingStep * tickInterval;

  // Cardinal directions map
  hudSprite.setTextColor(TFT_GREEN, TFT_BLACK);
  for(int i=-18; i<=18; i++) {
    int deg = (centerDeg + i * tickInterval) % 360;
    if (deg < 0) deg += 360; // Normalize to 0-359
    int x = cx + i * spacing + pixelOffset;

    if(x > leftLimit && x < rightLimit) {
      if(deg % labelInterval == 0) {
        // Major Tick
        hudSprite.drawLine(x, baseLineY - tickHeight / 2, x, baseLineY + tickHeight, TFT_GREEN);
        // Labels
        if(cardinalMap[deg]) {
          hudSprite.setCursor(x-3, yPos - 8); // Center-align
          hudSprite.print(cardinalMap[deg]);
        }
        hudSprite.setCursor(x-6, baseLineY + 15);
        hudSprite.print(String(deg));
      } else {
        // Minor Tick
        hudSprite.drawLine(x, baseLineY, x, baseLineY + 4, TFT_GREEN);
      }
    }
  }
  
  // Compas Pointer
  int pointerY = baseLineY + 18;
  hudSprite.fillTriangle(
    cx - 6, pointerY + 10,
    cx + 6, pointerY + 10,
    cx, pointerY, TFT_GREEN
  );
  hudSprite.drawTriangle(
    cx - 6, pointerY + 10,
    cx + 6, pointerY + 10,
    cx, pointerY, TFT_BLACK
  );

}

void drawTemperatureBar(float ambient, float object) {
  int barHeight = 100;
  int barTop = hudSprite.height() / 2 - barHeight / 2;
  int tempBarX = 5; // Left Side
  int gBarX = hudSprite.width() - tempBarX; // Right side

  // Ambient Temperature Bar -> Between 10 degrees to 40 degrees
  int ambientY = barTop + (1.0 - constrain((ambient - 10.0) / 30.0, 0.0, 1.0)) * barHeight;
  hudSprite.fillRect(tempBarX - 4, ambientY, 6, barHeight - (ambientY - barTop), TFT_RED);
  
  hudSprite.drawString(String(ambient, 1), 0, 140);
  hudSprite.drawString(String(object, 1), 0, 150);
  
}

void drawGForceBar(float gForce) {
  int barHeight = 100;
  int barTop = hudSprite.height() / 2 - barHeight / 2;
  int gBarX = hudSprite.width() - 15; // Right side

  // G-force Bar
  int gY = barTop + (int)((1.0 - (gForce + 0.5) / 2.5) * barHeight);
  hudSprite.fillRect(gBarX + 9, gY, 5, barHeight - (gY - barTop), TFT_YELLOW);
  hudSprite.drawString(String(gForce, 1), gBarX-5, 140);
}

void drawGPSData() {
  hudSprite.setTextColor(TFT_GREEN, TFT_BLACK);
  hudSprite.setTextSize(1);
  
  if(gpsSignal && gpsHasFix) {
    // Valid data
    hudSprite.drawString(gpsStatus, 25, 130);
    hudSprite.drawString("Lat: " + String(gpsLatitude, 6), 20, 150);
    hudSprite.drawString("Lon: " + String(gpsLongitude, 6), 20, 160);

  } else if(gpsSignal) {
    // Signal but no fix
    hudSprite.drawString(gpsStatus, 25 , 130);
  } else {
    // No signal
    hudSprite.drawString(gpsStatus, 25, 130);
  }
}

void setup() {


  delay(500);
  Serial.begin(9600);

  Wire.begin(21, 22);  // SDA = 21, SCL = 22
  
  // Init the display, show loading screen
  hud.init();
  hud.setRotation(0);
  hud.setViewport(0, 0, HUD_W, HUD_H);
  hud.setSwapBytes(true);
  hud.fillScreen(TFT_BLACK);

  staticHud.createSprite(128,160);
  staticHud.setRotation(0);
  staticHud.setTextColor(TFT_GREEN, TFT_BLACK);  // text color + background
  staticHud.setSwapBytes(true);
  initStaticHud();
  hudSprite.createSprite(128, 160);
  hudSprite.setRotation(0);
  hudSprite.setTextColor(TFT_GREEN, TFT_BLACK);  // text color + background

  // Display text
  hud.setTextColor(TFT_GREEN);
  hud.setTextSize(1);
  hud.setCursor(10, 30);
  hud.println("Initializing...");
  hud.println("");
  hud.println("  SamChenYu");
  hud.println("  Technologies");

  cardinalMap[0] = "N";   cardinalMap[45] = "NE";  cardinalMap[90] = "E";
  cardinalMap[135] = "SE";cardinalMap[180] = "S";  cardinalMap[225] = "SW";
  cardinalMap[270] = "W"; cardinalMap[315] = "NW";

  // Start sensors
  temp.begin();
  GPS_Serial.begin(9600, SERIAL_8N1, 16, 17); // We will override these next line
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
    hud.drawRect(x, y, w, h, TFT_WHITE);
    // Clear the inside of the bar
    hud.fillRect(x + 1, y + 1, w - 2, h - 2, TFT_BLACK);
    // Calculate width to fill
    int filled = ((w - 2) * i) / maxSteps;
    // Draw the filled portion
    hud.fillRect(x + 1, y + 1, filled, h - 2, TFT_GREEN);

    delay(100);
  }


  // Clear the screen after loading
  hud.fillScreen(TFT_BLACK);
}

void loop() {
  // Clear the framebuffer
  hudSprite.fillSprite(TFT_BLACK);

  // === Sensor Readings ===
  // Temp
  ambientTemp = temp.readAmbientTempC();
  objectTemp = temp.readObjectTempC();

  
  // GPS
  while (GPS_Serial.available()) {
    gps.encode(GPS_Serial.read());
  }

  if (!gps.location.isValid()) {
    // GPS location is not valid yet - no fix

    if (gps.satellites.isValid()) {
      // Receiving satellite data but not enough for a fix
      gpsSignal = true;
      gpsHasFix = false;
      gpsStatus = "[..] Locking"; // Acquiring Fix
      gpsSatellites = gps.satellites.value();

    } else {
      // No satellite data yet (cold start)
      gpsSignal = false;
      gpsHasFix = false;
      gpsStatus = "[--] Searching"; // Acquiring Signal
    }
  } else if (gps.location.isUpdated()) {
    // Valid and updated GPS fix
    gpsStatus = "[OK] GPS Locked"; // GPS ACQUIRED
    gpsSignal = true;
    gpsHasFix = true;
    gpsLatitude = gps.location.lat();
    gpsLongitude = gps.location.lng();
    gpsSatellites = gps.satellites.value();

    if (gps.altitude.isValid()) {
      // Altitude data is available valid
      gpsAltitude = gps.altitude.meters();
    }
  } else {
    // Lost GPS signal
    gpsSignal = true;
    gpsHasFix = false;
    gpsStatus = "[!!] Reacquiring"; // Reqcquiring Signal
    gpsSatellites = 0;
  }
  
  // Gyro
  gyro.accelUpdate();
  gyro.gyroUpdate();
  gyro.magUpdate();

  // Heading from magnetometer
  float newHeading = atan2(gyro.magX(), gyro.magY()) * 180 / PI;
  if (newHeading < 0) {
    newHeading += 360;
  }
  if( abs(newHeading - currentHeading) > headingNoiseThreshold ) currentHeading = newHeading;
  //Serial.print("Heading: "); Serial.print(heading); Serial.println("°");
  // G-force
  gForce = sqrt(
    sq(gyro.accelX()) +
    sq(gyro.accelY()) +
    sq(gyro.accelZ()));
  //Serial.print("G-force: "); Serial.println(gforce);

  // Gyro
  float accX = gyro.accelX();
  float accY = gyro.accelY();
  float accZ = gyro.accelZ();
  //Serial.print("Gyro X: "); Serial.print(accX);
  //Serial.print(" Y: "); Serial.print(accY);
  //Serial.print(" Z: "); Serial.println(accZ);
  
  float newRoll = atan2(accY, accZ) * 180.0 / PI;
  if(abs(newRoll - currentRoll) > rollNoiseThreshold) currentRoll = newRoll;
  float newPitch = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180.0 / PI;
  if(abs(newPitch - currentPitch) > pitchNoiseThreshold + 0.1) currentPitch = newPitch;

  // === Draw HUD ===
  hudSprite.pushImage(0,0,128,160, (uint16_t*) staticHud.getPointer()); // Clone the base
  drawTemperatureBar(ambientTemp, objectTemp);
  drawGForceBar(gForce);
  drawPitchLadders(currentPitch, currentRoll, TFT_LIGHTGREY);
  drawCrosshair(currentRoll, TFT_GREEN);
  drawCompassSlider(currentHeading);
  drawGPSData();
  hudSprite.pushSprite(0, 0);

  //delay(50);
}