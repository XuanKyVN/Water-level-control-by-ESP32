  
  // Chuong trinh giam sat va dieu khien muc Nuoc trong Be LORA
  //Chuong trinh co the cai dat cho ESP8266 hoac ESP32
#define BLYNK_TIMEOUT_MS  30000  // must be BEFORE BlynkSimpleEsp8266.h doesn't work !!!
#define BLYNK_HEARTBEAT   1700

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "LittleFS.h"
#define Filesystem LittleFS
#include <BlynkSimpleEsp8266.h>
int Dout[]={5,4,13,15}; // Pump,AutoMan,Overload,CBtrip
int Din[] ={0,16}; // Overload , TripCB

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(13, 15); //Define hardware connections 13,15

#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <FS.h>
#define Filesystem LITTLEFS
#include "LITTLEFS.h"
uint64_t chipid32;  
int Dout[] ={33,32,25,26}; // 25 Overload indicator, 26 CB trip Indicator
int Din[] ={35,34,27}; // Overload , TripCB, disBlk_mqtt pin27
#define RXD2 16
#define TXD2 17

#endif
//Add a below line for AutoConnect.
#include <AutoConnect.h>
#include <EEPROM.h>
//----------------------------------
#define DBG_OUTPUT_PORT Serial
const char* ssid;
const char* passwordwifi;
#if defined(ARDUINO_ARCH_ESP8266)
//const char* host = "esp8266fs";
String host = "esp8266fs";
ESP8266WebServer server(80);
#elif defined(ARDUINO_ARCH_ESP32)
//const char* host = "esp32fs";
String host = "esp32fs";
WebServer server(80);
#endif
//-------------------------------
//Add a below line for AutoConnect.
AutoConnect       portal(server);
AutoConnectConfig config;
// NPT Time Client Reading
//-------------------------
#include <Pushbutton.h>

#if defined(ARDUINO_ARCH_ESP8266)
Pushbutton button1(14); //Button Start/Stop
Pushbutton button2(12); // Button Auto/Man


#elif defined(ARDUINO_ARCH_ESP32)
Pushbutton button1(22);//Button Start/Stop
Pushbutton button2(23); // Button Auto/Man
#endif
bool state1 = 1;        // Save trang thai button 1
bool state2 = 1;
// Globle variable
int addr = 700; // address of eeprom
//#define LED  16 // Display the AP running status //D0 tren con V1 NodeMCU  
const int RSSI_MAX =-50;// define maximum strength of signal in dBm
const int RSSI_MIN =-100;// define minimum strength of signal in dBm
// state[10]; byte lightState[10];
uint8_t retry = 5;
unsigned int  Interval = 10000; // 10sec
unsigned long lastPub = 0;
//--------------------Tank------
//Readback = {"TankMax": 5.0, "SetStart": 0.0, "SetStop": 2.0, "AutoMan": 0, "Operate":0, "Level": 3.0, CBTrip_Off:0, Overload: 0, wifiSig:100};
float TankMax= 5.9 ,SetStart=1.3,SetStop=4.5 ,Level=2.3;
float TankMaxset,Startset, Stopset;
uint8_t AutoMan,Operate,CBTrip_Off,Overload;
bool last_state, enb_Blk_Mqtt;
//-------------------------------------
#include "NtpReadSerial.h"
//--------------------------------------
//--------------------------------------
#include "WebPageFS.h"
//MQTT//--------------------------MQTT------------------------------------------------
#include "Mqtt.h"
//------------  BLYNK------------------------------------------------------BLYNK-----
#include "Blynk.h"
//-------------------------------------END BLYNK-------------------------------------

void handle_setupdata(){
   String dataname[20];
   String datasetup[20];
   String fbwebsetup;
   if (server.args() > 0 ) { // Arguments were received
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      dataname[i] = server.argName(i);
      datasetup[i] = server.arg(i);
      //Serial.print(dataname[i]);
      //Serial.print(": ");
      //Serial.println(datasetup[i]);
        }

    //-----------------------------------------------MQTT-------------------------------
    if (dataname[0].indexOf("mqtt") >=0){ // neu data nhan duoc la mqtt setup, trong chuoi co mqtt
      mqttserver=datasetup[0];
      channelid=datasetup[1];
      username=datasetup[2];
      PassWord=datasetup[3];
      TopicPub=datasetup[4];
      TopicSub=datasetup[5];
      updateInterval=datasetup[6];
      MqttEnable=datasetup[7];
      fbwebsetup = "<html><body><h3> MQTT Setting Value </h3>";
      fbwebsetup += "<p>MQTT Server is: " +  mqttserver + " <br>";
      fbwebsetup+= "MQTT Channel ID is: " + channelid +"<br> ";
      fbwebsetup+= "MQTT username is: " +username+" <br>";
      fbwebsetup+= "MQTT password: " +PassWord+" <br>";
      fbwebsetup+= "Topic Subscribe: " +username+"/"+channelid +"/"+  TopicSub+" <br>";
      fbwebsetup+= "Topic publish: " +username+"/"+channelid +"/"+ TopicPub+" <br>";
      fbwebsetup+="Update interval: "+ updateInterval+" <br>";
      fbwebsetup+="Mqtt Enable is: "+MqttEnable+" <br></p>";
      fbwebsetup  += "<a href=\"/setup\"> Setting </a> <br>";
      fbwebsetup+= "<a href=\"/\"> Home</a> <body><html>";
      
      String a = MqttEnable;
      if (a == "Enable") {
        EnbValMqtt = true; // see the API Autoconnect
        Serial.println("Enable MQTT, Writing data to OR EEPROM");
      }
      else {
      EnbValMqtt = false;
      Serial.println("MQTT Disabled from server");
      }

      MqttwriteFLASH();
       
    }
  
    
    //--------------------------------------------------------BLYNK-------------------------------------
    else{ //nguoc lai ko phai la mqtt, thi la blynk set
      BLYNKServer= datasetup[0];
      Auth_Token=  datasetup[1];
      PORT=        datasetup[2];
      Interval_blk=datasetup[3];
      BlynkEnable=datasetup[4];
      fbwebsetup = "<html><body><h3> BLYNK Setting Value </h3>";
      fbwebsetup  += "<p>Blynk server is: " + BLYNKServer+" <br>"; 
      fbwebsetup  += "Blynk authorize token is: "+Auth_Token+" <br>";
      fbwebsetup  += "Blynk Port is: " +PORT+" <br>";
      fbwebsetup  += "Update interval is: "+Interval_blk+" <br>";
      fbwebsetup  += "Blynk Enable is: "+BlynkEnable+" <br></p>";
      fbwebsetup  += "<a href=\"/setup\"> Setting </a><br> ";
      fbwebsetup  += "<a href=\"/\"> Home</a> <body><html>";
      //Serial.println(fbwebsetup);
      String a =BlynkEnable;
      
    if (a=="Enable"){ // see the API Autoconnect
      EnbVal=0;   // this valvue use to limit the blynk configuration loop only one time during configuration
      if (EnbVal==0) { enableblynk = true; Serial.println("Blynk Enabled");}
      else { enableblynk = false; Serial.print("blynk running");}
     }    
      else { enableblynk=false;EnbVal=0; Serial.println("Blynk Disabled from server");
      } 
    
    BlynkwriteFLASH(); 
    
    }// Ket thuc neu La BLYNK     

   }
    
    server.send(200, "text/html", fbwebsetup);
}

//-----------------------------------------------------------
bool whileCP(void) {
  bool  rc;
  // Here, something to process while the captive portal is open.
  // To escape from the captive portal loop, this exit function returns false.
  // rc = true;, or rc = false;
   //rc = false;
        config.immediateStart=true;
        portal.config(config);
        rc = true;
    return rc;
}
//----------------------
bool SetupwriteFLASH(StaticJsonDocument<200> doc) {  // save data in flash                       
   
  File DataFile = Filesystem.open("/setdata.txt", "w");
  if (!DataFile) {
    Serial.println("Failed to write Setdata.txt file");
    return false;
  }
  serializeJson(doc, DataFile);
    DataFile.close();
  return true;

 Serial.println("Write Setting Data to FLASH Sussesful");    
}

//---------------
void readSetupdata(StaticJsonDocument<200> doc){
      TankMax=    doc["TankMax"].as<float>();       
      SetStart=   doc["SetStart"].as<float>();       
      SetStop=    doc["SetStop"].as<float>(); 
      AutoMan=    doc["AutoMan"].as<uint8_t>(); 
}
//---------------
bool read_SetData_FLASH(){
           
 File DataFile = Filesystem.open("/setdata.txt", "r");
  if (!DataFile) {
    Serial.println("Failed to read Setdata.txt file");
    return false;
  }

  size_t size = DataFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use DataFile.readString instead.
  DataFile.readBytes(buf.get(), size);

  StaticJsonDocument<200> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse Set data config file");
    return false;
  }
        readSetupdata(doc);
      
      Serial.println("Tank Max is: "+String(TankMax));
      Serial.println("Set Start   is: " +String(SetStart));
      Serial.println("Set Stop   is: " +String(SetStop));
      Serial.println("Auto/Manual is: " +String(AutoMan));
      
      
 DataFile.close(); // Khi mo File ra thi phai CLOSE FILE lai, Neu ko se ko doc ghi dc nua
 //return true;

}



//--------------------------------------------------------------------------------------- SPIFFS DATA SAVE------------------------
void handle_setState() {
  String temp;
   if (server.args() > 0 ) { // Arguments were received
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      temp = server.argName(i);
      String data_input = server.arg(i);
      // Serial.print(server.argName(i)); // Display the argument =
      if (temp=="Operate"){
        int valdata=data_input.toInt();
        if (valdata==0){          
           digitalWrite(Dout[0],LOW); //LED ON
            Operate=0;       
        }              
               else
               {
                digitalWrite(Dout[0],HIGH); //LED OFF
                Operate=1;
                }
        }            
      //--------------------------
        if (temp=="SetAuto"){
          int valdata=data_input.toInt();
        if (valdata==0){           
           digitalWrite(Dout[1],LOW); //LED ON
          AutoMan =0;
          }              
               else
               {
                digitalWrite(Dout[1],HIGH); //LED OFF
                AutoMan=1;
                }
        }     
        //--------------------------
      if (temp=="SetOperate"){
        
        StaticJsonDocument<200> doc;//V6
      deserializeJson(doc, data_input);
      //{"TankMax": 5.5, "SetStart":1.5 , "SetStop": 4.5, "AutoMan": 0}
     serializeJson(doc, Serial);        

     SetupwriteFLASH(doc); 
     readSetupdata(doc);  
      }


        
         
  }             
 }

 handle_FBtoWeb();
}      
     
    //----------------------------------------=-------------

void handle_FBtoWeb(){
       uint8_t state[5];
          
          for (int i=0; i<2;i++){
          if (digitalRead(Dout[i])){              
                state[i] = 1;
               }
               else
               {
                state[i] = 0;  
               }
          }
          // state[0] Pump OFF/ON ; State[1] : Man/Auto
          Operate= state[0];
          AutoMan= state[1];
  String webPage;

  
 StaticJsonDocument<300> doc;//V6
  // Create the root object

//Readback = {"TankMax": 5.0, "SetStart": 0.0, "SetStop": 2.0, "AutoMan": 0, "Operate":0, "Level": 3.0, CBTrip_Off:0, Overload: 0, wifiSig:100};
  doc["TankMax"]=TankMax;
  doc["SetStart"]=SetStart;
  doc["SetStop"]=SetStop;
  doc["AutoMan"]=AutoMan;
  doc["Operate"]=Operate;
  doc["Level"]=Level;
  doc["CBTrip_Off"]=CBTrip_Off;
  doc["Overload"]=Overload;
  doc["blk"]=enableblynk;
  doc["blkst"]=BlkStatus;
  doc["mqtt"]=EnbValMqtt;
  doc["mqttst"]=MqttStatus;
   
    // sent wifi signal to web
   String RssiSent = String(dBmtoPercentage(WiFi.RSSI()));
   doc["wifiSignal"] = RssiSent;
    
  serializeJson(doc, webPage);
  //serializeJson(doc, Serial); // in ra Serial
  //server.send(200, "application/json", webPage);
  server.send(200, "text/json", webPage);
 }

//-------------------------------------------------
void handle_datasetupdisplay(){
   String Setup_fb;  // feedback data setup to Webpage Setup
  StaticJsonDocument<500> doc;//V6
  // Create the root object
// HOST NAME SENT to Client
  doc["HostName"]= String(host);
  doc["IP"]= (WiFi.localIP()).toString();
//MQTT JSON DATA for MQTT
    doc["Mq_server"]= mqttserver;
    doc["Mq_Channel"]= channelid;
    doc["Mq_username"]= username;
    doc["Mq_password"]= PassWord;
    doc["Mq_TopicPub"]= TopicPub;
    doc["Mq_TopicSub"]= TopicSub;
    doc["mq_updateinterval"]=updateInterval;

    doc["Mq_enable"]= MqttEnable;
//BLYNK JSON DATA
    doc["blk_server"]= BLYNKServer;
    doc["blk_auth"]= Auth_Token;
    doc["blkPort"]= PORT;
    doc["blk_interval"]= Interval_blk;
    doc["BlkENB"]= BlynkEnable;
   
    
  serializeJson(doc, Setup_fb);
 // serializeJson(doc, Serial); // in ra Serial
 // server.send(200, "application/json", webPage);
  server.send(200, "text/json", Setup_fb);
}


//----------END----------------------------------------
bool handleFileRead(String path) {
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if (path.endsWith("/")) {
    path += "/index.htm";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
 
  if (Filesystem.exists(pathWithGz) || Filesystem.exists(path)) {
    if (Filesystem.exists(pathWithGz)) {
      path += ".gz";
    }
    File file = Filesystem.open(path, "r");

    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}
String getContentType(String filename) {
  if (server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
    } else if (filename.endsWith(".csv")) {
    return "text/csv";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}
//---------------------------------
void onConnect(IPAddress& ipaddr) {
  Serial.print("WiFi connected with ");
  Serial.println(WiFi.SSID());
  Serial.print("WiFi Password: ");
  Serial.println(WiFi.psk());
  Serial.print(", IP:");
  Serial.println(ipaddr.toString());
//Serial.printf("password: %s ", WiFi.psk().c_str() ); // conver to String
//Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
//Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
  ssid =WiFi.SSID().c_str(); // constant char*
  passwordwifi= WiFi.psk().c_str();
}
//---------------------------------------------------
void setup(void){
    //-----------
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.print("\n");
  DBG_OUTPUT_PORT.setDebugOutput(true);
  // Pin Mode
   for (int i=0;i<4;i++){
  pinMode(Dout[i],OUTPUT);
  
 }
  for (int i=0;i<3;i++){
  pinMode(Din[i],INPUT_PULLUP);
  
 }
//EEPROM.begin(addr);  //Initialize EEPROM
mqttClient.setCallback(callback);
timer.setInterval(10000L, CheckConnection); // check if still connected every 60s  BLYNK
timer1.setInterval(3000L, Send_Blk_Mqtt);

  #ifdef ESP8266
  config.apid = "ESP-" + String(ESP.getChipId(), HEX);
  host = "esp-" + String(ESP.getChipId(), HEX);
  //mySerial.begin(9600);
  
  #else //ESP32
  chipid32=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
  uint16_t a= chipid32>>32; // lay 2byte phia tren
  host="esp-"+String(a,DEC);
  
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); //Define Serial 2 Pin
  #endif
  Serial.println("Enter link below to enter webserver");Serial.print(host); Serial.println(".local");
  config.psk = 12345678;
  config.title = "ESP-Setup";
  portal.config(config);
  
  //config.portalTimeout = 30000;  // Captive Portal Se bi mat sau thoi gian 30s. neu Wifi Mat, captive Portal se chay va thoat trong 30s neu ko lam j het.
  config.ota = AC_OTA_BUILTIN;
  portal.config(config);
  
  //config.autoRise = false;  // Mac dinh True
  //config.retainPortal = true; //mac dinh faulse
  // config.autoReset = true;
  //----------------------------------------------------------------------------
  config.autoReconnect = true;    // hai ham nay phai di chung voi nhau
  config.reconnectInterval = 2; // Reconnection attempting interval is 1min.
  //----------------------------------------------------------------------------
  config.ticker = true;  // Wifi status
  config.tickerPort = 2; // 
  config.tickerOn = LOW;
  /*Short blink: The ESP module stays in AP_STA mode.
  Short-on and long-off: No STA connection state. (i.e. WiFi.status != WL_CONNECTED)
  No blink: WiFi connection with access point established and data link enabled. (i.e. WiFi.status = WL_CONNECTED)*/
  //---------------------------------
  
  portal.config(config);
  config.menuItems = AC_MENUITEM_CONFIGNEW | AC_MENUITEM_OPENSSIDS | AC_MENUITEM_RESET | AC_MENUITEM_UPDATE | AC_MENUITEM_HOME;
  portal.config(config);

   #ifdef ESP8266
  if (!LittleFS.begin()) {
  #else
  if (!LITTLEFS.begin(true)) {
  #endif  
    Serial.println("SPIFFS initialisation failed...");
    SPIFFS_present = false; 
  }else{
    Serial.println(F("SPIFFS initialised... file access enabled..."));
    SPIFFS_present = true; }
  
  
//---------------------------------------------------------------------------
   
   portal.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "Website not found , Please contact to phamxuanky82@gmail.com");
  });
// filesystem will be LittleFS if ESP8266, LITTLEFS if ESP32
  server.serveStatic("/setup", Filesystem, "/setup.htm");  // su dung .htm ko su dung html de EDIT duoc
  server.serveStatic("/about",Filesystem,"/about.htm"); // Update Analog value power
   
  server.on("/setupdata_display", handle_datasetupdisplay);
  server.on("/setup_data",handle_setupdata); // update pin value to server
  server.on("/FBtoWeb", handle_FBtoWeb);
  server.on("/setState", handle_setState); // handles button values
  
  
  
///////////////////////////// Server Commands SPIFFS
  server.on("/fs_read",         HomePage);
  server.on("/download", File_Download);
  server.on("/upload",   File_Upload);
  server.on("/fupload",  HTTP_POST,[](){ server.send(200);}, handleFileUploadFS);
  server.on("/stream",   File_Stream);
  server.on("/delete",   File_Delete);
  server.on("/dir",      SPIFFS_dir);

  //Relocation as follows to make AutoConnect recognition.
 portal.whileCaptivePortal(whileCP);   // HAM NAY GIUP KEEP TRACKING WIFI ALL TIME NHE CAC BAN, Neu ko co ham nay thi ko dc nhe
      if (portal.begin()) {
    DBG_OUTPUT_PORT.print(F("Connected! IP address: "));
    DBG_OUTPUT_PORT.println(WiFi.localIP());
    config.immediateStart=false;
    portal.config(config);
  }
  DBG_OUTPUT_PORT.println("HTTP server started");
  if (MDNS.begin(host.c_str())) {
    MDNS.addService("http", "tcp", 80);
    DBG_OUTPUT_PORT.print(F("Open http://"));
    DBG_OUTPUT_PORT.print(host);
    DBG_OUTPUT_PORT.println(F(".local/edit to open the webserver"));
  }
  portal.onConnect(onConnect); 
//---------------------------------
read_MQTT_FLASH_Started(); // READ EEPROM during CPU Reset power supply
read_Bink_FLASH_Started();
read_SetData_FLASH();  
//--------------------
// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  //timeClient.setTimeOffset(0);
timeClient.setTimeOffset(+7*60*60); // VietNam Gmt +7
timeClient.update();  // CAP NHAT THOI GIAN LUC CAU HINH XONG

}
//----------------------------

void readserial(){
  if (Serial.available()>0){    
  String inputdata = Serial.readString();
if(inputdata=="blkdis"){
   enableblynk=false;
   Serial.println("Disable blynk");
}
if(inputdata=="mqdis"){
   EnbValMqtt=false;
      Serial.println("Disable mqtt");
}
if(inputdata=="blkenb"){
   enableblynk=true;
   EnbVal=false;
      Serial.println("enable blynk");
}
if(inputdata=="mqenb"){
   EnbValMqtt=true;
      Serial.println("enable mqtt");
}
  }
} 
//----------------------------
void loop(void){

  portal.handleClient();
#ifdef ARDUINO_ARCH_ESP8266
  MDNS.update();
#endif

//if (WiFi.status() == WL_IDLE_STATUS) {
//#if defined(ARDUINO_ARCH_ESP8266)
//    ESP.reset();
//#elif defined(ARDUINO_ARCH_ESP32)
//    ESP.restart();
//#endif
//    delay(1000);
//  }

if (WiFi.status() == WL_CONNECTED) {
 
//-----------------------------
if(Blynk.connected()){
       if (EnbVal==1){
           Blynk.run();
           
          }
       }
      timer.run();
      timer1.run();
//----------------------------
  if (EnbValMqtt==true&&mqttConnect()==true) { // If Enable to start MQTT Client Client.loop
    mqttClient.loop();
  }
NPT_TimeUpdate();
ReadSerial();
pushbuttonact();
LedWidgetdisplay();
readserial();
}else{
#if defined(ARDUINO_ARCH_ESP32)
    ESP.restart();
#endif
    delay(10000);
}

}
// END OF CHILD PROGRAM------------------------------------

void pushbuttonact(){
  //-----------------------------BUTTON- tac Dong o Muc 0. GND---
if (button1.getSingleDebouncedPress() )
{
state1 =! state1;               //chuyen trang thai Relay 1 PUMP OPERATE
digitalWrite(Dout[0],state1);   // 
Operate=state1;
}
if (button2.getSingleDebouncedPress() )
{
state2 =! state2;               //AutoMANUAL
digitalWrite(Dout[1],state2);
AutoMan=state2;
}
// Enb and Disable Blynk and MQTT

//--------------------------------ReadINPUT ---------
uint8_t stateInput[3];
for (int i=0;i<3;i++){
  if (digitalRead(Din[i])){
    stateInput[i] =1;
  } else{   stateInput[i] =0; }
}

Overload =stateInput[0];
CBTrip_Off=stateInput[1];
digitalWrite(Dout[2],Overload);
digitalWrite(Dout[3],CBTrip_Off);

if (Overload==1|CBTrip_Off==1|Level==TankMax){
  digitalWrite(Dout[0],0);   // 
  Operate=0;
}

if (AutoMan==1){
  
  if(Level>=SetStop){
  digitalWrite(Dout[0],0);   // 
  Operate=0;
} 
  if (Level<=SetStart){
  digitalWrite(Dout[0],1);   // 
  Operate=1;
}

}

   //Enb Blk and Mqtt when server failure, we can access Webserver to config
enb_Blk_Mqtt=stateInput[2];
if (last_state!=enb_Blk_Mqtt){
if (enb_Blk_Mqtt){
  enableblynk=false;BlkStatus=false;
  EnbValMqtt=false; MqttStatus=false;
  Serial.println("Blynk and Mqtt was disabled");
}else{ Serial.println("Blynk and Mqtt Enable");
enableblynk=true; EnbVal=false;
EnbValMqtt=true; 
}
last_state=enb_Blk_Mqtt; // save last state of switch 
}
}
