/*
  Ultrasonic Sensor HC-SR04 and Arduino Tutorial

  by Dejan Nedelkovski,
  www.HowToMechatronics.com

*/

// Import required libraries

#include <WiFi.h>
#include <AsyncTCP.h>
#include <HTTPClient.h>
#include "Credentials.h"
#include <Arduino_JSON.h>

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

// Replace with your network credentials
const char* ssid = "condourama";
const char* checkEndpoint = "http://192.168.1.66/";
const char* toggleEndpoint = "http://192.168.1.66/toggle";
const int openPin = 13;
const int closedPin = 12;
const int networkPin = 14;
const int buttonPin = 15;
int connectCount = 0;
String fetchedResult;
struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button1 = {18, 0, false};

void IRAM_ATTR isr() {
  button1.numberKeyPresses += 1;
  button1.pressed = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr, FALLING);
  pinMode(openPin, OUTPUT);
  pinMode(closedPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  Serial.println(WiFi.macAddress());

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(networkPin, HIGH);
    delay(250);
    connectCount++;
    digitalWrite(networkPin, LOW);
    delay(250);
    Serial.println("not yet...");
    Serial.println(WiFi.status());
    if(connectCount > 10) {
      digitalWrite(networkPin, HIGH);
      WiFi.begin(ssid, password);
      connectCount = 0;
      delay(500);
      
      digitalWrite(networkPin, LOW);
    }
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(networkPin, HIGH);
  delay(500);
}

String httpGETRequest(const char* serverName) {
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
void loop() {
  
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
      if (button1.pressed) {
      Serial.printf("Button 1 has been pressed %u times\n", button1.numberKeyPresses);
      httpGETRequest(toggleEndpoint);
      button1.pressed = false;
  }
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
  
      // Your Domain name with URL path or IP address with path
      fetchedResult = httpGETRequest(checkEndpoint);
      // Send HTTP GET request
      JSONVar resultJSON = JSON.parse(fetchedResult);
      if(JSON.typeof(resultJSON) == "undefined") {
        Serial.println("PARSE FAILURE");
        return;
      }
      if((resultJSON["status"] == (JSONVar) "Closed")){
        Serial.println("CLOSED");
        digitalWrite(closedPin, HIGH);
        digitalWrite(openPin, LOW);
      } else {
        Serial.println((const char*) resultJSON["status"]);
        Serial.println("OPEN");
        digitalWrite(openPin, HIGH);
        digitalWrite(closedPin, LOW);
      }
    }
    else {
      Serial.println("WiFi Disconnected");
      ESP.restart();
    }
    lastTime = millis();
  }
}
