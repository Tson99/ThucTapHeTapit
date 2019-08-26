#include <Client.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>




///////////////////////////////

// Initialize WiFi and MQTT for this board
Client *netClient;
CloudIoTCoreDevice *device;
CloudIoTCoreMqtt *mqtt;
MQTTClient *mqttClient;
unsigned long iss = 0;
String jwt;

///////////////////////////////
// Helpers specific to this board
///////////////////////////////

const char *ssid = "IoT-Research";
const char *password = "Tapit168";

// Cloud iot details.
const char *project_id = "dotuanson";
const char *location = "asia-east1";
const char *registry_id = "my-registry";
const char *device_id = "ESP32";

// Configuration for NTP
const char* ntp_primary = "pool.ntp.org";
const char* ntp_secondary = "time.nist.gov";

// To get the private key run (where private-key.pem is the ec private key
// used to create the certificate uploaded to google cloud iot):
// openssl ec -in <private-key.pem> -noout -text
// and copy priv: part.
// The key length should be exactly the same as the key length bellow (32 pairs
// of hex digits). If it's bigger and it starts with "00:" delete the "00:". If
// it's smaller add "00:" to the start. If it's too big or too small something
// is probably wrong with your key.
const char *private_key_str =
    "44:05:8a:87:79:6d:f4:52:53:ba:4e:ca:c9:2a:6f:"
    "10:05:a4:18:50:31:af:b4:85:81:c5:c9:c1:e4:33:"
    "86:3b";

// Time (seconds) to expire token += 20 minutes for drift
const int jwt_exp_secs = 3600; // Maximum 24H (3600*24)

// To get the certificate for your region run:
// openssl s_client -showcerts -connect mqtt.googleapis.com:8883
// Copy the certificate (all lines between and including ---BEGIN CERTIFICATE---
// and --END CERTIFICATE--) to root.cert and put here on the root_cert variable.

const char *root_cert =
"-----BEGIN CERTIFICATE-----"
"MIIDfDCCAmSgAwIBAgIJAJB2iRjpM5OgMA0GCSqGSIb3DQEBCwUAME4xMTAvBgNV"
"BAsMKE5vIFNOSSBwcm92aWRlZDsgcGxlYXNlIGZpeCB5b3VyIGNsaWVudC4xGTAX"
"BgNVBAMTEGludmFsaWQyLmludmFsaWQwHhcNMTUwMTAxMDAwMDAwWhcNMzAwMTAx"
"MDAwMDAwWjBOMTEwLwYDVQQLDChObyBTTkkgcHJvdmlkZWQ7IHBsZWFzZSBmaXgg"
"eW91ciBjbGllbnQuMRkwFwYDVQQDExBpbnZhbGlkMi5pbnZhbGlkMIIBIjANBgkq"
"hkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzWJP5cMThJgMBeTvRKKl7N6ZcZAbKDVA"
"tNBNnRhIgSitXxCzKtt9rp2RHkLn76oZjdNO25EPp+QgMiWU/rkkB00Y18Oahw5f"
"i8s+K9dRv6i+gSOiv2jlIeW/S0hOswUUDH0JXFkEPKILzpl5ML7wdp5kt93vHxa7"
"HswOtAxEz2WtxMdezm/3CgO3sls20wl3W03iI+kCt7HyvhGy2aRPLhJfeABpQr0U"
"ku3q6mtomy2cgFawekN/X/aH8KknX799MPcuWutM2q88mtUEBsuZmy2nsjK9J7/y"
"hhCRDzOV/yY8c5+l/u/rWuwwkZ2lgzGp4xBBfhXdr6+m9kmwWCUm9QIDAQABo10w"
"WzAOBgNVHQ8BAf8EBAMCAqQwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMC"
"MA8GA1UdEwEB/wQFMAMBAf8wGQYDVR0OBBIEELsPOJZvPr5PK0bQQWrUrLUwDQYJ"
"KoZIhvcNAQELBQADggEBALnZ4lRc9WHtafO4Y+0DWp4qgSdaGygzS/wtcRP+S2V+"
"HFOCeYDmeZ9qs0WpNlrtyeBKzBH8hOt9y8aUbZBw2M1F2Mi23Q+dhAEUfQCOKbIT"
"tunBuVfDTTbAHUuNl/eyr78v8Egi133z7zVgydVG1KA0AOSCB+B65glbpx+xMCpg"
"ZLux9THydwg3tPo/LfYbRCof+Mb8I3ZCY9O6FfZGjuxJn+0ux3SDora3NX/FmJ+i"
"kTCTsMtIFWhH3hoyYAamOOuITpPZHD7yP0lfbuncGDEqAQu2YWbYxRixfq2VSxgv"
"gWbFcmkgBLYpE8iDWT3Kdluo1+6PHaDaLg2SacOY6Go="
"-----END CERTIFICATE-----";


// In case we ever need extra topics
const int ex_num_topics = 0;
const char* ex_topics[ex_num_topics];
//const int ex_num_topics = 1;
//const char* ex_topics[ex_num_topics] = {
//  "/devices/my-device/tbd/#"
//};    


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  pinMode(0, INPUT_PULLUP);
  setupCloudIoT();
}

unsigned long lastMillis = 0;

void loop() {
  mqttClient->loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!mqttClient->connected()) {
    connect();
  }

  // TODO: replace with your code
  // publish a message roughly every second.
  Button();
  /*if (millis() - lastMillis > 6000) {
    lastMillis = millis();
    //publishTelemetry(mqttClient, "/sensors", getDefaultSensor());
    //publishTelemetry(getDefaultSensor());
    publishTelemetry((String)digitalRead(2));
  }*/
}

void Button(){
  if (digitalRead(0) == 0)
  { 
     delay(50);
     while (digitalRead(0) == 0);
     delay(50);
     if (digitalRead(2) == HIGH)
     {
        digitalWrite(2,LOW);
        publishTelemetry("Tat den");
     }
     else
     {
        digitalWrite(2,HIGH);
        publishTelemetry("Bat den");
     }  
  }
}

void setupWifi() {
  Serial.println("Starting wifi");

  WiFi.mode(WIFI_STA);
  // WiFi.setSleep(false); // May help with disconnect? Seems to have been removed from WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }

  configTime(0, 0, ntp_primary, ntp_secondary);
  Serial.println("Waiting on time sync...");
  while (time(nullptr) < 1510644967) {
    delay(10);
  }
}

void connectWifi() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
}

void publishTelemetry(String data) {
  mqtt->publishTelemetry(data);
}

void publishTelemetry(const char* data, int length) {
  mqtt->publishTelemetry(data, length);
}

void publishTelemetry(String subfolder, String data) {
  mqtt->publishTelemetry(subfolder, data);
}

void publishTelemetry(String subfolder, const char* data, int length) {
  mqtt->publishTelemetry(subfolder, data, length);
}

void connect() {
  connectWifi();
  mqtt->mqttConnect();
}


String getJwt() {
  iss = time(nullptr);
  Serial.println("Refreshing JWT");
  jwt = device->createJWT(iss, jwt_exp_secs);
  return jwt;
}


void setupCloudIoT() {
  device = new CloudIoTCoreDevice(
      project_id, location, registry_id, device_id,
      private_key_str);
      
  setupWifi();
  netClient = new WiFiClientSecure();
  mqttClient = new MQTTClient(512);
  mqttClient->setOptions(180, true, 1000); // keepAlive, cleanSession, timeout
  mqtt = new CloudIoTCoreMqtt(mqttClient, netClient, device);
  mqtt->startMQTT();
}



// !!REPLACEME!!
// The MQTT callback function for commands and configuration updates


// Place your message handler code here.
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  if (payload == "1" || payload == "ON")
    digitalWrite(2,HIGH);
  else
    if (payload == "0" || payload == "OFF")
      digitalWrite(2,LOW);
}
