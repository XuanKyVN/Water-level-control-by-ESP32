#include <ArduinoJson.h>
// Arduino Nano dieu khien May Bom + Lora

#include <SoftwareSerial.h>
SoftwareSerial mySerial(8, 9); // RX, TX  Arduino Nano Them Chan RX TX ( Chu Y chi co 8,9,10,11 cuar Nano)

//const int  Start_StopPB = 6;// hằng số buttonPin mang giá trị là chân digital được nối với button
const int Motor_Pump = 2;   // hằng số ledPin mang giá trị là chân digital đươc nối với led
// Nut Nhan 1 va den 1
#include <Pushbutton.h>
Pushbutton button1(6);//Button Start/Stop
Pushbutton button2(7); // Button Auto/Man
bool state1 = 1;        // Save trang thai button 1
bool state2 = 1;

uint16_t Level=0;
int Operate =0, AutoMan = 0, Overload=0,CBTrip_Off=0 ;  // 0 Manual ; 1 Auto
const int AutoMan_LED = 3,Overload_LED =4,CBTrip_LED=5, Simulate=12;   // LED ON Auto, LED OFF MANUAL

#include <arduino-timer.h>
auto timer = timer_create_default(); // create a timer with default settings

unsigned int  Interval = 2000; // 10sec
unsigned long lastPub = 0;

void setup() {
  pinMode(Simulate, INPUT_PULLUP); // Cài đặt button là INPUT   // Khai bao inout led 2
  //pinMode(AutoMantPB, INPUT_PULLUP); // Cài đặt button là INPUT   // Khai bao inout led 2

  pinMode(Motor_Pump, OUTPUT); // Cài đặt đèn LED là OUTPUT
  pinMode(AutoMan_LED, OUTPUT); // Cài đặt đèn LED là OUTPUT

  pinMode(Overload_LED, OUTPUT);
  pinMode(CBTrip_LED, OUTPUT);

  
  Serial.begin(9600);
  mySerial.begin(9600);

  timer.every(2000, sendLoradataLevel);
}
 
 
void loop() {
  
 Read_Level();
 ReceiveLora();
 pushbuttonact();
 timer.tick();


  
} // End loop


void Read_Level(){
  // range 8 bit 0 -1024; truyen gia di so thuc sau do ve ESP32 chuyen doi sang muc nuoc
  // VIDU :  0 -1024 tuong ung 4-20ma  ; 0-5Met Nuoc   ;  Level_Met = Level*5 / 1023
  // Neu Pin 12 = 0 thi o Che Do SiMULATE
if (digitalRead(Simulate)){Level= analogRead(A0);}
else{  
  //--------------
  if (Interval > 0) {
    if (millis() - lastPub > Interval) {
        Level = Level +10;
       if (Level>1023) {Level=0;}
    lastPub = millis();
     
  }
  }
 //---------------    
  } 
}
//-------------------------------
void pushbuttonact(){
  //-----------------------------BUTTON- tac Dong o Muc 0. GND---
if (button1.getSingleDebouncedPress() )
{
state1 =! state1;               //chuyen trang thai Relay 1 PUMP OPERATE
//digitalWrite(Motor_Pump,state1);   
Operate=state1;
sendLoraOperate();
}
if (button2.getSingleDebouncedPress() )
{
state2 =! state2;               //AutoMANUAL
//digitalWrite(AutoMan_LED,state2);
AutoMan=state2;
sendLoraAutoMan();
}
}
//--------------------
void ReceiveLora(){ 
if (mySerial.available()>0){                   // ktra trang thai Serial
  String inputdata = mySerial.readString();    

    StaticJsonDocument<100> doc;//V6
    deserializeJson(doc, inputdata);
    
    Operate = doc["Operate"].as<int>();
    digitalWrite(Motor_Pump,Operate);
    AutoMan = doc["AutoMan"].as<int>();
    digitalWrite(AutoMan_LED,AutoMan);
    Overload = doc["Overload"].as<int>();
    digitalWrite(Overload_LED,Overload);
    CBTrip_Off = doc["CBTrip_Off"].as<int>();
    digitalWrite(CBTrip_LED,CBTrip_Off);
           
      }
}

void sendLoradataLevel(){
  StaticJsonDocument<10> doc;//V6
  doc["LevelInt"]=Level;
  serializeJson(doc, mySerial);
}

  void sendLoraAutoMan(){
  StaticJsonDocument<10> doc;//V6
  doc["AutoMan"]=AutoMan;
  serializeJson(doc, mySerial);
}
  
void sendLoraOperate(){
  StaticJsonDocument<10> doc;//V6
  doc["Operate"]=Operate;
  serializeJson(doc, mySerial);
}




 
