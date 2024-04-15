#include <ArduinoJson.h>
// Arduino Nano dieu khien May Bom + Lora

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX  Arduino Nano Them Chan RX TX ( Chu Y chi co 8,9,10,11 cuar Nano)

const int  Start_StopPB = 6;// hằng số buttonPin mang giá trị là chân digital được nối với button
const int Motor_Pump = 2;   // hằng số ledPin mang giá trị là chân digital đươc nối với led
// Nut Nhan 1 va den 1
int buttonPushCounter1 = 0;   // số lần button được nhấn
int buttonState1 = 0;         // Start/Stop Local 
int lastButtonState1 = 0;     // Trang thai Run/Stop

uint16_t Level=0;
int Operate =0, AutoMan = 0, Overload=0,CBTrip_Off=0 ;  // 0 Manual ; 1 Auto


int buttonPushCounter2 = 0;   // số lần button được nhấn
int buttonState2 = 0;         // Auto/Man state
int lastButtonState2 = 0;     // trạng thái Auto =1; Man =0
const int AutoMan_LED = 3,Overload_LED =4,CBTrip_LED=5;   // LED ON Auto, LED OFF MANUAL
const int  AutoMantPB = 7;// Auto/Manual Button

#include <arduino-timer.h>
auto timer = timer_create_default(); // create a timer with default settings


void setup() {
  pinMode(Start_StopPB, INPUT_PULLUP); // Cài đặt button là INPUT   // Khai bao inout led 2
  pinMode(Motor_Pump, OUTPUT); // Cài đặt đèn LED là OUTPUT

  pinMode(AutoMantPB, INPUT_PULLUP); // Cài đặt button là INPUT   // Khai bao inout led 2
  pinMode(AutoMan_LED, OUTPUT); // Cài đặt đèn LED là OUTPUT

  pinMode(Overload_LED, OUTPUT);
  pinMode(CBTrip_LED, OUTPUT);
  Serial.begin(9600);
  mySerial.begin(9600);

  timer.every(5000, sendLoradataLevel);
}
 
 
void loop() {
  
 Read_Level();
 ReceiveLora();
  //StartPumpManual();
  //setAutoManual();
  timer.tick();
} // End loop



void Read_Level(){
  // range 8 bit 0 -1024; truyen gia di so thuc sau do ve ESP32 chuyen doi sang muc nuoc
  // VIDU :  0 -1024 tuong ung 4-20ma  ; 0-5Met Nuoc   ;  Level_Met = Level*5 / 1023
Level= analogRead(A0);
}
//-------------------------------
void StartPumpManual(){
// Manual Push button Start Pump
if (buttonPushCounter1==3){ // gioi han Bo nho counter
  buttonPushCounter1=1;
}
 // đọc giá trị hiện tại của button
  buttonState1 = digitalRead(Start_StopPB);
  
  // so sánh với giá trị trước đó
  if (buttonState1 != lastButtonState1) {
    if (buttonState1 == HIGH) {
      buttonPushCounter1++;   
    }
  }
  // lưu lại trạng thái button cho lần kiểm tra tiếp theo
  lastButtonState1 = buttonState1;

  if (buttonPushCounter1 % 2 == 0) { 
    Operate=1;
    sendLoraOperate();
  } else {
   Operate=0;
    sendLoraOperate();
  }
       
  }

  //--------------------------

  void setAutoManual(){

if (buttonPushCounter2==3){ // gioi han Bo nho counter
  buttonPushCounter2=1;
}
 // đọc giá trị hiện tại của button
  buttonState2 = digitalRead(AutoMantPB);
  
  // so sánh với giá trị trước đó
  if (buttonState2 != lastButtonState2) {
    if (buttonState2 == HIGH) {
      buttonPushCounter2++;   
    }
  }
  // lưu lại trạng thái button cho lần kiểm tra tiếp theo
  lastButtonState2 = buttonState2;

  if (buttonPushCounter2 % 2 == 0) { 
    AutoMan = 1; // Auto Pump Dk bang level
    sendLoraAutoMan();
    } else {
    AutoMan = 0; // Manual pump DK bang nut nhan
   sendLoraAutoMan();
   
   }
          
  }

//--------------------
void ReceiveLora(){ 
if (Serial.available()>0){                   // ktra trang thai Serial
  String inputdata = Serial.readString();    

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
  serializeJson(doc, Serial);
}

  void sendLoraAutoMan(){
  StaticJsonDocument<10> doc;//V6
  doc["AutoMan"]=AutoMan;
  serializeJson(doc, Serial);
}
  
void sendLoraOperate(){
  StaticJsonDocument<10> doc;//V6
  doc["Operate"]=Operate;
  serializeJson(doc, Serial);
}




 
