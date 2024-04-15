
BlynkTimer timer;  // blynk timer
BlynkTimer timer1;
//BlynkTimer timer2;
WidgetLED pump(V2);
WidgetLED overload(V3);
WidgetLED CBtrip(V4);
// blynk variable
bool enableblynk = false,BlkStatus=false; bool EnbVal=false;

//String      BLYNKServerVal,Auth_TokenVal,PORTVal, Interval_blkVal, BlynkEnableVal,BLYNK_Saved; // data in EEPROM    
String      BLYNKServer,Auth_Token,PORT,Interval_blk,BlynkEnable,BLYNK_Save; // data from web
bool blynkConnect() {
        if ((enableblynk==true)and(EnbVal==false)){          
        Blynk.connectWiFi(ssid, passwordwifi); // used with Blynk.connect() in place of Blynk.begin(auth, ssid, pass, server, port);                 
        Blynk.config(Auth_Token.c_str(), BLYNKServer.c_str(), PORT.toInt());
        Blynk.connect();  
        Serial.println("Blynk server connected and running ");
        EnbVal =true; // bien byte nay de gioi han chuong trinh blynkconnect chi chay duoc 1 lan khi chung ta register, neu ko no se chay loop trong chuong trinh.
        return true;
      } else {return false;}
}
//----------- CHECK BLYNK Connection-------------------------------------
void CheckConnection(){    // check every 11s if connected to Blynk server

}

//------------
//OPEN SCREEN
    BLYNK_WRITE(V0) // Pump Operate
    {
      int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
    
      if (pinValue==1){
      Operate=!Operate; // Push Button On/Off Pump
      digitalWrite(Dout[0],Operate);         
        } 
       }

    BLYNK_WRITE(V1) // Auto /Man
    {
     int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
    
      if (pinValue==1) {
      AutoMan=!AutoMan;
      digitalWrite(Dout[1],AutoMan);
      }
      }
    
    BLYNK_WRITE(V9) // Tank Max
    {
      float pinValue = param.asFloat(); // assigning incoming value from pin V1 to a variable
      TankMaxset = pinValue;
     }
    BLYNK_WRITE(V10) // Set Start pump by Level
    {
      float pinValue = param.asFloat(); // assigning incoming value from pin V1 to a variable
      if(pinValue>0 && pinValue<TankMax) {
      Startset = pinValue;
      } else{Startset = 0;}
      }
    
    BLYNK_WRITE(V11) // Set Stop pump by Level
    {
      float pinValue = param.asFloat(); // assigning incoming value from pin V1 to a variable
      if (pinValue<TankMax) {
      Stopset = pinValue;
      } else {Stopset=TankMax;}
      }
    
   
        
    BLYNK_WRITE(V12) // SET BUTTON
    {
      int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable    
      if (pinValue==1) {
      TankMax=TankMaxset;
      SetStart=Startset;
      SetStop=Stopset;
      writeSettingFLASH();
      }
      }
//----------------------------------BLYNK---------------------------------
// Child program

bool read_Bink_FLASH_Started(){
           
 File DataFile = Filesystem.open("/blk.txt", "r");
  if (!DataFile) {
    Serial.println("Failed to open Data file");
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
    Serial.println("Failed to parse config file");
    return false;
  }

    const char* T_BLYNKServer=doc["BLKSer"];
    const char* T_Auth_Token =doc["Auth"];
    const char* T_PORT=doc["Port"];
    const char* T_Interval_blk= doc["Interval"];
    const char* T_BlynkEnable=doc["BlkEnb"];
   
     BLYNKServer= String( T_BLYNKServer);
     Auth_Token=String(T_Auth_Token);
     PORT=  String(T_PORT);
     Interval_blk= String(T_Interval_blk);
     BlynkEnable= String(T_BlynkEnable);

      if (BlynkEnable=="Enable"){EnbVal =false;enableblynk = true;}                    
          else{enableblynk = false;}  
      if (BLYNKServer != "") {
      Serial.println("Blynk Server is: "+BLYNKServer);
      Serial.println("Auth Token   is: " +Auth_Token);
      Serial.println("PORT Value   is: " +PORT);
      Serial.println("Interval Time is: " +Interval_blk);
      Serial.println("BlynEnable   is: " +BlynkEnable);
      } else {  Serial.println("BLYNK SERVER is not config");}
 DataFile.close(); // Khi mo File ra thi phai CLOSE FILE lai, Neu ko se ko doc ghi dc nua
 //return true;

}

bool BlynkwriteFLASH() {  // save data in flash
                         
  StaticJsonDocument<200> doc;  // JSON DATA ADDING
   // Setting Date and Time Save // STRING
  doc["BLKSer"]= BLYNKServer;//D_SaveEnergy;
  doc["Auth"]= Auth_Token;
  doc["Port"] = PORT; // String
  doc["Interval"]= Interval_blk;//D_SaveEnergy;
  doc["BlkEnb"]= BlynkEnable;
  
// Json data: { MonData:{"Jan":"1",Feb:"2"..... Dec         }  WkData:{"Mon":"2",  ....."Sun":"1"}     } 
  File DataFile = Filesystem.open("/blk.txt", "w");
  if (!DataFile) {
    //Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, DataFile);
    DataFile.close();
  return true;

 Serial.println("Write BLYNK Setting to FLASH Sussesful");    
}


//---------------------
void LedWidgetdisplay()
{
  //Serial.println("Alarm set");
  //Serial.println(AlarmSet);
  if (Operate){  pump.on();}
   else {  pump.off(); }

    if (Overload){  overload.on();}
   else {  overload.off(); }
   
    if (CBTrip_Off){  CBtrip.on();}
   else {  CBtrip.off(); }


}
//-------------------------
// BLYNK TIMER UPDATE


void Send_Blk_Mqtt(){
if (!Blynk.connected()){
    BlkStatus=false; EnbVal=false;  //Serial.println("Blink Not connected");
    }
    else{BlkStatus=true;  //Serial.println("Blynk Server connected successfull");
      }
 
 if (enableblynk){ 
   if(BlkStatus==true){  
 Operate =    digitalRead(Dout[0]);
 AutoMan =    digitalRead(Dout[1]);
 CBTrip_Off = digitalRead(Din[0]);
 Overload=    digitalRead(Din[1]);    
  enableblynk1=enableblynk;
  BlkStatus1=BlkStatus;


Blynk.virtualWrite(V0, digitalRead(Dout[0])); // Pump On/Off
Blynk.virtualWrite(V1, digitalRead(Dout[1])); // Auto/Man
Blynk.virtualWrite(V2, Operate); // Pump On = V0

Blynk.virtualWrite(V3, Overload); // Overload
Blynk.virtualWrite(V4, CBTrip_Off); // trip/CB Off

Blynk.virtualWrite(V5, String(TankMax)); // gan String vao de giam so 5.100 sang 5.1
Blynk.virtualWrite(V6, String(SetStart));   
Blynk.virtualWrite(V7, String(SetStop));   
Blynk.virtualWrite(V8, String(Level));   


Blynk.virtualWrite(V13, String(host));
Blynk.virtualWrite(V14, WiFi.localIP().toString());
String RssiSent = String(dBmtoPercentage(WiFi.RSSI()));
Blynk.virtualWrite(V15,RssiSent); 
}
       else {Serial.println("Blynk try to connect again");
       EnbVal==false;
       blynkConnect();     
       }
    }else{ 
      //Serial.println("Blynk is Not enabled");
     BlkStatus=false;
    }


// Publish to MQTT
mqttPublish();
}

//---BLYNK---------------------------------BLYNK-------------------------------BLYNK----------------------------------------
