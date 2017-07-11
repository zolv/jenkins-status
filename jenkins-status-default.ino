
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

const char* ssid = "SSID";
const char* password = "PASSWORD";

#define trunkR D2
#define trunkY D3
#define trunkG D4

#define releaseR D5
#define releaseY D6
#define releaseG D7

const char* host = "your.host.com";
const int httpsPort = 443;
String token = "TOKEN_TOKEN_TOKEN";

String url1 =  "/job/NAME-OF-YOUR-JOB-1";
String url2 =  "/job/NAME-OF-YOUR-JOB-2";

#define SUCCESS 2
#define FAILURE 1
#define UNDEFINED 0

int trunkStatus = UNDEFINED;
int releaseStatus = UNDEFINED;
int trunkBuilding = false;
int releaseBuilding = false;

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  pinMode(trunkR, OUTPUT);
  pinMode(trunkY, OUTPUT);
  pinMode(trunkG, OUTPUT);
  pinMode(releaseR, OUTPUT);
  pinMode(releaseY, OUTPUT);
  pinMode(releaseG, OUTPUT);

  digitalWrite(trunkR, LOW);
  digitalWrite(trunkY, LOW);
  digitalWrite(trunkG, LOW);
  
  digitalWrite(releaseR, LOW);
  digitalWrite(releaseY, LOW);
  digitalWrite(releaseG, LOW);
}

void loop() {
  
  blinkYellow(trunkBuilding, trunkY);
  
  trunkStatus = lastCompleted(url1, trunkG, trunkY, trunkR);
  setResult(trunkStatus, trunkR, trunkG);

  blinkYellow(trunkBuilding, trunkY);
  
  trunkBuilding = isBuilding(url1);
  setBuilding(trunkBuilding, trunkY);


  delay(10000);

  blinkYellow(releaseBuilding, releaseY);
  
  releaseStatus = lastCompleted(url2, releaseG, releaseY, releaseR);
  setResult(releaseStatus, releaseR, releaseG);
  
  blinkYellow(releaseBuilding, releaseY);
  
  releaseBuilding = isBuilding(url2);
  setBuilding(releaseBuilding, releaseY);
  
  delay(10000);
}

boolean lastCompleted(String url, int ledG, int ledY, int ledR) {
   while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return false;
  }

  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + "/lastCompletedBuild/api/json?tree=result HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Authorization: " + token + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
    Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
  
  if (line.indexOf("\"result\":\"SUCCESS\"") >= 0 ) {
    return SUCCESS;
  } else {
  if (line.indexOf("\"result\":\"FAILURE\"") >= 0 ) {
    return FAILURE;
  } else {
    return UNDEFINED;
  }
  }

}

boolean isBuilding(String url) {
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return false;
  }
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + "/lastBuild/api/json?tree=building HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Authorization: " + token + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  boolean building;
  if (line.indexOf("\"building\":true") >= 0 ) {
    building = true;
  } else {
    building = false;
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
  return building;
}

void setResult(int result, int rPin, int gPin) {
  if(result == SUCCESS) {
    digitalWrite(rPin, LOW);
    digitalWrite(gPin, HIGH);
  } else {
    if(result == FAILURE) {
      digitalWrite(gPin, LOW);
      digitalWrite(rPin, HIGH);
    } else {
      digitalWrite(gPin, LOW);
      digitalWrite(rPin, LOW);
    }
  }
}

void setBuilding(int result, int yPin) {
   if(result) {
    digitalWrite(yPin, HIGH);
  } else {
    digitalWrite(yPin, LOW);
  }
}

void blinkYellow(int result, int yPin) {
  if(result) {
    digitalWrite(yPin, LOW);
    delay(50);
    digitalWrite(yPin, HIGH);
    delay(50);
  } else {
    digitalWrite(yPin, HIGH);
    delay(50);
    digitalWrite(yPin, LOW);
    delay(50);
  }
}


