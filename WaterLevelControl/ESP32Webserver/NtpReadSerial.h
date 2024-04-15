#include <NTPClient.h>
const long utcOffsetInSeconds = 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);// automatic sellect Server nearby

String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
String TimeWrite, MonthWrite, WeekDayWrite,CurrentDate;
int DayWrite,CurrentYear,CurrentMonth,CurrentSecond,CurrentMinute;
uint32_t  TimeUp_Interval = 600000; // 5s Phut cap nhat thoi gian tren mang 1 lan.
unsigned long TimeUp_lastPub = 0;

void NPT_TimeUpdate() {
// NPT CLIENT UPDATE TIME.
 //GIOI HAN THOI GIAN CAP NHAT SERVER, TIME TU DONG CHAY TRONG ESP
  if (TimeUp_Interval > 0) {
    if (millis() - TimeUp_lastPub > TimeUp_Interval) {
       
         timeClient.update();
        
      }     

      TimeUp_lastPub = millis();
  }
  // Get Time
  unsigned long epochTime = timeClient.getEpochTime();
  String formattedTime = timeClient.getFormattedTime();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();
  String weekDay = weekDays[timeClient.getDay()];
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  String currentMonthName = months[currentMonth-1];
  int currentYear = ptm->tm_year+1900;
  //Print complete date:
  String currentDate = String(monthDay) + "/" + String(currentMonth) + "/" + String(currentYear); // day-month-year format
  //READING DATA FROM SERIAL OF NTP
//Epoch Time: 1606836923
//Formatted Time: 15:35:23
//Hour: 15
//Minutes: 35
//Seconds: 23
//Week Day: Tuesday
//Month day: 1
//Month: 12
//Month name: December
//Year: 2020
//Current date: 2020-12-1
// -------------------COPPY DATA TO GLOBLE VARIABLE-----------------------
TimeWrite= formattedTime; //16:50:00
DayWrite = monthDay; // int 1
MonthWrite = currentMonthName;  //"December"
WeekDayWrite = weekDay ; //Monday,
CurrentYear =currentYear;
CurrentSecond=currentSecond;
CurrentDate=currentDate;
CurrentMonth=currentMonth;
CurrentMinute=currentMinute;
}


//----------------------------------CONVERT FLOAT to String-----------------------------------
String convertFloatToString(float Variable)
{ // begin function

  char temp[10];
  String tempAsString;
   
    // perform conversio7
    dtostrf(Variable,7,3,temp);
    
    // create string object
  tempAsString = String(temp);
  
  return tempAsString;
  
} // end function
//--------------------------------------------------------------------------------

void sendtoLoraNano(){

StaticJsonDocument<100> doc1;//V6
doc1["Operate"] =Operate;
doc1["AutoMan"] = AutoMan;
doc1["Overload"] =Overload;
doc1["CBTrip_Off"] = CBTrip_Off;
  #if defined(ARDUINO_ARCH_ESP32)
  serializeJson(doc1, Serial2); // in ra Serial    
  #else //ESP8266
  serializeJson(doc1, Serial); // in ra Serial 
  #endif
}

void ReadSerial(){  // control output by LORA WIFI with Json Packed
   #if defined(ARDUINO_ARCH_ESP32)
     if (Serial2.available()>0){    
     String inputdata = Serial2.readString();
   #else // ESP8266
    if (Serial.available()>0){    
     String inputdata = Serial.readString();
   #endif
        StaticJsonDocument<100> doc;//V6
        deserializeJson(doc, inputdata); // tach Json ra khoi input dua ve, bo ten cua json doccument
                                        //{LevelInt:1024} Or {Operate:1} or {AutoMan: 0}
        bool CheckLevelInt = doc.containsKey("LevelInt");
        bool CheckOperate = doc.containsKey("Operate");
        bool CheckAutoMan = doc.containsKey("AutoMan");
        
        if (CheckLevelInt){   
          int LevelInt = doc["LevelInt"].as<int>();// 
          Level = round(LevelInt*100*TankMax/1023)/100;  // DOi Don vi sang m sau do lam tron 2 con so
        }
           if (CheckOperate) {Operate = doc["Operate"].as<int>();
             digitalWrite(Dout[0],Operate);
             }
                if(CheckAutoMan) {
                  AutoMan = doc["AutoMan"].as<int>();
                  digitalWrite(Dout[1],AutoMan);               
                }  // CheckAutoMan = true
   
// Send data to Serial 2 
sendtoLoraNano();
 }

  uint16_t LevelInt1=random(100,1023);//     
//  if (Interval1 > 0) {
//    if (millis() - lastPub1 > Interval1) {
//        LevelInt = LevelInt +10;
//       if (LevelInt>1023) {LevelInt=0;}
//    lastPub1 = millis();
//     }
//  }
  Level = round(LevelInt1*100*TankMax/1023)/100;  // DOi Don vi sang m sau do lam tron 2 con so
  
}
//----------------
