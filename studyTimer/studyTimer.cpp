#include <LiquidCrystal.h>
#include "Timer.h"


#define TOTAL_TASK 3
#define PERIOD_GCD 50
#define PERIOD_OnOff 100
#define PERIOD_Pause 100
#define PERIOD_StopWatch 1000

bool SYSTEM = 0;

//LCD pin setup
const int RS = 2;
const int E = 3;
const int D4 = 4;
const int D5 = 5;
const int D6 = 6;
const int D7 = 7;
//important to control LCD screen On/Off
const int A = 10;
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

//ON/OFF Button Setup
const int buttonPinOnOff = 8;
const int ledPinOnOff = 9;
bool buttonStateOnOff = 0;

//stopwatch Play/Pause Button Setup
const int buttonPinPause = 11;
const int ledPinPause = 12;
bool buttonStatePause = 0;
bool play = 0;

//Stopwatch time tracker
unsigned short tSec = 0;
unsigned short tMin = 0;
unsigned short tHr = 0;

//creating a struct named "task" 
typedef struct task{
  unsigned short period;
  unsigned short timeElapsed;

  void(*tick) (void);
};

//an array of type task for each task
static struct task gTaskSet[TOTAL_TASK];

//initialize all task in the array gTaskSet
void initializeTask(void){
  //on off task
  gTaskSet[0].period = PERIOD_OnOff;
  gTaskSet[0].timeElapsed = 0;
  gTaskSet[0].tick = tickOnOff;

  //play pause task
  gTaskSet[1].period = PERIOD_Pause;
  gTaskSet[1].timeElapsed = 0;
  gTaskSet[1].tick = tickPause;

  //run stopwatch
  gTaskSet[2].period = PERIOD_StopWatch;
  gTaskSet[2].timeElapsed = 0;
  gTaskSet[2].tick = tickStopWatch;
}

//Task scheduler to figure out which task is ready to be called
void scheduleTask(){
  for(int i = 0; i < TOTAL_TASK; i++){
    gTaskSet[i].timeElapsed += PERIOD_GCD;
    if(gTaskSet[i].timeElapsed >= gTaskSet[i].period){
      gTaskSet[i].tick();
      gTaskSet[i].timeElapsed = 0;
    }
  }
}

//creating the statemachine for specific task
enum STATES_ONOFF {INIT, REST, ON, ON_IDLE, OFF} gState = INIT; //on off button
enum STATES_PAUSE {START, INITIAL, PLAY, PLAY_IDLE, PAUSE} pState = START; //play pause button
enum STATES_STOPWATCH {WATCH, IDLE, rSec, rMin, rHr} tState = WATCH;

void tickOnOff (void){
  switch(gState){
    case INIT:
      if(1){
        gState = REST;
      }
      break;

    case REST:
      if(buttonStateOnOff == HIGH){
        gState = ON;
      }
      else{
        gState = REST;
      }
      break;

    case ON:
      if(buttonStateOnOff == LOW){
        gState = ON_IDLE;
      }
      else{
        gState = ON;
      }
      break;

    case ON_IDLE:
      if(buttonStateOnOff == HIGH){
        gState = OFF;
      }
      else{
        gState = ON_IDLE;
      }
      break;

    case OFF:
      if(buttonStateOnOff == LOW){
        gState = REST;
      }
      else{
        gState = OFF;
      }
      break;
    default:
      gState = REST;
      break;
  }
  switch(gState){
    case INIT:
      break;

    case REST:
      digitalWrite(ledPinOnOff, LOW);
      //turn off LCD
      SYSTEM = 0;
      digitalWrite(A, LOW);
      lcd.clear();
      play = 0;
      break;

    case ON:
      digitalWrite(ledPinOnOff, HIGH);
      //turn on LCD
      SYSTEM = 1;
      digitalWrite(A, HIGH);
      play = 1;
      break;
    
    case ON_IDLE:
      break;
    
    case OFF:
      digitalWrite(ledPinOnOff, LOW);
      //turn off LCD
      SYSTEM = 0;
      digitalWrite(A, LOW);
      lcd.clear();
      play = 0;
      break;
  }
}

void tickPause (void){
  switch(pState){
    case START:
      if(1){
        pState = INITIAL;
      }
      break;
    case INITIAL:
      if(buttonStatePause == HIGH){
        pState = PLAY;
      }
      else if(buttonStatePause == LOW){
        pState = INITIAL;
      }
      break;
    case PLAY:
      if(buttonStatePause == LOW){
        pState = PLAY_IDLE;
      }
      else if(buttonStatePause == HIGH){
        pState = PLAY;
      }
      break;
    case PLAY_IDLE:
      if(buttonStatePause == HIGH){
        pState = PAUSE;
      }
      else if(buttonStatePause == LOW){
        pState = PLAY_IDLE;
      }
      break;
    case PAUSE:
      if(buttonStatePause == LOW){
        pState = INITIAL;
      }
      else if(buttonStatePause == HIGH){
        pState = PAUSE;
      }
      break;
    default:
      pState = INITIAL;
      break;
  }
  switch(pState){
    case START:
      break;
    case INITIAL:
      digitalWrite(ledPinPause, LOW);
      play = 0;
      break;
    case PLAY:
      digitalWrite(ledPinPause, HIGH);
      play = 1;
      break;
    case PLAY_IDLE:
      break;
    case PAUSE:
      digitalWrite(ledPinPause, LOW);
      play = 0;
      break;
  }
}


void tickStopWatch (void){
  switch(tState){
    case WATCH:
      if(1){
        tState = IDLE;
      }
      break;

    case IDLE:
      if(play){
        tState = rSec;
      }
      else if(!play){
        tState = IDLE;
      }
      break;

    case rSec:
      if(tSec < 59 && play){
        tState = rSec;
      }
      else if(!(tSec < 59) && play){
        tState = rMin;
      }
      else if(!play){
        tState = IDLE;
      }
      break;

    case rMin:
      if(tMin < 59 && play){
        tState = rSec;
      }
      else if(!(tMin < 59) && play){
        tState = rHr;
      }
      else if(!play){
        tState = IDLE;
      }
      break;
    case rHr:
      if(play){
        tState = rSec;
      }
      else if(!play){
        tState = IDLE;
      }
      break;

    default:
      tState = IDLE;
      break;
  }
  switch(tState){
    case WATCH:
      break;

    case rSec:
      tSec++;
      break;

    case rMin:
      tMin++;
      tSec = 0;
      break;

    case rHr:
      tHr++;
      tMin = 0;
      break;

    case IDLE:
      break;
  }
}

void setup() {
  //lcd
  lcd.begin(16, 2);
  //On/Off Button & LED
  pinMode(ledPinOnOff, OUTPUT);
  pinMode(buttonPinOnOff, INPUT);
  //determine whether LCD is ON/OFF
  pinMode(A, OUTPUT);

  //Play/Pause Button & LED
  pinMode(ledPinPause, OUTPUT);
  pinMode(buttonPinPause, INPUT);

  TimerSet(PERIOD_GCD);
  TimerOn();
  initializeTask();
}

void loop() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(tHr);
  lcd.print(":");
  lcd.print(tMin);
  lcd.print(":");
  lcd.print(tSec);
  
  buttonStateOnOff = digitalRead(buttonPinOnOff);
  if(SYSTEM == HIGH){
    buttonStatePause = digitalRead(buttonPinPause);
  }
  else{
    digitalWrite(ledPinPause, LOW);
    pState = START;
    play = 0;
  }
  scheduleTask();
  while(!TimerFlag){}
  TimerFlag = 0;
}
