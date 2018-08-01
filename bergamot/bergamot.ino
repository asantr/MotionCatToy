#include <DuoBLE.h>

SYSTEM_MODE(MANUAL);

const char * const deviceName = "BERG";

Servo myservo;  // create servo object to control a servo
                
int calibrationTime = 30;
long unsigned int lowIn;
long unsigned int none = 5000;


int ledPin = D7;                 // choose the pin for the LED
int inputPin = D1;               // choose the PIR sensor pin
bool available;                  // status of conference room
int motionCounter = 0;           // variable to count motion events
String status;  



int pos = 0;    // variable to store the servo position
int toyPosition = 0; //variable to store where the toy is
int maxVal = 50;
int delayTime = 0;
String timerUsed = "weird"; //timerUsed monitors the mode last used
String command = "on";

bool off = false;

byte catStat[] = {0};

// slow mode for cat toy called by slowTimer to tick servo every 5 seconds
void slowMode(){
    timerUsed = "slow";
    if(toyPosition < 180){
            toyPosition += value();
            myservo.write(toyPosition);      // move servo to random position
             
            Serial.println(toyPosition);
        }else{
            toyPosition = 0;
             myservo.write(toyPosition);       // move servo to 0°
             Serial.println(toyPosition);
        }
}
// fast mode called by fast timer and will tick every 2 seconds
void fastMode(){
    timerUsed = "fast";
    if(toyPosition < 180){
            toyPosition += value();
            myservo.write(toyPosition);      // move servo to random position
             
            Serial.println(toyPosition);
        }else{
            toyPosition = 0;
            myservo.write(toyPosition);       // move servo to 0°
            Serial.println(toyPosition);
        }
}
//first of two timers for random mode called by weirdOne to tick every 1.5 seconds
void weirdModeOne(){
    timerUsed = "weird";
    toyPosition = 180-value();
    myservo.write(toyPosition);      // move servo to random position
    
    Serial.println(toyPosition);
    
}
//second of two timers for random mode called by weirdTwo to tick every 6 seconds
void weirdModeTwo(){
    toyPosition = 0 + value();
    
    myservo.write(toyPosition);      // move servo to random position
    Serial.println(toyPosition);
}
        
//timers for the different modes
Timer slowTimer(5000, slowMode);
Timer fastTimer(2000, fastMode);
Timer weirdOne(1500, weirdModeOne);
Timer weirdTwo(6000, weirdModeTwo);

void determineMotion() {    // determines if there's motion
    if(off == false){ //unless the system is off we check for motion
       if(digitalRead(inputPin) == HIGH) { // if motion was detected
           myservo.write(15);
           //statements to turn on the timer last used
          if(timerUsed == "slow"){
            slowTimer.start();
          }
          else if (timerUsed == "fast"){
             fastTimer.start();
          }else if(timerUsed == "weird"){
            weirdOne.start();
            weirdTwo.start();
          } 
            catStat[0] = 1; //keep track of if berg is in room of not
           Serial.println("BERG IS IN THE ROOM");

        } else if(digitalRead(inputPin) == LOW) { //if nothing moves
          //no motion turns everything off
           slowTimer.stop();
           fastTimer.stop();
            weirdOne.stop();
            weirdTwo.stop();
            catStat[0] = 0; //set to say berg isn't in the room 
           Serial.println("NOT HERE");
        
        }
    }
   
}
Timer timer(15000, determineMotion); // timer to check every 15s

//ble characteristics
BLEService simpleCustomService("cd914837-b0d8-4cf7-9b11-f3f985a03c2d");
BLECharacteristic onoff("FF01", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE);
BLECharacteristic slowC("FF02", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE);
BLECharacteristic fastC("FF03", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE);
BLECharacteristic weirdC("FF04", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE);
BLECharacteristic leftC("FF05", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE);
BLECharacteristic rightC("FF06", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE);
BLECharacteristic circleC("FF07", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE);
BLECharacteristic catInRoom("FF08", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE);

//callback to see if there is motion in room or not
void catCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  if(reason == POSTREAD){
    catInRoom.setValue(catStat,1);
  }
}
// this acts like a switch for on and off write 1 for on and 0 for off
void onOffCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
     if(reason == POSTWRITE) {
       int x=0;
       byte value[1];
       int bytes = onoff.getValue(value, 1);
       for(int i=0;i<bytes;i++){
         if(value[i]>0){
          x=1;
         } 
       }
       if(x==1){
        off = false;
        Serial.println(toyPosition);
        Serial.println("ON");
       }
       if(x==0){
        slowTimer.stop();
        fastTimer.stop();
        weirdOne.stop();
        weirdTwo.stop();

        off = true;
        Serial.println("off");
       }
     }
 }

 //callback for slowC if written 1 turns on slowTimer
 void slowCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
     if(reason == POSTWRITE) {
       int x=0;
       byte value[1];
       int bytes = slowC.getValue(value, 1);
       for(int i=0;i<bytes;i++){
         if(value[i]>0){
          x=1;
         } 
       }
       if(x==1){
        slowTimer.start();
        fastTimer.stop();
        weirdOne.stop();
        weirdTwo.stop();
        off = false;
        Serial.println("SLOW");
        Serial.println(toyPosition);
       }
     }
 }

 //callback for fastC write 1 to turn on fastTimer
 void fastCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
     if(reason == POSTWRITE) {
       int x=0;
       byte value[1];
       int bytes = fastC.getValue(value, 1);
       for(int i=0;i<bytes;i++){
         if(value[i]>0){
          x=1;
         } 
       }
       if(x==1){
        slowTimer.stop();
        fastTimer.start();
        weirdOne.stop();
        weirdTwo.stop();
       Serial.println("FAST");
        Serial.println(toyPosition);
       }
     }
 }
 //callback for weird. write 1 to turn on random mode
 void weirdCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
     if(reason == POSTWRITE) {
       int x=0;
       byte value[1];
       int bytes = weirdC.getValue(value, 1);
       for(int i=0;i<bytes;i++){
         if(value[i]>0){
          x=1;
         } 
       }
       if(x==1){
        weirdOne.start();
        weirdTwo.start();
        slowTimer.stop();
        Serial.println("RANDOM");
        Serial.println(toyPosition);
       }
     }
 }

 //callback for left ticks 10 in neg direction if written 1
 void leftCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
     if(reason == POSTWRITE) {
       int x=0;
       byte value[1];
       int bytes = leftC.getValue(value, 1);
       for(int i=0;i<bytes;i++){
         if(value[i]>0){
          x=1;
         } 
       }
       if(x==1){
        if(toyPosition > 1){
            toyPosition -= 10;
            myservo.write(toyPosition);
       
        }else{
          toyPosition = 0;
          myservo.write(toyPosition);
        }
        off = false;
        Serial.println(toyPosition); 
       }
     }
 }
 //callbacm for rightC. write 1 to tick 10 to pos direction
 void rightCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
     if(reason == POSTWRITE) {
       int x=0;
       byte value[1];
       int bytes = rightC.getValue(value, 1);
       for(int i=0;i<bytes;i++){
         if(value[i]>0){
          x=1;
         } 
       }
       if(x==1){
        if(toyPosition < 180){
            toyPosition += 10;
            myservo.write(toyPosition);
             
        }else{
          toyPosition = 180;
          myservo.write(toyPosition);
        }
        off = false;
        Serial.println(toyPosition);  
       }
     }
 }

 //callback for circle motion. write 1 to tick in half circle
 void circleCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
     if(reason == POSTWRITE) {
       int x=0;
       byte value[1];
       int bytes = circleC.getValue(value, 1);
       for(int i=0;i<bytes;i++){
         if(value[i]>0){
          x=1;
         } 
       }
       if(x==1){
        while (toyPosition < 180){
            toyPosition += 3;
            delay(100);
            digitalWrite(D7, HIGH); // flash the LED (as an indicator)
            myservo.write(toyPosition);
            digitalWrite(D7, LOW);  // turn off LED 
        } 
        off = false;
         Serial.println(toyPosition); 
       }
     }
 }


//ble connect and disconnect

void BLE_connected() {
   Serial.println("Central Connected");
 }
 void BLE_disconnected() {
   Serial.println("Central Disconnected");
 }

//SETUP 
void setup()
{
    pinMode(ledPin, OUTPUT);       // set LED as output
    pinMode(inputPin, INPUT);      // set sensor as input

    timer.start(); // start the determineMotion time

    myservo.attach(D0);   // attach the servo on the D0 pin to the servo object
    myservo.write(25);    // test the servo by moving it to 25°
    pinMode(D7, OUTPUT);  // set D7 as an output so we can flash the onboard LED

//set all characteristics to 1 
    byte setbyte[] = {0};
    onoff.setValue(setbyte,1);
    slowC.setValue(setbyte,1);
    fastC.setValue(setbyte,1);
    weirdC.setValue(setbyte,1);
    leftC.setValue(setbyte,1);
    rightC.setValue(setbyte,1);
    circleC.setValue(setbyte,1);
    catInRoom.setValue(catStat,1);

    //set all callbacks
    onoff.setCallback(onOffCallback);
    slowC.setCallback(slowCallback);
    fastC.setCallback(fastCallback);
    weirdC.setCallback(weirdCallback);
    leftC.setCallback(leftCallback);
    rightC.setCallback(rightCallback);
    circleC.setCallback(circleCallback);
    catInRoom.setCallback(catCallback);
    
    //add characteristics
    simpleCustomService.addCharacteristic(onoff);
    simpleCustomService.addCharacteristic(slowC);
    simpleCustomService.addCharacteristic(fastC);
    simpleCustomService.addCharacteristic(weirdC);
    simpleCustomService.addCharacteristic(leftC);
    simpleCustomService.addCharacteristic(rightC);
    simpleCustomService.addCharacteristic(circleC);
    simpleCustomService.addCharacteristic(catInRoom);

    DuoBLE.addService(simpleCustomService);
    
  
  DuoBLE.begin();
  
  DuoBLE.advertisingDataAddName(ADVERTISEMENT, deviceName);
  DuoBLE.setName(deviceName);
   DuoBLE.startAdvertising();
}
//returns a random value less then 50 more then 1
int value(){
    return rand()%maxVal+1;
    
}
//LOOP
void loop()
{   
  
    if (digitalRead(inputPin) == HIGH) {  // check if the input is HIGH
        digitalWrite(ledPin, HIGH);         // turn LED ON if high
    } else {
        digitalWrite(ledPin, LOW);          // turn LED OFF if no input
    }
  //used to test commands and timers in serial monitor
  if(Serial.available()<=0) {  /* Wait for input */

    String command = Serial.readString();   
    if(command == "slow")   // if the string is "slow", the mode is in slow
    {  timerUsed = "slow";
       slowTimer.start();
        fastTimer.stop();
        weirdOne.stop();
        weirdTwo.stop();
        off = false;
        Serial.println(toyPosition);              
    }
    else if(command == "fast")     //fast mode
    {   timerUsed = "fast";
        slowTimer.stop();
        fastTimer.start();
        weirdOne.stop();
        weirdTwo.stop();
       
        Serial.println(toyPosition); 
    }
    else if(command == "random"){ //random mode
      timerUsed = "weird";
        weirdOne.start();
        weirdTwo.start();
        slowTimer.stop();
        Serial.println(toyPosition);
        
    }
    else if(command == "left"){ //move to left
        if(toyPosition > 1){
            toyPosition -= 10;
            myservo.write(toyPosition);
       
        }else{
          toyPosition = 0;
          myservo.write(toyPosition);
        }
        off = false;
        Serial.println(toyPosition); 
   
    }
    else if(command == "right"){ //move to right
        if(toyPosition < 180){
            toyPosition += 10;
            myservo.write(toyPosition);
             
        }else{
          toyPosition = 180;
          myservo.write(toyPosition);
        }
        off = false;
        Serial.println(toyPosition);  
        
    }
    else if(command == "circle"){  //circle
        while (toyPosition < 180){
            toyPosition += 3;
            delay(100);
            digitalWrite(D7, HIGH); // flash the LED (as an indicator)
            myservo.write(toyPosition);
            digitalWrite(D7, LOW);  // turn off LED 
        } 
        off = false;
         Serial.println(toyPosition); 
    }
    else if(command == "off"){ //turn off
        slowTimer.stop();
        fastTimer.stop();
        weirdOne.stop();
        weirdTwo.stop();

        off = true;
        Serial.println("off");
    }
    else if(command == "on"){ //turn on 
        off = false;
        Serial.println(toyPosition);

    }
  }
}
