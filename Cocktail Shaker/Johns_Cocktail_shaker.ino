#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

SoftwareSerial mySerial(10, 11); // RX, TX for DFPlayer Mini
DFRobotDFPlayerMini myDFPlayer;

const int vibrationPin = 2;  // Pin connected to vibration sensor output
const int redLedPin = 4;     // Pin connected to Red LED
const int yellowLedPin = 8;  // Pin connected to Yellow LED
const int greenLedPin = 12;  // Pin connected to Green LED

int playing = 0;

unsigned long vibrationTime = 0;
unsigned long elapsedTime = 0;
unsigned long interruptTimeStart = 10000;
unsigned long lastVibrationDetected = 0;
unsigned long ignoreNoVibrationUntil = 0;

int tracks[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
int interruptTracks[] = {19, 20, 21, 22};

bool musicPlaying = false;
bool interruptPlaying = false;
bool shouldInterrupt = false;
bool yellowLedOn = false;
bool greenLedOn = false;

bool finishedShakingPeriod = false;
unsigned long finishedShakingTime = 0;

const unsigned long ignorePeriod = 1000;  // Ignore "no vibration" for 500 ms

void setup() {
  Serial.begin(9600);    // Initialize Serial Monitor for debugging
  mySerial.begin(9600);  // Initialize SoftwareSerial for DFPlayer Mini

  randomSeed(analogRead(A0)); // seed a random number
  
  pinMode(vibrationPin, INPUT);  // Set vibration sensor pin as input
  pinMode(redLedPin, OUTPUT);    // Set Red LED pin as output
  pinMode(yellowLedPin, OUTPUT); // Set Yellow LED pin as output
  pinMode(greenLedPin, OUTPUT);  // Set Green LED pin as output

  Serial.println("Initializing DFPlayer Mini...");
  
  if (!myDFPlayer.begin(mySerial)) {
    Serial.println("DFPlayer Mini initialization failed!");
    while(true);  // Stop execution if initialization fails
  }

  Serial.println("DFPlayer Mini initialized successfully!");
  
  myDFPlayer.volume(20);    // Set volume level (0-30)
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  ignoreNoVibrationUntil = millis() + 9999999999;
}

void loop() {
  bool vibrationDetected = digitalRead(vibrationPin); // Read the vibration sensor
  
  unsigned long currentTime = millis();

  if(finishedShakingPeriod) {
    digitalWrite(greenLedPin, HIGH);
    if(currentTime - finishedShakingTime >= 10000) {
      finishedShakingPeriod = false;
    }

    return;
  }





  if (vibrationDetected) {
    lastVibrationDetected = currentTime;  // Update last vibration time
    ignoreNoVibrationUntil = currentTime + ignorePeriod;  // Extend ignore period
    Serial.println("Vibration detected!");
    handleVibrationDetected();
  } else if (!vibrationDetected && currentTime > ignoreNoVibrationUntil) {
    // Only handle no vibration if the ignore period has passed
    Serial.println("No Vibration detected.");
    handleNoVibration();
  }

      Serial.println("playing track ");
      Serial.println(playing);

}

void handleVibrationDetected() {
  if (!musicPlaying) {
    startSequence();
  }

  elapsedTime = millis() - vibrationTime;
  Serial.print("Elapsed Time: ");
  Serial.println(elapsedTime);

  if (elapsedTime >= 15000 && !yellowLedOn) {
    digitalWrite(redLedPin, LOW);
    digitalWrite(yellowLedPin, HIGH);
    yellowLedOn = true;
    Serial.println("Yellow LED ON");
  }

  if (elapsedTime >= 30000 && !greenLedOn) {
    digitalWrite(yellowLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
    myDFPlayer.stop(); // Stop the music
    greenLedOn = true;
    Serial.println("Green LED ON and Music Stopped");
  }

  if (elapsedTime >= 40000) {
    finishedShakingPeriod = true;
    finishedShakingTime = millis();
    resetSystem();
  }
}

void handleNoVibration() {
  shouldInterrupt = musicPlaying;
  resetSystem();
  handleInterruptTrack();
}

void startSequence() {
  if (interruptPlaying) {
    myDFPlayer.stop(); // Stop the interrupt track
  }

  vibrationTime = millis(); // Record the time when vibration is detected
  Serial.println("Vibration detected! Starting sequence...");

  digitalWrite(redLedPin, HIGH); // Turn on the red LED
   playing = tracks[random(0,19)];
  myDFPlayer.play(playing); // Start playing the track
  musicPlaying = true;
  yellowLedOn = false; // Reset Yellow LED flag
  greenLedOn = false;  // Reset Green LED flag
}

void handleInterruptTrack() {
  if (shouldInterrupt) {
    myDFPlayer.stop();
    musicPlaying = false;
    playing = interruptTracks[random(0,3)];
    myDFPlayer.play(playing);
    shouldInterrupt = false;
    elapsedTime = 0;
    interruptTimeStart = millis();
  } else {
    if (millis() - interruptTimeStart >= 20000) {
      myDFPlayer.stop();
      resetSystem();
      musicPlaying = true; // forces the interrupt
      
    }
  }
}

void resetSystem() {
  // Reset flags and turn off LEDs
  digitalWrite(redLedPin, LOW);
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(greenLedPin, LOW);
  
  vibrationTime = 0;
  musicPlaying = false;
  yellowLedOn = false;
  greenLedOn = false;
}
