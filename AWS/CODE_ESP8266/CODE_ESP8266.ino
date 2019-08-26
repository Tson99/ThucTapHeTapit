#include <ArduinoJson.h>
#include <Arduino.h>
#include <Stream.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

//AWS
#include "sha256.h"
#include "Utils.h"


//WEBSockets
#include <Hash.h>
#include <WebSocketsClient.h>

//MQTT PUBSUBCLIENT LIB 
#include <PubSubClient.h>

//AWS MQTT Websocket
#include "Client.h"
#include "AWSWebSocketClient.h"
#include "CircularByteBuffer.h"

extern "C" {
  #include "user_interface.h"
}

//AWS IOT config, change these:
char wifi_ssid[]       = "IoT-Research";
char wifi_password[]   = "Tapit168";
char aws_endpoint[]    = "a20xh0y90cepw1-ats.iot.us-west-2.amazonaws.com";
char aws_key[]         = "AKIAWGI4NWUYIWYF3ZWX";
char aws_secret[]      = "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";
char aws_region[]      = "us-west-2";
int port = 443;

//MQTT TOPIC
char TOPIC_NAME_PUBLISH_UPDATE[]= "$aws/things/LIGHT/shadow/update";  
char TOPIC_NAME_PUBLISH_GET[] = "$aws/things/LIGHT/shadow/get";
char TOPIC_NAME_SUBSCRIBER_DELTA[]= "$aws/things/LIGHT/shadow/update/delta"; //dựa vào topic này để thay đổi
char TOPIC_NAME_SUBSCRIBER_GET[]  = "$aws/things/LIGHT/shadow/get/accepted";
char rcvdPayload[512];  //chuỗi nhận về
char payload[512];   //chuỗi gửi đi

//enum
enum TypeOfTopic {
  DELTA,
  GET,
  EMPTY
};

TypeOfTopic TOPIC = EMPTY;

//Prototype
void Process_Connected();
void Process_Update_Desired();
void Process_Update_Last_State();


//MQTT config
const int maxMQTTpackageSize = 512;
const int maxMQTTMessageHandlers = 1;

ESP8266WiFiMulti WiFiMulti;

AWSWebSocketClient awsWSclient(1000);

PubSubClient client(awsWSclient);

//# of connections
long connection = 0;

//generate random mqtt clientID
char* generateClientID () {
  char* cID = new char[23]();
  for (int i=0; i<22; i+=1)
    cID[i]=(char)random(1, 256);
  return cID;
}

//count messages arrived
int arrivedcount = 0;

//callback to handle mqtt messages
void callback(char* topic, byte* payload, unsigned int length) {
  /*Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    rcvdPayload[i] = (char)payload[i];
  }
  Serial.println(rcvdPayload);*/
  Serial.println("CALLBACK");
  if ((String)topic == (String)TOPIC_NAME_SUBSCRIBER_DELTA)
    TOPIC = DELTA;
  else if ((String)topic == (String)TOPIC_NAME_SUBSCRIBER_GET)
    TOPIC = GET;
}

//connects to websocket layer and mqtt layer
bool connect () {
    if (client.connected()) {    
        client.disconnect ();
    }  
    //delay is not necessary... it just help us to get a "trustful" heap space value
    delay (1000);
    Serial.print (millis ());
    Serial.print (" - conn: ");
    Serial.print (++connection);
    Serial.print (" - (");
    Serial.print (ESP.getFreeHeap ());
    Serial.println (")");


    //creating random client id
    char* clientID = generateClientID ();
    
    client.setServer(aws_endpoint, port);

    const char willTopic[] = "my/things/LIGHT/update";
    int willQoS = 0;
    bool willRetain = false; 
    const char willMessage[] = "{"
        "\"state\": {"
        "\"reported\":{"
        "\"Connected\":\"false\""
        "}}}";
    if (client.connect(clientID,willTopic, willQoS, willRetain, willMessage)) {
      Serial.println("Connected to AWS");     
      return true;
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      return false;
    }
    
}

//subscribe to a mqtt topic
void subscribe () {
    client.setCallback(callback);
    delay(5000);
    client.subscribe(TOPIC_NAME_SUBSCRIBER_DELTA);
    client.subscribe(TOPIC_NAME_SUBSCRIBER_GET);
    delay(5000);
   //subscript to a topic
    Serial.println("MQTT subscribed");
}

//send a message to a mqtt topic
void sendmessage () {
    //send a message   
    //char buf[100];
    //strcpy(buf, "{\"state\":{\"reported\":{\"on\": false}, \"desired\":{\"on\": false}}}");   
    //int rc = client.publish(aws_topic, buf); 
}


void setup() {
    wifi_set_sleep_type(NONE_SLEEP_T);
    Serial.begin (9600);
    delay (2000);
    Serial.setDebugOutput(1);
    pinMode(LED_BUILTIN, OUTPUT);
    //fill with ssid and wifi password
    WiFiMulti.addAP(wifi_ssid, wifi_password);
    Serial.println ("connecting to wifi");
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
        Serial.print (".");
    }
    Serial.println ("\nConnected");

    //fill AWS parameters    
    awsWSclient.setAWSRegion(aws_region);
    awsWSclient.setAWSDomain(aws_endpoint);
    awsWSclient.setAWSKeyID(aws_key);
    awsWSclient.setAWSSecretKey(aws_secret);
    awsWSclient.setUseSSL(true);
    
    if (connect()){
      subscribe();
      Process_Connected();
    }

}

void loop() {
  //keep the mqtt up and running
  if (awsWSclient.connected()) {    
      client.loop();
      if (TOPIC == DELTA)
      {
         Serial.println("TOPIC DELTA");
         Process_Update_Desired();
         TOPIC = EMPTY;
      }
      else if (TOPIC == GET)
      {
         Serial.println("TOPIC GET");
         Process_Update_Last_State();
         TOPIC = EMPTY;
      }
  } else {
    //handle reconnection
    if (connect()){
         subscribe(); 
    }
  }
}

void Process_Connected()
{ 
    //Thông báo đã kết nối
    const char* Json = "{"
        "\"state\": {"
        "\"reported\":{"
        "\"Connected\":\"true\""
        "}}}";
    strcpy(payload,Json);
    if (client.publish(TOPIC_NAME_PUBLISH_UPDATE, payload) != 0) 
    {
       Serial.print("Publish Message: ");
       Serial.println(payload);
    } else {
       Serial.println("Publish failed");
    }

    //Cập nhập giá trị lần cuối cùng trên AWS THING SHADOW
    if (client.publish(TOPIC_NAME_PUBLISH_GET, "null") != 0)  //chua chac 
    {
       Serial.print("Publish Message: ");
       Serial.println("null");
    } else {
       Serial.println("Publish failed");
    }
}

void Process_Update_Last_State()
{     
    const size_t capacity = 1000;
    DynamicJsonBuffer jsonBuffer(capacity);

    JsonObject& root = jsonBuffer.parseObject(rcvdPayload);

    String state_reported_Light = root["state"]["reported"]["Light"]; // "on"

    long metadata_reported_Light_timestamp = root["metadata"]["reported"]["Light"]["timestamp"]; // 1563337957
    int version = root["version"]; // 762
    long timestamp = root["timestamp"]; // 1563414402

    if (state_reported_Light == "on")
       digitalWrite(LED_BUILTIN,HIGH);
    else if (state_reported_Light == "off")
       digitalWrite(LED_BUILTIN,LOW);  
}

void Process_Update_Desired()  //Cập nhập trạng thái trong trường reported đồng thời cập nhập gói tin "null" trong desired
{
    const size_t capacity = 1000;
    DynamicJsonBuffer jsonBuffer(capacity);

    JsonObject& root = jsonBuffer.parseObject(rcvdPayload);

    int version = root["version"]; // 7
    long timestamp = root["timestamp"]; // 1548419142

    String state_Light = root["state"]["Light"]; // "off"

    long metadata_Light_timestamp = root["metadata"]["Light"]["timestamp"]; // 1548419142
    //Nếu thấy có sự khác nhau giữa trường reported và desired thì xử lý
    if (state_Light != NULL)
    {
       String Json = "{"
       "\"state\": {"
       "\"reported\":{"
       "\"Light\": \""+ state_Light +"\""
       "}, \"desired\":null" 
       "}}";
       strcpy(payload,Json.c_str());
       if (client.publish(TOPIC_NAME_PUBLISH_UPDATE, payload) != 0)
       {
          if (state_Light == "on")
            digitalWrite(LED_BUILTIN,HIGH);
          else
            digitalWrite(LED_BUILTIN,LOW); 
          Serial.print("Publish Message: ");
          Serial.println(payload);
          //Serial.println(state_Light);
       }
       else
       {
          Serial.println("Publish failed");
       }
    }
}

/*void Process_Update_ESP32(String state_Light)
{
  msgUpdated = 0;
  String Json = "{"
        "\"state\": {"
        "\"reported\":{"
        "\"Light\": \""+ state_Light + "\"" 
        "}}}";
  sprintf(payload,Json.c_str());
  if(hornbill.publish(TOPIC_NAME_PUBLISH_UPDATE,payload) == 0)
  {
      if (state_Light == "on")
         digitalWrite(2,HIGH);
      else
         digitalWrite(2,LOW); 
      Serial.print("Publish Message: ");
      Serial.println(payload);
      Serial.println(state_Light);
  }
  else
      Serial.println("Publish failed");
}*/
