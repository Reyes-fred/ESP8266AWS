/*Developed by Alfredo Reyes
Project: AWS | NodeMCU ESP32
AWS Iot Core
*/
#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
 // Actualiza con base a los valores de tu coneccion 
const char * ssid = "xxxxxxxxxx";
const char * password = "xxxxxxxxxxx";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//AWS este valor lo optines del portal de AWS
const char * AWS_endpoint = "xxxxxxxx-ats.iot.us-east-1.amazonaws.com"; //MQTT broker ip
void callback(char * topic, byte * payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
}

WiFiClientSecure espClient;
PubSubClient client(AWS_endpoint, 8883, callback, espClient); //MQTT port 8883 - standard
long lastMsg = 0;
char msg[50];
int value = 0;
void setup_wifi() {
  delay(10);
  // Inicializamos Wifi
  espClient.setBufferSizes(512, 512);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  espClient.setX509Time(timeClient.getEpochTime());
}
void reconnect() {
  // Reconeccion si es necesario
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESPthing")) {
      Serial.println("connected");

      client.publish("outTopic", "hello world");
      
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      char buf[256];
      espClient.getLastSSLError(buf, 256);
      Serial.print("WiFiClientSecure SSL error: ");
      Serial.println(buf);
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  // inicializamos el led del esp8266
  pinMode(LED_BUILTIN, OUTPUT);
  setup_wifi();
  delay(1000);
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  Serial.print("Heap: ");
  Serial.println(ESP.getFreeHeap());
  // Cargamos los certificados
  File cert = SPIFFS.open("/cert.der", "r"); //verifica
  if (!cert) {
    Serial.println("Failed to open cert file");
  } else
    Serial.println("Success to open cert file");
  delay(1000);
  if (espClient.loadCertificate(cert))
    Serial.println("cert loaded");
  else
    Serial.println("cert not loaded");
  // Cargamos llave privada
  File private_key = SPIFFS.open("/private.der", "r"); 
  if (!private_key) {
    Serial.println("Failed to open private cert file");
  } else
    Serial.println("Success to open private cert file");
  delay(1000);
  if (espClient.loadPrivateKey(private_key))
    Serial.println("private key loaded");
  else
    Serial.println("private key not loaded");
  // Cargamos CA 
  File ca = SPIFFS.open("/ca.der", "r"); 
  if (!ca) {
    Serial.println("Failed to open ca ");
  } else
    Serial.println("Success to open ca");
  delay(1000);
  if (espClient.loadCACert(ca))
    Serial.println("ca loaded");
  else
    Serial.println("ca failed");
  Serial.print("Heap: ");
  Serial.println(ESP.getFreeHeap());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  client.publish("outTopic", "Hello World");
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(100); // wait for a second
  digitalWrite(LED_BUILTIN, LOW);
  delay(100); // wait for a second
}
