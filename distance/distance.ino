/*
  Ultrasonic Sensor HC-SR04 and Arduino Tutorial

  by Dejan Nedelkovski,
  www.HowToMechatronics.com

*/

// Import required libraries
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Credentials.h"
// Replace with your network credentials
const char* ssid = "condourama";

// Set web server port number to 80
AsyncWebServer server(80);

// Variable to store the HTTP request
String header;

// defines pins numbers
const int trigPin = 4;
const int echoPin = 2;
const int redPin = 18;
const int THRESHOLD = 40;
const int SAMPLE_LENGTH = 20;
int readings[SAMPLE_LENGTH];
int readIndex = 0;
int average = 0;
int total = 0;
// defines variables
long duration;
int distance;
bool inProximity = false;

String current_status;

const char index_html[] PROGMEM = R"rawliteral(
 { "status": "%current_status_placeholder%" }
)rawliteral";


String processor(const String& var) { 
    if(var == "current_status_placeholder") {
      return current_status;
    }
}

void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(redPin, OUTPUT);
   Serial.println(WiFi.macAddress());
  Serial.begin(115200); // Starts the serial communication
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < SAMPLE_LENGTH; thisReading++) {
    readings[thisReading] = 0;
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(WiFi.status());
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/json", index_html, processor);
  });
  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("TOGGLE");
    request->send(200, "application/json", "{ \"status\": \"ok\" }");
  });
}

void loop() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  distance = duration * 0.034 / 2;

  total = total - readings[readIndex];
  readings[readIndex] = distance;
    total = total + readings[readIndex];
  readIndex = readIndex+1;
  if(readIndex >= SAMPLE_LENGTH) {
    readIndex = 0;
  }
  average = total/SAMPLE_LENGTH;
  // Serial.println(String(total) + " " + String(average) + " " + String(distance));
 
  // Prints the distance on the Serial Monitor
  if (average < THRESHOLD) {
    current_status = "Open";
    digitalWrite(redPin, HIGH);
  }
  if (average > THRESHOLD) {
    current_status = "Closed";
    digitalWrite(redPin, LOW);
  }
  delay(5);
}
