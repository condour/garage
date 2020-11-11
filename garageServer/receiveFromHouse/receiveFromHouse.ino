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
int colors[] = {green, red, amber};
char ssid[] = SECRET_SSID;          // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "condourama.com";

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10000L; // delay between updates, in milliseconds

// Initialize the Ethernet client object
WiFiEspClient client;
WiFiEspServer server(80);

void setLED(int color) {
  for(int i = 0; i < sizeof(colors)/sizeof(colors[0]); i++) {
    
      digitalWrite(colors[i], colors[i] == color ? HIGH : LOW); 
    
  }
}

void setup()
{
  
  // initialize serial for debugging
  Serial.begin(115200);
  // initialize serial for ESP module
  Serial1.begin(9600);

   pinMode (green, OUTPUT);
   pinMode (amber, OUTPUT);
   pinMode (red, OUTPUT);
  setLED(amber);
  // initialize ESP module
  WiFi.init(&Serial1);

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
  
  printWifiStatus();
}

void loop()
{
  // if there's incoming data from the net connection send it out the serial port
  // this is for debugging purposes only
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if 10 seconds have passed since your last connection,
  // then connect again and send data
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }
}

// this method makes a HTTP connection to the server
void httpRequest()
{
  Serial.println();
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("Connecting...");
    
    // send the HTTP PUT request
    client.println(F("GET / HTTP/1.1"));
    client.println(F("Host: arduino.cc"));
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made
    lastConnectionTime = millis();
    setLED(green);
  }
  else {
    setLED(red);
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}


void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
