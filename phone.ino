#include <MP3_Module.h>
#include <SoftwareSerial.h>

#define button1 4   
#define button2 5 // issue
#define button3 6
#define button4 7
#define button5 8
#define button6 9
#define button7 10
#define button8 11
#define button9 12
#define button10 A0

MP3 mp3;

int input;
unsigned long len = 0;
uint8_t i = 0;

const int audrey = 1;   // uint8_t i = 1;
const int darrel = 2; // 2
const int dylan = 4;  // 3
const int ean = 8;    // 4
const int henri = 16 ;    // 5
const int imm = 32;      // 6
const int jerald = 64; // 7 
const int rayner = 128; // 8 
const int maxx = 256; // 9
const int sam = 512;    //11



const int duckSquad = 1+2;// 10
const int amos = 2+4; // 12
const int eanhuncle = 4+8; //13 
const int hdb = 8+16;  // 14
const int kurt = 16+32 ; // 15
const int wangan = 32+64; // 16
const int aomvara = 64+128; // 17
const int panties = 128 + 256; //18
// const int junray = 256 + 512;

void setup() {

  Serial.begin(9600);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
  pinMode(button5, INPUT_PULLUP);
  pinMode(button6, INPUT_PULLUP);
  pinMode(button7, INPUT_PULLUP);
  pinMode(button8, INPUT_PULLUP);
  pinMode(button9, INPUT_PULLUP);
  pinMode(button10,INPUT_PULLUP);
  
  mp3.begin();
  mp3.volume(20); // Set max volume (1-30)
  

}

void loop(){

  
  len = 0;
  int input1 = !digitalRead(button1)*1 + !digitalRead(button2)*2 + !digitalRead(button3)*4 + !digitalRead(button4)*8 + !digitalRead(button5)*16
   + !digitalRead(button6)*32 + !digitalRead(button7)*64 + !digitalRead(button8)*128 + !digitalRead(button9)*256 + !digitalRead(button10)*512;
  
  if(input1 != 0) {
      delay(500);
      int input2 = !digitalRead(button1)*1 + !digitalRead(button2)*2 + !digitalRead(button3)*4 + !digitalRead(button4)*8 + !digitalRead(button5)*16
      + !digitalRead(button6)*32 + !digitalRead(button7)*64 + !digitalRead(button8)*128 + !digitalRead(button9)*256 + !digitalRead(button10)*512;
      
      if(input1 != input2) {
          input = input1 + input2;
      } else { input = input1; }
  } else { input = input1; }
  


  Serial.println(input);

  switch(input) {
    case(audrey):
      i = 1;
      len = 4000;
      break;

    case(darrel):
      i = 2;
      len = 6000;
      break;

    case(dylan):
      i = 3;
      len = 4000;
      break;

    case(ean):
      i = 4;
      len = 3000;
      break;

    case(henri):
      i = 5;
      len = 4000;
      break;

    case(imm):
      i = 6;
      len = 10000;
      break;

    case(jerald):
      i = 7;
      len = 6000;
      break;
    
    case(rayner):
      i = 8;
      len = 6000;
      break;

    case(maxx):
      i = 9;
      len = 32000;
      break;

    case(sam):
      i = 11;
      len = 8000;
      break;

    case(duckSquad):
      i = 10;
      len = 198000;
      break;

    case(amos):
      i = 12;
      len = 36000;
      break;

    case(eanhuncle):
      i = 13;
      len = 58000;
      break;

    case(hdb):
      i = 14;
      len = 56000 ;
      break;

    case(kurt):
      i = 15;
      len = 20000;
      break;

    case(wangan):
      i = 16;
      len = 11000;
      break;

    case(aomvara):
      i = 17;
      len = 8000;
      break;
   case(panties):
      i = 18;
      len = 11000;
      break;
   /*
    * case(junray):
    *   i = 19;
    *   len = (insert length of mp3 file in miliseconds here);
    *   break;
    */
  
  }
  mp3.begin();
  mp3.volume(30);
  mp3.eq(POP);
  mp3.play_track(i);
  delay(len);
  

}