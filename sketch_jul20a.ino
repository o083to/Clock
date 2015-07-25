#include "enums.h"

#define H4_LED_PIN 13
#define H3_LED_PIN 12
#define H2_LED_PIN 8
#define DATA_PIN 7
#define LATCH_PIN 4
#define CLOCK_PIN 2
#define BUZZER_PIN 3
#define LEFT_BUTTON_PIN 6
#define RIGHT_BUTTON_PIN 5

#define SHORT_CLICK_MIN_DURATION 20
#define LONG_CLICK_MIN_DURATION 500
// RGB LED
#define RED_PIN 9
#define BLUE_PIN 10
#define GREEN_PIN 11

#define DELAY 1000
#define COLORS_COUNT 5
#define SOUND_DURATION 500
#define ALARM_START_HOUR 0
#define ALARM_START_MINUTE 0

#define HOUR_LEDS_COUNT 5
#define MINUTE_LEDS_COUNT 6
#define SINGLE_LEDS_COUNT 3

const byte SINGLE_LEDS[] = { H2_LED_PIN, H3_LED_PIN, H4_LED_PIN };

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

// Colors for RGB LED
byte r[] = { 50, 25, 2, 30, 30 };
byte g[] = { 2, 25, 20, 20, 1 };
byte b[] = { 2, 0, 30, 0, 20 };
byte color = 0;

// Assign current time to these variables
byte hour = 12;
byte minute = 0;
byte second = 0;

//Buttons variables
boolean lButtonWasDown = false;
boolean rButtonWasDown = false;
unsigned long lButtonDownTime = 0;
unsigned long rButtonDownTime = 0;

// Alarm clock
boolean isAlarmNow = false;
boolean isAlarmSet = false;
byte alarmHour = ALARM_START_HOUR;
byte alarmMinute = ALARM_START_MINUTE;

TimeLEDsMode timeLEDsMode = SHOW_TIME_MODE;

void setup() {
  pinMode(H4_LED_PIN, OUTPUT);
  pinMode(H3_LED_PIN, OUTPUT);
  pinMode(H2_LED_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  updateTimeLEDs();
}

void loop() {
  checkButtons();
  
  currentMillis = millis();
  if ((currentMillis - previousMillis) > DELAY) {
    
    changeRGBLEDColor();
    updateTime();
      
    if (isAlarmNow) {
      alarm();
    }
        
    previousMillis = currentMillis;
  }
}

void checkButtons() {
  checkLeftButton();
  checkRightButton();  
  updateTimeLEDs();
}

void checkLeftButton() {
  switch(checkButton(LEFT_BUTTON_PIN, &lButtonWasDown, &lButtonDownTime)) {
    case SHORT_CLICK:
      if (timeLEDsMode == SHOW_TIME_MODE) {
        hour = hour == 23 ? 0 : hour + 1;
        checkAlarmClock();
      } else {
        isAlarmSet = true;
        alarmHour = alarmHour == 23 ? 0 : alarmHour + 1;
      }
      break;
     case LONG_CLICK:
      if (timeLEDsMode == SHOW_TIME_MODE) {
        timeLEDsMode = SET_ALARM_MODE;
      } else {
        timeLEDsMode = SHOW_TIME_MODE;
      }      
      break;
  }  
}

void checkRightButton() {
  switch(checkButton(RIGHT_BUTTON_PIN, &rButtonWasDown, &rButtonDownTime)) {
    case SHORT_CLICK:
      if (timeLEDsMode == SHOW_TIME_MODE) {
        minute = minute == 59 ? 0 : minute + 1;
        second = 0;
        checkAlarmClock();
      } else {
        isAlarmSet = true;
        alarmMinute = alarmMinute == 59 ? 0 : alarmMinute + 1;
      }
      break;
    case LONG_CLICK:
      if (timeLEDsMode == SHOW_TIME_MODE) {
        stopAlarm();
      } else {
        removeAlarm();
      }
      break;
  }
}

ButtonState checkButton(byte buttonPin, boolean *buttonWasDown, unsigned long *buttonDownTime) {
  boolean buttonIsUp = digitalRead(buttonPin);

  if (!*buttonWasDown && !buttonIsUp) {
    *buttonWasDown = true;
    *buttonDownTime = millis();
  } else if (*buttonWasDown && buttonIsUp) {
    *buttonWasDown = false;
    unsigned long clickDuration = millis() - *buttonDownTime;
    if (clickDuration >= LONG_CLICK_MIN_DURATION) {
      return LONG_CLICK;
    } else if (clickDuration >= SHORT_CLICK_MIN_DURATION) {
      return SHORT_CLICK;
    }
  }
  
  return UNDEFINED;
}

void updateTime() {
  second++;
  if (second == 60) {
    second = 0;
    minute++;
    if (minute == 60) {
      minute = 0;
      hour++;
      if (hour == 24) {
        hour = 0;
      }
    }
    if (timeLEDsMode == SHOW_TIME_MODE) {
      updateTimeLEDs();
    }
    checkAlarmClock();
  }
}

void updateTimeLEDs() {
  if (timeLEDsMode == SHOW_TIME_MODE) {
    showTime(hour, minute);
  } else {
    if (isAlarmSet) {
      showTime(alarmHour, alarmMinute);
    } else {
      showTime(31, 63);
    }
  }
}

void showTime(byte hour, byte minute) {
  byte value = 0;
  for (byte i = 0, m = minute; i < MINUTE_LEDS_COUNT; i++, m /= 2) {
    value |= (m % 2) << i;
  }
  byte h = hour;
  for (byte i = 0; i < 2; i++, h /= 2) {
    value |= (h % 2) << (MINUTE_LEDS_COUNT + i);
  }
  for (byte i = 0; i < SINGLE_LEDS_COUNT; i++, h /= 2) {
    digitalWrite(SINGLE_LEDS[i], h % 2); 
  }
  writeToShiftRegister(value);
}

void writeToShiftRegister(byte value) {
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, value);
  digitalWrite(LATCH_PIN, HIGH);
}

void changeRGBLEDColor() {
  if (timeLEDsMode == SHOW_TIME_MODE) {
    analogWrite(RED_PIN, r[color]);
    analogWrite(GREEN_PIN, g[color]);
    analogWrite(BLUE_PIN, b[color]);    
    color = (color == COLORS_COUNT - 1) ? 0 : color + 1;
  } else {
    analogWrite(RED_PIN, 50);
    analogWrite(GREEN_PIN, 50);
    analogWrite(BLUE_PIN, 50);
  }
}

void checkAlarmClock() {
  if (isAlarmSet) {
    if (hour == alarmHour && minute == alarmMinute) {
      isAlarmNow = true;
    }  
  }
}

void stopAlarm() {
  isAlarmNow = false;
}

void removeAlarm() {
  isAlarmSet = false;
  isAlarmNow = false;
}

void alarm() {
  tone(BUZZER_PIN, 3500, SOUND_DURATION);
}

