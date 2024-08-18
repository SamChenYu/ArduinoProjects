#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

SoftwareSerial mySerial(10, 11); // RX, TX for DFPlayer Mini
DFRobotDFPlayerMini myDFPlayer;

const int vibrationPin = 2;  // Pin connected to vibration sensor output
const int redLedPin = 3;     // Pin connected to Red LED
const int yellowLedPin = 4;  // Pin connected to Yellow LED
const int greenLedPin = 5;   // Pin connected to Green LED

unsigned long vibrationTime = 0;
bool musicPlaying = false;
bool yellowLedOn = false;
bool greenLedOn = false;

void setup() {
  Serial.begin(9600);    // Initialize Serial Monitor for debugging
  mySerial.begin(9600);  // Initialize SoftwareSerial for DFPlayer Mini
  
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
}

void loop() {
  bool vibrationDetected = digitalRead(vibrationPin); // Read the vibration sensor
  
  if (vibrationDetected) {
    if (!musicPlaying) {
      // Vibration detected and music is not playing yet
      vibrationTime = millis(); // Record the time when vibration is detected
      Serial.println("Vibration detected! Starting sequence...");
      
      // Turn on the red LED
      digitalWrite(redLedPin, HIGH);
      
      // Start playing the track
      myDFPlayer.play(1);
      musicPlaying = true;
      yellowLedOn = false;  // Reset Yellow LED flag
      greenLedOn = false;   // Reset Green LED flag
    }
    
    unsigned long currentMillis = millis(); // Continuously update current time
    unsigned long elapsedTime = currentMillis - vibrationTime;

    // Debugging
    Serial.print("Elapsed Time: ");
    Serial.println(elapsedTime);

    // Turn on the Yellow LED after 10 seconds
    if (elapsedTime >= 10000 && !yellowLedOn) {
      digitalWrite(yellowLedPin, HIGH);
      yellowLedOn = true;
      Serial.println("Yellow LED ON");
    }

    // Turn on the Green LED after 20 seconds and stop the music
    if (elapsedTime >= 20000 && !greenLedOn) {
      digitalWrite(greenLedPin, HIGH);
      myDFPlayer.stop(); // Stop the music
      greenLedOn = true;
      Serial.println("Green LED ON and Music Stopped");

      // Reset the system to allow for the next sequence
      delay(2000); // Optional: Delay to allow visual confirmation of the Green LED
      resetSystem();
    }
  } else {
    // If no vibration detected and music has stopped, ensure LEDs are reset
    if (!musicPlaying) {
      digitalWrite(redLedPin, LOW);
      digitalWrite(yellowLedPin, LOW);
      digitalWrite(greenLedPin, LOW);
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
  
  Serial.println("System reset. Waiting for vibration...");
}
