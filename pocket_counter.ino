#include <SevenSegmentDisplay.h>
#include <FlashStorage.h>

#define MODE_COUNTER 0
#define MODE_STOPWATCH 1
#define SW_INIT 0
#define SW_RUNNING 1
#define SW_PAUSED 2

#define GREEN_BTN 1
#define RED_BTN 0

#define BLUE_LED 9
#define LED_ON_LOW 240
#define LED_ON_MED 200
#define LED_ON_HIGH 100

#define BUILTIN_YELLOW LED_BUILTIN
#define BUILTIN_BLUE PIN_LED3
#define BUILTIN_LOW HIGH
#define BUILTIN_HIGH LOW

#define SEG_DISPLAY_GROUND_PIN 4
#define dashPin 6

FlashStorage(stored_value, uint);

SevenSegmentDisplay lcd(3, 2, 10, 8, 7, 5, dashPin, PIN_LED2, false);
uint value = 0;
uint lastClick = 0;
uint bothHeldStart = 0;
uint MODE = MODE_COUNTER;
uint SW_MODE = SW_INIT;
uint SW_START_TIME = 0;
uint SW_PAUSED_TIME = 0;
bool debug = false;

void setup() {

  Serial.begin(9600);

  pinMode(BUILTIN_YELLOW, OUTPUT);
  pinMode(BUILTIN_BLUE, OUTPUT);

  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_BTN, INPUT_PULLDOWN);
  pinMode(GREEN_BTN, INPUT_PULLDOWN);

  pinMode(SEG_DISPLAY_GROUND_PIN, OUTPUT);
  digitalWrite(SEG_DISPLAY_GROUND_PIN, LOW);


  bool greenHeld = digitalRead(GREEN_BTN);
  bool redHeld = digitalRead(RED_BTN);
  if(greenHeld){
    MODE = MODE_COUNTER;
  } else {
    MODE = MODE_STOPWATCH;
  }
  if(redHeld){
    debug = true;
  }

  if(MODE == MODE_COUNTER) {
    attachInterrupt(digitalPinToInterrupt(GREEN_BTN), addOne, RISING);
    attachInterrupt(digitalPinToInterrupt(RED_BTN), subtractOne, RISING);
    value = stored_value.read();
  } else if(MODE == MODE_STOPWATCH){
    attachInterrupt(digitalPinToInterrupt(GREEN_BTN), swGreen, RISING);
    attachInterrupt(digitalPinToInterrupt(RED_BTN), swRed, RISING);
  }
}

void printStopwatchDebug(){
  Serial.print("Mode: Stopwatch | ");
  Serial.print(SW_MODE);
  
  Serial.print(" | SW_START_TIME: ");
  Serial.print(SW_START_TIME);
  Serial.print(" | SW_PAUSED_TIME: ");
  Serial.println(SW_PAUSED_TIME);
  if(SW_MODE == SW_INIT){
    Serial.print("SW INIT");
  } else if(SW_MODE == SW_RUNNING){
    Serial.print("SW Running");  
  } else if(SW_MODE == SW_PAUSED){
    Serial.print("SW Paused");
  } else {
    Serial.print("SW Mode ERROR: ");
    Serial.print(SW_MODE);
  }
  Serial.print("Running Time");
  Serial.print((millis()-SW_START_TIME)/1000);
  Serial.print("s");
  
}
void printDebug(){
  if(MODE == MODE_STOPWATCH){
    printStopwatchDebug();
  }
}
void loop() {
  //if(debug){ printDebug(); }

  if(MODE == MODE_COUNTER){
    loop_counter();
  } else if(MODE == MODE_STOPWATCH) {
    loop_stopwatch();
  }
}

void blueLEDOFF(){analogWrite(BLUE_LED, 255);}
void blueLEDLOW(){analogWrite(BLUE_LED, LED_ON_LOW);}
void blueLEDMED(){analogWrite(BLUE_LED, LED_ON_MED);}
void blueLEDHIGH(){analogWrite(BLUE_LED, LED_ON_HIGH);}

void addOne() {
  if(millis() - lastClick < 200){
    return;
  }
  analogWrite(9, 240);
  lastClick = millis();
  value++;
  stored_value.write(value);
}
void subtractOne(){
  if(millis() - lastClick < 200){
    return;
  }
  analogWrite(9, 200);
  
  if(value == 0){
    return;
  }
  lastClick = millis();
  value--;
  stored_value.write(value);
}
void clear() {
  lastClick = millis();
  value = 0;
  stored_value.write(0);
}

void showTime(uint secs) {
  uint mins = secs/60;
  secs = secs % 60;
  uint hours = mins/60;
  mins = mins % 60;
  uint days = hours/24;
  hours = hours % 24;
  String timeStr = "";

  if(days > 0){
    timeStr = String(days) + '-' + String(hours) + '-' + String(mins) + '-' + String(secs);
  } else if(hours > 0){
    timeStr = String(hours) + '-' + String(mins) + '-' + String(secs);
  } else if(mins > 0){
    timeStr = String(mins) + '-' + String(secs);
  } else {
    timeStr = secs;
  }

  uint dupes = 0;
  for(int i=1;i<timeStr.length();i++){
    if(timeStr.charAt(i) == timeStr.charAt(i-1)){
      dupes++;
    }
  }

  // total display time target
  uint displayTime = 950 - 55*dupes;
  uint charsCount = timeStr.length();

  //we'll have an extra blank at the end if we have more than one char
  if(charsCount > 1){
    charsCount++;
  }
  uint displayCharTime = displayTime / charsCount;

  Serial.println(String("Time: ") + timeStr + ' ' + charsCount + ' ' + displayTime + '-' + '>' + displayCharTime);
  for(int i=0;i<timeStr.length();i++){

    // if we're about to print the same char a 2nd time add a blink
    if(i > 0){
      if(timeStr.charAt(i) == timeStr.charAt(i-1)){
        lcd.displayCharacter(' ');
        delay(50);
      }
    }
    char chr = timeStr.charAt(i);
    if(chr == '-'){
      lcd.displayCharacter(' ');
      digitalWrite(dashPin, HIGH);
    } else {
      digitalWrite(dashPin, LOW);
      lcd.displayCharacter(chr);
    }
    delay(displayCharTime);
  }
  if(timeStr.length() > 1){
    lcd.displayCharacter(' ');
    delay(displayCharTime);
  }
}

void loopNum(int num, int delayMS){
  String str = String(num);
  for(int i=0;i<str.length();i++){

    // if we're about to print the same char a 2nd time add a blink
    if(i > 0){
      if(str.charAt(i) == str.charAt(i-1)){
        lcd.displayCharacter(' ');
        delay(50);
      }
    }
    lcd.displayCharacter(str.charAt(i));
    delay(delayMS);
  }
  if(str.length() > 1){
    lcd.displayCharacter(' ');
    delay(delayMS);
  }
}

void swGreen(){
  if(millis() - lastClick < 200){
    return;
  }
  lastClick = millis();
  if(SW_MODE == SW_INIT){
    SW_MODE = SW_RUNNING;
    SW_START_TIME = millis();
  } else if(SW_MODE == SW_RUNNING){
    // normally the "lap" button
    // not implemented
  } else if(SW_MODE == SW_PAUSED){
    SW_MODE = SW_RUNNING;
    
    // subtract the original time from current time to "fake" original start time
    SW_START_TIME = millis()-(SW_PAUSED_TIME-SW_START_TIME);
  }
}
void swRed(){
  if(millis() - lastClick < 200){
    return;
  }
  lastClick = millis();
  if(SW_MODE == SW_INIT){
    // init and red hit = do nothing?
  } else if(SW_MODE == SW_RUNNING){
    SW_MODE = SW_PAUSED;
    SW_PAUSED_TIME = millis();
  } else if(SW_MODE == SW_PAUSED){
    SW_MODE = SW_INIT;
  }
}

void loop_stopwatch() {
  uint loopStart = millis();
  if(SW_MODE == SW_INIT){
    blueLEDLOW();
    lcd.displayCharacter('0');
  } else if(SW_MODE == SW_RUNNING){
    blueLEDOFF();
    uint secs = (millis()-SW_START_TIME)/1000;
    showTime(secs);
    //loopNum(secs, 200);

    // Tries to start next loop at next second time
    uint elapsedLoopTime = millis() - loopStart;
    Serial.println(String("loop: ") + millis() + '-' + loopStart + '=' + elapsedLoopTime);
    if(elapsedLoopTime < 1000){
      delay(1000-elapsedLoopTime);
    }
  } else if(SW_MODE == SW_PAUSED){
    blueLEDHIGH();
    uint secs = (SW_PAUSED_TIME-SW_START_TIME)/1000;
    showTime(secs);
    //loopNum(secs, 300);
  }
}
void loop_counter() {
  //Serial.println(value);
  bool greenBtn = digitalRead(1);
  bool redBtn = digitalRead(0);
  if(greenBtn && redBtn){
    if(bothHeldStart == 0) {
      bothHeldStart = millis();
    }
    if(millis() - bothHeldStart > 1500){
      clear();
      // this will keep the Flash from getting written over and over until they let go..;
      bothHeldStart = millis();
    }
  } else {
    bothHeldStart = 0;
  }
  //digitalWrite(PIN_LED2, oneOn ? BUILTIN_HIGH : BUILTIN_LOW);
  loopNum(value, 200);
  delay(100);
}
