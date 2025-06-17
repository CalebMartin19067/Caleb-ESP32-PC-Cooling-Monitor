#include <Arduino.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Arduino_JSON.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AHTX0.h>

#include "arduino_secrets.h" //defined SSID/password in this library
//////////////////////////////////
// Variables for my network and wifi status
#define SSID "T8-Arduino"
#define PASSWORD "T8-Arduino"
///////////////////////////////////

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create event source on /events
AsyncEventSource events("/events");
// Create a separate WiFi server for manual socket handling
WiFiServer manualServer(81); // Changed from server.available()

// json variable to hold sensor readings
JSONVar readings;

// timer variables 
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;
// Routing the LEDPIN to the onboard LED on ESP32
// create a sensor object
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp; //BMP280 connect to ESP =32 I2C (GPIO 21 = SDA, GPIO)

const byte LEDPIN = LED_BUILTIN; 
const byte SENSORPIN = SENSORPIN;

String ledState;

//---------------------------------------------
// Intialize LittleFS
void initBMP()
{
  if (!bmp.begin(0x77)) // what does 0x77 signal? - the I2C address of the BMP280 sensor.
  {
    Serial.println("Couldn't find the BMP280 sensor, check wiring!");
    while (1); // what does this while loop do? and why would we do it?
  }
}

// Retrieve Sensor readings and return JSON object
String getSensorReadings()
{
  readings["temperature"] = String(bmp.readTemperature() / 2);
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

// LittleFS has began it will be true,
// if not (!) it will return an error
void initLittleFS()
{
  if (!LittleFS.begin(true))
  {
    Serial.println("An error has occured while mounting LittleFS");
  }
  else
  {
    Serial.println("LittleFS mounted successfully");
  }
}

// initialize Wifi
void initWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting to Wifi....");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("Connect to ");
  Serial.println(SSID);
  Serial.print("Use http://");
  Serial.println(WiFi.localIP());
}

// Return the state of the physical LED when called on
String processor(const String &VAR)
{
  if (VAR == "STATE")
  {
    if (digitalRead(LEDPIN))
    {
      ledState = "ON";
    }
    else
    {
      ledState = "OFF";
    }
    return ledState;
  }
  return String();
}

// run start up code and server requests
void setup()
{
  pinMode(LEDPIN, OUTPUT);
  pinMode(SENSORPIN, INPUT);

  Serial.begin(115200);
  while (!Serial)
  {
    delay(5000); // to let serial catch up
  }

  initWifi();

  //initLittleFS();
  server.begin();
  manualServer.begin(); // Start the manual socket server

  initBMP();

  pinMode(LEDPIN, OUTPUT);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html", false, processor); });

  server.serveStatic("/", LittleFS, "/");

  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              digitalWrite(LEDPIN, HIGH);
              request->send(LittleFS, "/index.html", "text/html", false, processor);
            });

  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String json = getSensorReadings();
              request->send(200, "application/json", json);
              json = String();
            });

  events.onConnect([](AsyncEventSourceClient *client)
                   {
                     if (client->lastId())
                     {
                       Serial.printf("Client reconnected: Last message ID that it got is: %u\n", client->lastId());
                     }
                     client->send("hello!", NULL, millis(), 10000);
                   });
  server.addHandler(&events);

  server.begin();
}

// loop for manual client handling
void loop()
{
  WiFiClient client = manualServer.available(); // Changed to match correct server
  if (client)
  {
    Serial.println("new client");

    String currentLine = "";

    while (client.connected())
    {
      if (client.available())
      {
        // another variable to hold any incoming
        char c = client.read();
        Serial.write(c);

        ///////////////////////////////////////
        // if browser sent a new line character
        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {
            // HTTP code for a webpage after some initial search
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println("Refresh: 5");
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<style>html{font-family: Arial, H }</style>");

            client.println("<h1>Sensor stuff</h1>");

            int sensorReading = analogRead(SENSORPIN);
            client.print("RAW Sensor value is ");
            client.print(sensorReading);

            byte LEDReading = digitalRead(LEDPIN);
            if (LEDReading == HIGH)
            {
              client.print("Red led is on<br><br>");
            }
            else
            {
              client.print("Red LED is off<br><br>");
            }

            client.print("Click <a href=\"H\">here</a> turn the LED on<br>");
            client.print("Click <a href=\"L\">here</a> turn the LED off<br>");

            client.println("</html>");
            break;
          }
          else
          {
            currentLine = "";
          }
        }
        else if (c != '\r')
        {
          currentLine += c;
        }

        if (currentLine.endsWith("GET /H"))
        {
          digitalWrite(LEDPIN, HIGH); // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L"))
        {
          digitalWrite(LEDPIN, LOW); // GET /L turns the LED off
        }
      }
    }
    client.stop();
    Serial.println("client disconnected");
  }
}
 
