//https://awsm.dvlprz.com/2017-10-16-esp32-aws-iot-mqtt
//Không nhận được tin nhắn callback từ hệ thống aws gửi về (Fix: Tang maxsize packet trong thu vien)
//#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <AWS_IOT.h>

void connectAWSIoT();
void mqttCallback (char* topic, byte* payload, unsigned int length);

char *ssid = "IoT-Research";
char *password = "Tapit168";

const char *endpoint = "a20xh0y90cepw1-ats.iot.us-west-2.amazonaws.com";

const int port = 8883;

//MQTT TOPIC
char TOPIC_NAME_PUBLISH_UPDATE[] = "$aws/things/LIGHT/shadow/update";
char TOPIC_NAME_PUBLISH_GET[] = "$aws/things/LIGHT/shadow/get";
char TOPIC_NAME_SUBSCRIBER_DELTA[] = "$aws/things/LIGHT/shadow/update/delta"; //dựa vào topic này để thay đổi
char TOPIC_NAME_SUBSCRIBER_GET[]  = "$aws/things/LIGHT/shadow/get/accepted";
char rcvdPayload[512];  //chuỗi nhận về
char payload[512];   //chuỗi gửi đi
volatile int msgUpdated = 0;

//enum
enum TypeOfTopic {
  DELTA,
  GET,
  EMPTY
};

TypeOfTopic TOPIC = EMPTY;

const char* rootCA = \
                     "-----BEGIN CERTIFICATE-----\n"
                     "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
                     "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
                     "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
                     "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
                     "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
                     "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
                     "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
                     "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
                     "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
                     "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
                     "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
                     "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
                     "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
                     "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
                     "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
                     "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
                     "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
                     "rqXRfboQnoZsG4q5WTP468SQvvG5\n"
                     "-----END CERTIFICATE-----\n";

const char* certificate = \
                          "------BEGIN CERTIFICATE-----\n"
                          "MIIDWTCCAkGgAwIBAgIUXOsEJXdf6vGVTxCYAZDV4aAKM58wDQYJKoZIhvcNAQEL\n"
                          "BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"
                          "SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE5MDcxMTAzMzk1\n"
                          "OFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"
                          "ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALs/AZQEjEX1pzIe1kjJ\n"
                          "UvOFTINzXtsLoCPbO7q/T7P7tdrPHEAcOyx0QnCVa+MnYgo+/KrMHjYLNQiJygCk\n"
                          "sTSMeHjk+6b6SNhlifR47dkAzvBsHQKvlbPzGy+EZ1mseuTNHizUpFpxmup3O+yQ\n"
                          "HEH/nIf3+GWm3SL0J8ueAmR7oz5gZ6TChuHKo2yrdbW4ZZWHosrZ/O3JC6EfGVAn\n"
                          "nZyrsUchky674VRRIpaC1J62V1M4ugLQ74852vW8J7Yi2+HcUzW47xx/N3JGOy5a\n"
                          "I8CnIJsyg9gT7pkLKj0KjZb9I36EEBq57EiVc/0CG1fIxVGG3L7MXZrmr+sucktn\n"
                          "TCECAwEAAaNgMF4wHwYDVR0jBBgwFoAUfUSD4nuoc4iHyyIBTJRw/Fo/2eUwHQYD\n"
                          "VR0OBBYEFOidrUuAXFCy1AcV1PGKatWBgRBPMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"
                          "AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBVyQaNKRqL7D/wAHaGC9vn7gAB\n"
                          "dBmpQKaGJVOI59nwaxCUTAgNDofr+1tldHCgDeAZmcCmxLVfd10L2w406q6IZmwT\n"
                          "DFnwZuN1Qib9nQFT1rzRxECZHS+4vcIkSISksbKz0J3tvdekMz6/oR5VGp9/itYx\n"
                          "qGMC1w/RoEV34Y/hz7UbOc9rtSrugln8zvkG0i9oGlIm74RZiH0UBJa6VqRj0lvE\n"
                          "AT/8MTrJB9VgOnBgWCOCBoG/UTo6f1TxiNWO7tnLh0X+b06XQzLowrmfWZNA/IlA\n"
                          "fzoRCdARuI6VKYj5jreWZfncZtT9e2Cb11nXm+84fH1MT3zNxPMny8A4tSdh\n"
                          "-----END CERTIFICATE-----\n";

const char* privateKey = \
                         "-----BEGIN RSA PRIVATE KEY-----\n"
                         "MIIEpQIBAAKCAQEAuz8BlASMRfWnMh7WSMlS84VMg3Ne2wugI9s7ur9Ps/u12s8c\n"
                         "QBw7LHRCcJVr4ydiCj78qsweNgs1CInKAKSxNIx4eOT7pvpI2GWJ9Hjt2QDO8Gwd\n"
                         "Aq+Vs/MbL4RnWax65M0eLNSkWnGa6nc77JAcQf+ch/f4ZabdIvQny54CZHujPmBn\n"
                         "pMKG4cqjbKt1tbhllYeiytn87ckLoR8ZUCednKuxRyGTLrvhVFEiloLUnrZXUzi6\n"
                         "AtDvjzna9bwntiLb4dxTNbjvHH83ckY7LlojwKcgmzKD2BPumQsqPQqNlv0jfoQQ\n"
                         "GrnsSJVz/QIbV8jFUYbcvsxdmuav6y5yS2dMIQIDAQABAoIBAQCkcNHe/xt9uR4L\n"
                         "iz9fDsd1q+QlMSXlstEBjGBpQegqRW1Q60CBIQnqoXNT0jW8ePLF0sks6jPB/Jen\n"
                         "lvK7G+ewIYkPivFilD1zJcFA3Q7s26PhSuEzYaZUMbT3Rw1ImIwY8faFIn3wHAMT\n"
                         "1IDg4TKf127Njj0scJ2WO9vTG0I4oUZ9Y6xCWBpX46esi7mWugLS1HYg9erS73wu\n"
                         "JLwO9jbKF4gTKFDogjg7IpyX+PYMBTcZjdAnwDXELTiWzP1gEVOds52lYjCuenRi\n"
                         "BaQBejPMKEcuHIxn5RNv3PW4w1OMTO00+zxmnhzrC5H/grd/sZkpVRdJKAtJVgcx\n"
                         "/DwzSLp9AoGBAOveafAeowCgBuZLAstlvBZ4w2jJMA79KCglpyM0tfVvRtMmKsDD\n"
                         "QtFH3xlMdFl92mqThmYQu4aa2NbzDfIJgjMsghi9A3a+dkGymarTfhkJkjRR7r9f\n"
                         "d7ZdwTxeEdFFZhbWNK86rCCn5bb4ZnXoDOkgVT2nGHYJUj2WGvxpbUXLAoGBAMs6\n"
                         "N4znrYDf/GqHPRigswXNbqGJ8YGe1HGk1s2ucSoRjHHNExxE93lFfsNTStyEAIWR\n"
                         "J2lq7XKUA805sJapNDl/OGH0qOZrUgEaJit9KlikH9YxD3LgZDJWn7oZMvmsyuIb\n"
                         "7EE84TK9F9nBchf+yG47I2TuNOax9904YEnfaRhDAoGBAK5iGMa7j93VcmRvDtjc\n"
                         "FWwzyp09KXfbeQX9mI1x2hlkNOPFMxSjCFJ8JZipqtfDMWlcOqBZPJuu/iQlPKk2\n"
                         "mHlHLcQmclonRH0cE3p1tKPELMDb1nzMk1BUDpqYHeovueOmaPsokLvwBej5y57e\n"
                         "ejUdf4f+cxpJl/EKelclhvFzAoGASXFsidtf9Gb9zXG6PqA0kjA/ftlfEbzYgYlU\n"
                         "p1gXRWC6JJY8BcdWt9jJisUbFL4PgjWGzK37yaB8Dzp9+rSDTxQOyDemoj6+j+9n\n"
                         "Q+gj2YzWhI2OS7GUBC0R5CsfJOOxHi2BH8Jkj/WOHP+vv8QvosZ0cBE1xPkj3ip1\n"
                         "gbzfOgkCgYEAqAQvqpp5HIAWW1MOEBFxujfCHAjE9FhdMHGQXe/+UfJyeTcmbSmo\n"
                         "ZQDWVqArOzeGM/iBBwz7OkXdCbxuB78B8OSS/l+JNyKy3V3k5N0l1baBwTbNWvDv\n"
                         "jvBFYIP1luTMKueEsowxS3G9lqcWZiHPSWAV+JTWbOjfdopD2grJxb8=\n"
                         "-----END RSA PRIVATE KEY-----\n";

WiFiClientSecure httpsClient;
PubSubClient mqttClient(httpsClient);


void mqttCallback (char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    rcvdPayload[i] = (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println("\n");
  if ((String)topic == (String)TOPIC_NAME_SUBSCRIBER_DELTA)
    TOPIC = DELTA;
  else if ((String)topic == (String)TOPIC_NAME_SUBSCRIBER_GET)
    TOPIC = GET;
}

void setup() {
  //delay(1000);
  pinMode(0,INPUT_PULLUP);
  pinMode(2,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(0),_Mode,FALLING);
  Serial.begin(115200);
  
  // Start WiFi
  Serial.println("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to wifi.");

  // Configure MQTT Client
  httpsClient.setCACert(rootCA);
  httpsClient.setCertificate(certificate);
  httpsClient.setPrivateKey(privateKey);
  mqttClient.setServer(endpoint, port);
  mqttClient.setCallback(mqttCallback);
  
  connectAWSIoT();
}

void connectAWSIoT() {
  //Kết nối tới AWS kem theo LWT và subscribe topic
  while (!mqttClient.connected()) {
    char* clientID = "ESP32_Device";
    const char willTopic[] = "my/things/LIGHT/update";
    int willQoS = 0;
    bool willRetain = false;
    const char willMessage[] = "{"
                               "\"state\": {"
                               "\"reported\":{"
                               "\"Connected\":\"false\""
                               "}}}";
    if (mqttClient.connect(clientID, willTopic, willQoS, willRetain, willMessage)) {
      Serial.println("Connected to AWS.");
      int qos = 0;
      mqttClient.subscribe(TOPIC_NAME_SUBSCRIBER_DELTA, qos);
      mqttClient.subscribe(TOPIC_NAME_SUBSCRIBER_GET, qos);
      delay(1000);
      Serial.println("Subscribed.");
    } else {
      Serial.print("Failed. Error state=");
      Serial.print(mqttClient.state());
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

  //Thông báo đã connection và gửi gói tin lên topic get để nhận trạng thái cuối cùng trên ThingShadow
  const char* Json = "{"
                     "\"state\": {"
                     "\"reported\":{"
                     "\"Connected\":\"true\""
                     "}}}";
  strcpy(payload, Json);
  if (mqttClient.publish(TOPIC_NAME_PUBLISH_UPDATE, payload) != 0)
  {
    Serial.print("Publish Message connection: ");
    Serial.println(payload);
  } else {
    Serial.println("Publish failed");
  }

  //Publish message "null" toi topic get de nhan trang thai cuoi cung cua thiet bi tren ThingShadow
  if (mqttClient.publish(TOPIC_NAME_PUBLISH_GET, "null") != 0)  //chua chac
  {
    Serial.print("Publish Message to topic get: ");
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
    digitalWrite(2, HIGH);
  else if (state_reported_Light == "off")
    digitalWrite(2, LOW);
}

void Process_Update_Delta() {
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
                  "\"Light\": \"" + state_Light + "\""
                  "}, \"desired\":null"
                  "}}";
    strcpy(payload, Json.c_str());
    if (mqttClient.publish(TOPIC_NAME_PUBLISH_UPDATE, payload) != 0)
    {
      if (state_Light == "on")
        digitalWrite(2, HIGH);
      else
        digitalWrite(2, LOW);
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


void Process_Update_ESP32(String state_Light) //update thao tac truc tiep tren ESP32
{
  msgUpdated = 0;
  String Json = "{"
        "\"state\": {"
        "\"reported\":{"
        "\"Light\": \""+ state_Light + "\"" 
        "}}}";
  sprintf(payload,Json.c_str());
  if(mqttClient.publish(TOPIC_NAME_PUBLISH_UPDATE,payload) != 0)
  {
      if (state_Light == "on")
         digitalWrite(2,HIGH);
      else
         digitalWrite(2,LOW); 
      Serial.print("Publish Message: ");
      Serial.println(payload);
      //Serial.println(state_Light);
  }
  else
      Serial.println("Publish failed");
}


void mqttLoop() {
  if (!mqttClient.connected()) {
    connectAWSIoT();
  }
  mqttClient.loop();
  if (TOPIC == DELTA) {
    Serial.println("TOPIC DELTA");
    Process_Update_Delta();
    TOPIC = EMPTY;
  }
  else if (TOPIC == GET) {
    Serial.println("TOPIC GET");
    Process_Update_Last_State();
    TOPIC = EMPTY;
  }
}

void button(){
    if (msgUpdated == 1)
    { 
       if (digitalRead(2) == HIGH)
          Process_Update_ESP32("on");
       else
          Process_Update_ESP32("off");
    }
}

void loop() {
  mqttLoop();
  button();
}

void _Mode(){
  if (digitalRead(2) == HIGH)
    digitalWrite(2,LOW);
  else
    digitalWrite(2,HIGH);
  msgUpdated = 1;
}
