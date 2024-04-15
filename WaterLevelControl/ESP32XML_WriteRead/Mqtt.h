#include <PubSubClient.h>
//--------------------
WiFiClient   wifiClient;
PubSubClient mqttClient(wifiClient);
// Callback function header
void callback(char* topic, byte* payload, unsigned int length); // gan bien nay vao de goi publish ben trong call back

// global setting variable
String mqttserver,channelid,username,PassWord,TopicPub,TopicSub, updateInterval,MqttEnable; // data from web
bool MqttStatus = false;
bool EnbValMqtt = false;
bool enableblynk1,BlkStatus1;
bool mqttConnect() {

  //uint8_t retry = 5;
  if (!mqttClient.connected()){
      if (EnbValMqtt==true){

    mqttClient.setServer(mqttserver.c_str(), 1883);
    //Serial.println(String("Attempting MQTT broker:") + mqttserver);
    
    if (mqttClient.connect(channelid.c_str(), username.c_str(), PassWord.c_str())) {
      Serial.println("MQTT Broker Connection Established:" + channelid);
      String Sub = username+"/"+channelid + "/" + TopicSub+"/#";
      String Pub = username+"/"+channelid + "/" + TopicPub;
      mqttClient.subscribe(Sub.c_str()); // Subscribe Channel
      mqttClient.publish(Pub.c_str(), "Data_received OK");
      MqttStatus=true; // feed back to Web       
      return true;
    }
    else {
      //Serial.println("MQTT BROKER Connection failed:" + String(mqttClient.state()));
      MqttStatus=false; // feed back to Web
      return false;  
      }   
  }
}// End of main child program
}
// WIFI TO Integer RSSI Quality
int dBmtoPercentage(int dBm)
{
  int quality;
    if(dBm <= RSSI_MIN)
    {
        quality = 0;
    }
    else if(dBm >= RSSI_MAX)
    {  
        quality = 100;
    }
    else
    {
        quality = 2 * (dBm + 100);
   }

     return quality;
}//dBmtoPercentage 
//--------------------------------------
bool writeSettingFLASH() {  // save data in flash
                         
  StaticJsonDocument<200> doc;  // JSON DATA ADDING
   // Setting Date and Time Save // STRING
  doc["TankMax"]= TankMax;
  doc["SetStart"]= SetStart;
  doc["SetStop"] = SetStop; 
  doc["AutoMan"]= AutoMan;
  File DataFile = Filesystem.open("/setdata.txt", "w");
  if (!DataFile) {
    //Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, DataFile);
    DataFile.close();
  return true;

 Serial.println("Write Setting to FLASH Sussesful");    
}

//-----------------------------------
void mqttPublish() {
  String path = username+String("/")+channelid + String("/") + TopicPub;
  String msg;  uint8_t state[5];
 
 if (mqttConnect()==false){
  MqttStatus=false;  //Serial.println("mqtt Not connected");
}  else{
  MqttStatus=true; //Serial.println("mqtt connected"); 
}

if (EnbValMqtt){
  if (MqttStatus==true){
    
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

  
 StaticJsonDocument<500> doc;//V6
  // Create the root object

//Readback = {"TankMax": 5.0, "SetStart": 0.0, "SetStop": 2.0, "AutoMan": 0, "Operate":0, "Level": 3.0, "CBTrip_Off":0, "Overload": 0, "wifiSig":100};
  doc["TankMax"]=TankMax;
  doc["SetStart"]=SetStart;
  doc["SetStop"]=SetStop;
  doc["AutoMan"]=AutoMan;
  doc["Operate"]=Operate;
  doc["Level"]=Level;
  doc["CBTrip_Off"]=CBTrip_Off;
  doc["Overload"]=Overload;
  
  doc["blk"]=enableblynk1;
  doc["blkst"]=BlkStatus1;
  doc["mqtt"]=EnbValMqtt;
  doc["mqttst"]=MqttStatus;
   
    // sent wifi signal to web
   String RssiSent = String(dBmtoPercentage(WiFi.RSSI()));
    doc["wifiSignal"] = RssiSent; 
    doc["Host"]=String(host);
    doc["IP"]=WiFi.localIP().toString();
  serializeJson(doc, msg);
  mqttClient.publish(path.c_str(), msg.c_str());
   
} 
 else{
  
  //Serial.println("Mqtt try to connect again"); 
  mqttConnect() ;   
  
  }
  
  }else {//Serial.println("Mqtt is not enable!");
        MqttStatus=false; 
        }
  }
  
//--------------------------------------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  if (EnbValMqtt) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    String strTopic = String((char*)topic);
    //String path = username+String("/")+channelid + String("/") + TopicPub;
        
    if (strTopic.indexOf(channelid) >= 0)
    {
      // Note Topic will sent lighting tag is D0,D1,D2,D3,D4,D5,D6,D7; We will put data into array here
      if (strTopic.indexOf("Operate") >= 0) { //D0 La PUMP ON/OFF
        if ((char)payload[0] == '1') {
          digitalWrite(Dout[0], HIGH);
          Operate=1;
          mqttPublish();
        } else {
          digitalWrite(Dout[0], LOW);  // Turn the LED off by making the voltage HIGH
          Operate=0;
          mqttPublish();
        }
      }//------------------------
      if (strTopic.indexOf("AutoMan") >= 0) {  // D1 la AUTO/MANUAL
        if ((char)payload[0] == '1') {
          digitalWrite(Dout[1], HIGH);
          AutoMan=1;
          mqttPublish();
        } else {
          digitalWrite(Dout[1], LOW);  // Turn the LED off by making the voltage HIGH
          AutoMan=0;
          mqttPublish();
        }
      }//------------------------

     if (strTopic.indexOf("setup") >= 0) {  // D1 la AUTO/MANUAL
      
        StaticJsonDocument<200> doc;//V6
          deserializeJson(doc, payload, length);

          serializeJson(doc, Serial);
          
      TankMax=    doc["TankMax"].as<float>();       
      SetStart=   doc["SetStart"].as<float>();       
      SetStop=    doc["SetStop"].as<float>(); 
      AutoMan=    doc["AutoMan"].as<uint8_t>();
        
        mqttPublish();
        writeSettingFLASH();
      }//------------------------

      
      }
       
    }
  }
//------------------------------
// Child program---------------------------------------------
bool read_MQTT_FLASH_Started() {
  
File DataFile = Filesystem.open("/mqtt.txt", "r");
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

const char* T_mqttserver=  doc["MqSer"];
const char*  T_channelid=doc["id"];
const char*  T_username=  doc["User"];
const char*  T_PassWord= doc["Pass"];
const char*  T_TopicPub= doc["Pub"];
const char*  T_TopicSub=doc["Sub"];
const char*  T_updateInterval=  doc["Interval"];
const char*  T_MqttEnable= doc["MqEnb"];


 mqttserver= String( T_mqttserver);
 channelid=String(T_channelid);
 username=  String(T_username);
 PassWord= String(T_PassWord);
 TopicPub= String(T_TopicPub);
 TopicSub=String(T_TopicSub);
 updateInterval=  String(T_updateInterval);
 MqttEnable= String(T_MqttEnable);
 
 if (MqttEnable == "Enable") {
    EnbValMqtt = true;
  }
  else {
    EnbValMqtt = false;
  }
 if (mqttserver != "") {
      Serial.println("Mqtt Server is: "+mqttserver);
      Serial.println("ChannelID   is: " +channelid);
      Serial.println("username   is: " +username);
      Serial.println("PassWord is: " +PassWord);
      Serial.println("TopicPub   is: " +TopicPub);
      Serial.println("TopicSub   is: " +TopicSub);
      Serial.println("updateInterval is: " +updateInterval);     
      Serial.println("MqttEnable   is: " +MqttEnable);
 } else {  Serial.println("MQTT Broker is not config");}
      
 DataFile.close(); // Khi mo File ra thi phai CLOSE FILE lai, Neu ko se ko doc ghi dc nua
 return true;


  
}
// END OF CHILD PROGRAM
bool MqttwriteFLASH() {
  StaticJsonDocument<200> doc;  // JSON DATA ADDING
   // Setting Date and Time Save // STRING
  doc["MqSer"]= mqttserver;//D_SaveEnergy;
  doc["id"]= channelid;
  doc["User"] = username; // String
  doc["Pass"]= PassWord;//D_SaveEnergy;
  doc["Pub"]= TopicPub;
  doc["Sub"] = TopicSub; // String
  doc["Interval"]= updateInterval;//D_SaveEnergy;
  doc["MqEnb"]= MqttEnable;
 
  File DataFile = Filesystem.open("/mqtt.txt", "w");
  if (!DataFile) {
    //Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, DataFile);
    DataFile.close();
  return true;
  Serial.println("Write MQTT Setting to FLASH Sussesful");     
  }
//----------------------------------MQTT----------------------------------------------------------------------------------------------
// Child program MQTT
