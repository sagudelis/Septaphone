#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// FOR SETTING PHONE NUMBERS
static const uint8_t NUM_NUMBERS = 8;                         //How many phone numbers there are
const byte inputs[7] = {A0, A1, A2, A3, A4, A5, A6};          //These pins are for the wires, if they are all connected correctly they'll all be high
// int number1[7] = {6, 6, 6, 6, 6, 6, 6};
int number0[7] = {1, 2, 3, 4, 5, 6, 7};                       // this is a dummy number
int number1[7] = {7, 3, 3, 2, 2, 2, 2};                       // Define each number separately 
int number2[7] = {2, 7, 7, 7, 7, 7, 6};
int number3[7] = {6, 6, 2, 2, 1, 1, 1};
int number4[7] = {1, 4, 4, 4, 4, 4, 4};
int number5[7] = {6, 6, 6, 6, 5, 2, 2};
int number6[7] = {6, 7, 4, 4, 1, 3, 3};
int number7[7] = {1, 2, 3, 4, 5, 6, 7};                       // this is a dummy number

// int number0[7] = {1, 2, 3, 4, 5, 6, 7};                       //Define each number separately 
// int number1[7] = {1, 1, 1, 1, 1, 1, 1};
// int number2[7] = {2, 2, 2, 2, 2, 2, 2};
// int number3[7] = {3, 3, 3, 3, 3, 3, 3};
// int number4[7] = {4, 4, 4, 4, 4, 4, 4};
// int number5[7] = {5, 5, 5, 5, 5, 5, 5};
// int number6[7] = {6, 6, 6, 6, 6, 6, 6};
// // int number1[7] = {7, 7, 7, 7, 7, 7, 7};
// int number7[7] = {1, 2, 3, 4, 5, 6, 7};

int *allNumbers[NUM_NUMBERS] = {number0, number1, number2, number3, number4, number5, number6, number7};   //add all numbers to the array
int currentNumber = 0;                                        //counter for number
static const uint8_t NUMCORRECT = 7;

// audio numbers
static const uint8_t PHONE_RING = 1;        // audio file number corresponding to phone ringing audio
// static const uint8_t WRONG_NUMBER = 2;      // audio file number corresponding to wrong number message
static const uint8_t FIRST_AUDIO = 4;       // what the actual first call audio is

// other input/output macros 
static const uint8_t PIN_BUT_IN = 52;   // push button pin
static const uint8_t LIGHT_OUT = A15;    // output pin for indicator light
static const uint8_t RESET_TRIGGER = A14;  //output pin for triggering board reset

// for light control
bool lightOn = false;     // bool to track light status (true is high/on, false is low/off)
unsigned long lastToggle = 0;
unsigned long slowBlinkTime = 500;
unsigned long fastBlinkTime = 250;
unsigned long firstPress = 0;   // save the time the first button was pressed
unsigned long waitTime = 10 * 1000;   // how long to wait for second reset press (currently 10 seconds)

// DFPlayer Mini setup 
static const uint8_t PIN_MP3_TX = 50; // D7
static const uint8_t PIN_MP3_RX = 51; // D6
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);
DFRobotDFPlayerMini player;     //Create the player object

// track time button was last pressed for reset timing
unsigned long lastPressTime = 0;
unsigned long resetPeriod = 15 * 60 * 1000;   // how long to wait to reset, currently 15 minutes
bool waitingForReset = false;

void setup() 
{  
  Serial.begin(19200);
  softwareSerial.begin(9600);

  if (player.begin(softwareSerial)) 
  {
    Serial.println("OK");
    player.volume(30);                // Set volume to maximum (0 to 30).
    // Serial.print("Playing #1 \t");
    // player.play(1);
    // delay(1000);
  } 
  else 
  {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }
  
  Serial.println("Setting pins to outputs");
  for (int i = 1; i <= 49; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW); 
    String printStatus = String(i) + "set to output";
    Serial.println(printStatus);
  }
 
  // input pins for phone dialing
  for (int i = 0; i < 7; i++) {
    pinMode(inputs[i], INPUT);
    Serial.println(inputs[i]);
  }
  
  // i/o pins for button, light, and board reset
  pinMode(PIN_BUT_IN, INPUT);
  pinMode(LIGHT_OUT, OUTPUT);
  digitalWrite(LIGHT_OUT, LOW);  
  // digitalWrite(RESET_TRIGGER, HIGH);
  // pinMode(RESET_TRIGGER, OUTPUT);

  //set the first number
  // int nextNumber[7] = allNumbers[currentNumber];
  setNumber(allNumbers[currentNumber]);
  // currentNumber++;
  // player.play(3);/
  // delay(10000);

  // blink the light ten times fast 
  for (int i = 0; i<10; i++){
    Serial.println("High");
    digitalWrite(LIGHT_OUT, HIGH);
    delay(slowBlinkTime);
    Serial.println("LOW");
    digitalWrite(LIGHT_OUT, LOW);
    delay(slowBlinkTime);
  }  

  // digitalWrite(48, HIGH);
}

//function to check if a number is dialed correctly, returns the number of high pins
int checkNumber(){
    int theVal = 0;           // digital read of a pin
    int numHigh = 0;          // counter for how many pins are high

    for (int pinNum = 0; pinNum < 7; pinNum++){
      int readPin = inputs[pinNum];
      // theVal = digitalRead(readPin);
      Serial.print(readPin);
      Serial.print("A");
      Serial.print(pinNum);
      Serial.print(" is ");
      int anVal = analogRead(readPin);
      theVal = digitalRead(readPin);
      Serial.println(anVal);
      if (anVal >= 900){
        Serial.println("HIGH");
        numHigh++;}     // if the value is high, 
    }
    Serial.println("----------------------");

    return numHigh;
}

//function to raise pins to "set" a number
void setNumber(int *phoneNumber){
  // lower all pins
  Serial.print("Setting new number: ");
  for (int i = 0; i < 7; i++){
    int digit = phoneNumber[i];
    Serial.print(digit);
  }
  Serial.println("");
  for (int x = 1; x <= 49; x++){
    digitalWrite(x, LOW);
  }
  // raise the specified pins
  for (int i = 0; i < 7; i++){
    int digit = phoneNumber[i];
    int highPin = (i + 1) + (7*(digit-1));
    if (highPin == 1){
      digitalWrite(0, HIGH);
      Serial.print("0");
      Serial.println(" set to HIGH");
    }
		// int highPin = digit + (7*(i));
    digitalWrite(highPin, HIGH);
    Serial.print(highPin);
    Serial.println(" set to HIGH");
  } 
  Serial.println("--------------");
}

// check if all pins are disconnected
bool allDisconnected() {
  // throw all output pins high
  for (int x = 1; x <= 49; x++){
    digitalWrite(x, HIGH);
  }

  delay(5000);  // delay a bit to let pins catch up

  // check and see if any pins are connected
  if (checkNumber() == 0) {   // if none connected, return true
    setNumber(allNumbers[currentNumber]);
    return true;
  }
  else {      // else, set pins back to current number and return false
    setNumber(allNumbers[currentNumber]);
    return false;
  }
}

// check timing for automatic reset
void checkResetTime(){
  unsigned long currentTime = millis();

  if (currentTime - lastPressTime > resetPeriod){    //if 15 minutes have elapsed without a button press, 
    // digitalWrite(RESET_TRIGGER, HIGH);
    resetPeriod = 15 * 60 * 1000;   // change reset timing back to 15 minutes
    currentNumber = 0; 
    lastPressTime = currentTime;
    waitingForReset = false;
  }
}

// control light
void controlLight() {
  // if (waitingForReset) {    
  unsigned long currentTime = millis();
  // Serial.println("Blink");
  if (currentTime - firstPress >= waitTime) {     // if the wait time is over
    Serial.println("done waiting");
    waitingForReset = false;    // no longer waiting for confirmation
    digitalWrite(LIGHT_OUT, LOW);     // turn the light off
    lightOn = false;
  }
  else if (currentTime - lastToggle >= fastBlinkTime){   // if it's time to toggle
    Serial.println("Blink");
    if (lightOn) {
      digitalWrite(LIGHT_OUT, LOW);     // turn the light off
      lightOn = false;
      lastToggle = millis();
    }
    else {
      digitalWrite(LIGHT_OUT, HIGH);    // turn the light on
      lightOn = true;
      lastToggle = millis();
    }
  }
  // }
  // eventually add check for if player is busy
}


void loop() 
{

  // checkNumber();
  // delay(5000);
  int buttonVal = digitalRead(PIN_BUT_IN);
  
  if (buttonVal == HIGH){
    Serial.println("Button pressed!");
    digitalWrite(LIGHT_OUT, HIGH);
    if (!waitingForReset) {
      // play ringer noise
      Serial.println("Ringing");
      player.play(1);
      delay(500);
    }

    // check if it's playing the current number 
    Serial.print("Checking number ");
    Serial.println(currentNumber);
    if (checkNumber() >= NUMCORRECT) {
      Serial.println("Redial...");
      // play the call for the dialed number
      Serial.print("Playing ");
      int audioNumber = currentNumber + FIRST_AUDIO;    
      Serial.println(audioNumber);
      player.play(audioNumber);    
      delay(12000);  
      // setNumber(allNumbers[currentNumber]);
      // currentNumber++;
    }
    else {
      Serial.println("Checking for next number");
      int redialNumber = currentNumber + 1;
      Serial.print("Checking ");
      Serial.println(redialNumber);
      // currentNumber++;                        // increment number counter
      // int redialNumber = currentNumber - 1;
      setNumber(allNumbers[redialNumber]);      // set number to next number
      delay(5000);                            // delay a bit to wait for pins to catch up
      // Serial.print("Checking ");
      // Serial.println(redialNumber);
      int numHigh = checkNumber();

      if (numHigh >= NUMCORRECT) {   
        Serial.println("Calling new number");
        // play the dialed number
        Serial.print("Playing ");
        int audioNumber = currentNumber + 1 + FIRST_AUDIO;    
        Serial.println(audioNumber);
        player.play(audioNumber);    
        delay(12000);  
        currentNumber++;
        if (currentNumber >= 7)   // Emmet - can you check this and make sure it should be 7? it might need to be 6 but i'm not sure
        { 
          Serial.println("Last number, updating reset time");
          resetPeriod = 5 * 60 * 1000;  //updates this to be 5 minutes
        }
        // setNumber(allNumbers[currentNumber])
      }
      else if (numHigh == 0) {         // if there are no pins plugged in, check reset
        Serial.println("Checking for reset");
        // see if all pins are truly disconnected
        if (allDisconnected()) {
          player.stop();            // stop the ringing 
          if (waitingForReset) {    // if the button has already been pressed once for a reset, reset the board
            Serial.print("Resetting Board");
            digitalWrite(RESET_TRIGGER, HIGH);
            currentNumber = 0; 
            lastPressTime = millis();
            waitingForReset = false;
          }
          else {    // start reset confirmation timing
            Serial.print("Waiting for reset confirmation...");
            waitingForReset = true;            // now waiting for confirmation
            firstPress = millis();
            lastToggle = millis();
            digitalWrite(LIGHT_OUT, HIGH);     // turn the light on
            lightOn = true;  
          }
        }
        else {
          Serial.println("No reset");
          // player.stop();
          if (numHigh < NUMCORRECT ) {
            // play wrong number audio
            Serial.println("Wrong Numba!");    
            Serial.print("Playing ");
            int audioNumber = random(2, 5);
            // int audioNumber = WRONG_NUMBER;   
            Serial.println(audioNumber);
            player.play(audioNumber);    
            delay(12000); 
          }
        }
      }
      else if (numHigh < NUMCORRECT) {
        // play wrong number audio
        Serial.println("Wrong Numba!");    
        Serial.print("Playing ");
        int audioNumber = random(2, 5);
        // int audioNumber = WRONG_NUMBER;   
        Serial.println(audioNumber);
        player.play(audioNumber);    
        delay(12000); 
      }
      setNumber(allNumbers[currentNumber]);   // set number to next number
    }
    
    // update the button press timing for auto reset timing
    lastPressTime = millis();
    digitalWrite(LIGHT_OUT, LOW); //turn the light off to show it's ok to press again
  }
  else {        // run these always (button not pressed)
    checkResetTime();
    if (waitingForReset) { 
      controlLight();
    }
  }
}









