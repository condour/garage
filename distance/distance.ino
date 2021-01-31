/*
  Ultrasonic Sensor HC-SR04 and Arduino Tutorial

  by Dejan Nedelkovski,
  www.HowToMechatronics.com

*/

// Import required libraries

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
const int trigPin = 32;
const int echoPin = 33
;
const int closedPin = 12;
const int networkPin = 14;
const int relayPin = 27;

const int THRESHOLD = 40;
const int SAMPLE_LENGTH = 20;

int readings[SAMPLE_LENGTH];
int readIndex = 0;
int average = 0;
int total = 0;
// defines variables
long duration;
int distance;
int connectCount = 0;
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
  pinMode(closedPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(networkPin, OUTPUT);
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
  digitalWrite(networkPin, HIGH);
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
    digitalWrite(relayPin, HIGH);
    delay(1000);
    digitalWrite(relayPin, LOW);
    request->send(200, "application/json", "{ \"status\": \"ok\" }");
  });
  digitalWrite(networkPin, HIGH);
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
    digitalWrite(closedPin, HIGH);
  }
  if (average > THRESHOLD) {
    current_status = "Closed";
    digitalWrite(closedPin, LOW);
  }
  delay(5);
}
