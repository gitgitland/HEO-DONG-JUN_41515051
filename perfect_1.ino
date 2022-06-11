//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <Servo.h> 

#define FIREBASE_HOST "smart-home-b-496e2-default-rtdb.firebaseio.com/" //Without http:// or https:// schemes
#define FIREBASE_AUTH "wuGr8bB70Y2PlcjsJUgS7A7s1zxX4Opo1SxdHlcj"
#define WIFI_SSID "SK_WiFiGIGA2D4A_2.4G"
#define WIFI_PASSWORD "tomato4301"


//Define FirebaseESP8266 data object
FirebaseData firebaseData1;
FirebaseData firebaseData2;

unsigned long sendDataPrevMillis = 0;

String path = "/2";

uint16_t count = 0;
//int gpio13Led = 13;  //Sonoff
int gpio13Led = LED_BUILTIN;  // D1 mini
int gpio12Relay = 12;

void printResult(FirebaseData &data);
void printResult(StreamData &data);
void outResult(StreamData &data);


Servo servoLeft1;
Servo servoLeft2;
Servo servoRight1;
Servo servoRight2;
int nData[100],volume=8;  //volume : nData[]의 개수
int iTime=500;  // 로봇 동작속도
int onValue=0;



void streamCallback(StreamData data)
{

  Serial.println("Stream Data1 available...");
  Serial.println("STREAM PATH: " + data.streamPath());
  Serial.println("EVENT PATH: " + data.dataPath());
  Serial.println("DATA TYPE: " + data.dataType());
  Serial.println("EVENT TYPE: " + data.eventType());
  Serial.print("VALUE: ");
  printResult(data);
  Serial.println();
  outResult(data);
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
  {
    Serial.println();
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }
}

void setup()
{
  Serial.begin(115200);
  servoLeft1.attach(5);  // attaches the servo on GIO5 to the servo object 
  servoLeft2.attach(4);  
  servoRight1.attach(0); 
  servoRight2.attach(2); 
  
  pinMode(gpio13Led, OUTPUT);
  digitalWrite(gpio13Led, HIGH);
  pinMode(gpio12Relay, OUTPUT);
  digitalWrite(gpio12Relay, HIGH);
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData1.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData1.setResponseSize(1024);


  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData2.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData2.setResponseSize(1024);



  if (!Firebase.beginStream(firebaseData1, path))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + firebaseData1.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  Firebase.setStreamCallback(firebaseData1, streamCallback, streamTimeoutCallback);
}

void loop()
{
   if(onValue==1)
    DoArray();
}

void printResult(FirebaseData &data)
{

  if (data.dataType() == "int")
    Serial.println(data.intData());
  else if (data.dataType() == "float")
    Serial.println(data.floatData(), 5);
  else if (data.dataType() == "double")
    printf("%.9lf\n", data.doubleData());
  else if (data.dataType() == "boolean")
    Serial.println(data.boolData() == 1 ? "true" : "false");
  else if (data.dataType() == "string")
    Serial.println(data.stringData());
  else if (data.dataType() == "json")
  {
    Serial.println();
    FirebaseJson &json = data.jsonObject();
    //Print all object data
    Serial.println("Pretty printed JSON data:");
    String jsonStr;
    json.toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println();
    Serial.println("Iterate JSON data:");
    Serial.println();
    size_t len = json.iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++)
    {
      json.iteratorGet(i, type, key, value);
      Serial.print(i);
      Serial.print(", ");
      Serial.print("Type: ");
      Serial.print(type == FirebaseJson::JSON_OBJECT ? "object" : "array");
      if (type == FirebaseJson::JSON_OBJECT)
      {
        Serial.print(", Key: ");
        Serial.print(key);
      }
      Serial.print(", Value: ");
      Serial.println(value);
    }
    json.iteratorEnd();
  }
  else if (data.dataType() == "array")
  {
    Serial.println();
    //get array data from FirebaseData using FirebaseJsonArray object
    FirebaseJsonArray &arr = data.jsonArray();
    //Print all array values
    Serial.println("Pretty printed Array:");
    String arrStr;
    arr.toString(arrStr, true);
    Serial.println(arrStr);
    Serial.println();
    Serial.println("Iterate array values:");
    Serial.println();
    for (size_t i = 0; i < arr.size(); i++)
    {
      Serial.print(i);
      Serial.print(", Value: ");

      FirebaseJsonData &jsonData = data.jsonData();
      //Get the result data from FirebaseJsonArray object
      arr.get(jsonData, i);
      if (jsonData.typeNum == FirebaseJson::JSON_BOOL)
        Serial.println(jsonData.boolValue ? "true" : "false");
      else if (jsonData.typeNum == FirebaseJson::JSON_INT)
        Serial.println(jsonData.intValue);
      else if (jsonData.typeNum == FirebaseJson::JSON_DOUBLE)
        printf("%.9lf\n", jsonData.doubleValue);
      else if (jsonData.typeNum == FirebaseJson::JSON_STRING ||
               jsonData.typeNum == FirebaseJson::JSON_NULL ||
               jsonData.typeNum == FirebaseJson::JSON_OBJECT ||
               jsonData.typeNum == FirebaseJson::JSON_ARRAY)
        Serial.println(jsonData.stringValue);
    }
  }
}

void printResult(StreamData &data)
{

  if (data.dataType() == "int")
    Serial.println(data.intData());
  else if (data.dataType() == "float")
    Serial.println(data.floatData(), 5);
  else if (data.dataType() == "double")
    printf("%.9lf\n", data.doubleData());
  else if (data.dataType() == "boolean")
    Serial.println(data.boolData() == 1 ? "true" : "false");
  else if (data.dataType() == "string")
    Serial.println(data.stringData());
  else if (data.dataType() == "json")
  {
    Serial.println();
    FirebaseJson *json = data.jsonObjectPtr();
    //Print all object data
    Serial.println("Pretty printed JSON data:");
    String jsonStr;
    json->toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println();
    Serial.println("Iterate JSON data:");
    Serial.println();
    size_t len = json->iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++)
    {
      json->iteratorGet(i, type, key, value);
      Serial.print(i);
      Serial.print(", ");
      Serial.print("Type: ");
      Serial.print(type == FirebaseJson::JSON_OBJECT ? "object" : "array");
      if (type == FirebaseJson::JSON_OBJECT)
      {
        Serial.print(", Key: ");
        Serial.print(key);
      }
      Serial.print(", Value: ");
      Serial.println(value);
    }
    json->iteratorEnd();
  }
  else if (data.dataType() == "array")
  {
    Serial.println();
    //get array data from FirebaseData using FirebaseJsonArray object
    FirebaseJsonArray *arr = data.jsonArrayPtr();
    //Print all array values
    Serial.println("Pretty printed Array:");
    String arrStr;
    arr->toString(arrStr, true);
    Serial.println(arrStr);
    Serial.println();
    Serial.println("Iterate array values:");
    Serial.println();

    for (size_t i = 0; i < arr->size(); i++)
    {
      Serial.print(i);
      Serial.print(", Value: ");

      FirebaseJsonData *jsonData = data.jsonDataPtr();
      //Get the result data from FirebaseJsonArray object
      arr->get(*jsonData, i);
      if (jsonData->typeNum == FirebaseJson::JSON_BOOL)
        Serial.println(jsonData->boolValue ? "true" : "false");
      else if (jsonData->typeNum == FirebaseJson::JSON_INT)
        Serial.println(jsonData->intValue);
      else if (jsonData->typeNum == FirebaseJson::JSON_DOUBLE)
        printf("%.9lf\n", jsonData->doubleValue);
      else if (jsonData->typeNum == FirebaseJson::JSON_STRING ||
               jsonData->typeNum == FirebaseJson::JSON_NULL ||
               jsonData->typeNum == FirebaseJson::JSON_OBJECT ||
               jsonData->typeNum == FirebaseJson::JSON_ARRAY)
        Serial.println(jsonData->stringValue);
    }
  }
}

void DoArray() {
  int i,j,k;
  k=volume/4;
  for(i=0 ; i<volume ; i++) {
    j=i%4;
    Serial.println(j);
    Serial.println(nData[i]);
    if(j==0)
       servoLeft1.write(nData[i]);
    else if(j==1)
       servoLeft2.write(nData[i]);
    else if(j==2)
       servoRight1.write(nData[i]);
    else if(j==3)
       servoRight2.write(nData[i]);

    delay(iTime);
  }
}

void InitSet() {
  servoLeft1.write(90);
  delay(iTime);
  servoLeft2.write(90);
  delay(iTime);
  servoRight1.write(90);
  delay(iTime);  
  servoRight2.write(90);
  delay(iTime);   
}

void outResult(StreamData &data)
{
//  int onValue=1;
  int preonValue;        // 추가한 part(여기가 핵심!!!)
  preonValue=onValue;
  if (data.dataType() == "int")
    onValue=data.intData();
  else if (data.dataType() == "string") {
    if(data.stringData() != "1")
      onValue=0;
    Serial.println(data.stringData());
  }
  else if (data.dataType() == "json")
  {
    FirebaseJson *json = data.jsonObjectPtr();
    String jsonStr;
    json->toString(jsonStr, true);

    FirebaseJsonData jsonObj;
    json->get(jsonObj,"on");
    onValue=jsonObj.intValue;
  }

  
  if(onValue==1) {
    Serial.println("Led Turned ON");                         
    digitalWrite(gpio13Led, LOW);
    digitalWrite(gpio12Relay, HIGH);    
  }
  else {
    Serial.println("Led Turned OFF");
    digitalWrite(gpio13Led, HIGH);
    digitalWrite(gpio12Relay,LOW); 
  }
  Serial.println("onValue : ");
  Serial.println(onValue);

 if(onValue==0) {
    InitSet();                         
  }

//  if(onValue==0) {
//    volume=8;
//    nData[0]=90; nData[1]=90;  nData[2]=90;  nData[3]=90;
//    nData[4]=90;  nData[5]=90; nData[6]=90; nData[7]=90;                  
//  }

  else if(onValue==1){
    volume=8;
    nData[0]=110; nData[1]=70;  nData[2]=70;  nData[3]=110;
    nData[4]=70;  nData[5]=110; nData[6]=110; nData[7]=70;
  }
}
