/*
  WiFiEsp example: WebClientRepeating

  This sketch connects to a web server and makes an HTTP request
  using an Arduino ESP8266 module.
  It repeats the HTTP call each 10 seconds.

  For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html
*/

#include "WiFiEsp.h"
#include "Secrets.h"
// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(2, 3); // RX, TX
#endif

int green = 12;
int amber = 11;
int red = 10;
int echoPin = 5;
int trigPin = 6;
int relayPin = 7;
int colors[] = {green, red, amber};
char ssid[] = SECRET_SSID;          // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
long duration;
int distance;
int availCount = 0;
bool toggle = false;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds

void setLED(int color) {
  for (int i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {

    digitalWrite(colors[i], colors[i] == color ? HIGH : LOW);

  }
}
// Set web server port number to 80
WiFiEspServer server(80);
String header;



void setup()
{

  // initialize serial for debugging
  Serial.begin(115200);
  Serial1.begin(9600);
  WiFi.init(&Serial1);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode (green, OUTPUT);
  pinMode (amber, OUTPUT);
  pinMode (red, OUTPUT);
  pinMode(relayPin, OUTPUT);
  setLED(amber);


  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    setLED(red);
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
    setLED(amber);
  }
  setLED(green);

  Serial.println("You're connected to the network");
  server.begin();
  Serial.println("serving");
  //printWifiStatus();
}

void loop() {

  bool isClosed = getIsClosed();

  WiFiEspClient client = server.available();   // Listen for incoming clients
  //Serial.println(client);
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();

      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        availCount++;
        header += c;

        if (c == '\n') {                    // if the byte is a newline character
          int ind = currentLine.indexOf("toggle");
          if(ind != -1) {
            toggle = true;
            digitalWrite(relayPin, HIGH);
            delay(1000);
            digitalWrite(relayPin, LOW);
            Serial.println("toggling");
            Serial.println(currentLine.indexOf("toggle"));
            Serial.println(currentLine);
          }
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:application/json");
            client.println("Connection: close");
            client.println();
            isClosed = getIsClosed();
            client.print("{ garageStatus: {");
            client.print(isClosed);
            client.print("}}");
            client.println();
            // Break out of the while loop
            break;
          } else {
            currentLine = "";
          }
        } else if(c != '\r'){
          currentLine += c;
        }
      }
    }
    // Clear the header variable
    header = "";
    toggle = false;
    availCount = 0;
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");

  }

}


bool getIsClosed() {
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
  return (distance < 40 && distance > 0);
}
