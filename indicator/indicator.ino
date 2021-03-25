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
unsigned long timerDelay = 10000;
time_t now;
char strftime_buf[64];
struct tm timeinfo;
// Replace with your network credentials
const char* ssid = "condourama";
const char* checkEndpoint = "http://192.168.1.66/";
const char* toggleEndpoint = "http://192.168.1.66/toggle";
const int openPin = 33;
const int closedPin = 12;
const int networkPin = 23;
int connectCount = 0;
String fetchedResult;
struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
  uint32_t lastTime;
};

TaskHandle_t Task1;

Button button1 = {18, 0, false, millis()};
//
//void IRAM_ATTR isr() {
//  Serial.println("IN INTERRUPT");
//  button1.numberKeyPresses += 1;
//  button1.lastTime = millis();
//  button1.pressed = true;
//}
void Task1code( void * parameter) {
  for(;;) {
    if(digitalRead(button1.PIN) == LOW){
      button1.numberKeyPresses++;
      if(button1.numberKeyPresses > 3) {
        button1.pressed = true;
      }
    } else {
      if(button1.numberKeyPresses > 0) {
        button1.numberKeyPresses--;
      }
    }
    delay(100);
  }
}

void setup() {
    xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    1); 
  Serial.begin(115200);
  pinMode(button1.PIN, INPUT_PULLUP);
  pinMode(openPin, OUTPUT);
  pinMode(closedPin, OUTPUT);
  Serial.println(WiFi.macAddress());

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(networkPin, HIGH);
    Serial.println("should be toggling HIGH");
    delay(250);
    connectCount++;
    digitalWrite(networkPin, LOW);
        Serial.println("should be toggling LOW");
    delay(250);
    Serial.println("not yet...");
    Serial.println(WiFi.status());
    if (connectCount > 10) {
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

  if (httpResponseCode > 0) {
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

  //Send an HTTP POST request every 5 sec
  if ((millis() - lastTime) > timerDelay) {
    if (button1.pressed) {
      
    
      Serial.printf("Button 1 has been pressed %u times\n", button1.numberKeyPresses);
//      Serial.println(timeClient.getFormattedTime());
      Serial.println("Issuing toggle");
    
      httpGETRequest(toggleEndpoint);
      button1.pressed = false;
      button1.numberKeyPresses = 0;
    
    }
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {

      // Your Domain name with URL path or IP address with path
      fetchedResult = httpGETRequest(checkEndpoint);
      // Send HTTP GET request
      JSONVar resultJSON = JSON.parse(fetchedResult);
      if (JSON.typeof(resultJSON) == "undefined") {
        Serial.println("PARSE FAILURE");
        return;
      }
      if ((resultJSON["status"] == (JSONVar) "Closed")) {
        digitalWrite(closedPin, HIGH);
        digitalWrite(openPin, LOW);
      } else {
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
