#include <Arduino.h>
#include <WiFi.h>
//#include <AsyncUDP.h>
//#include <ESPAsyncWebServer.h> //drop AsyncWebServer for manual client handling only
#include <Arduino_JSON.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AHTX0.h>

#include "arduino_secrets.h" //defined SSID/password in this library
//////////////////////////////////
// Variables for my network and wifi status
const char SSID[] = "T8-Arduino";
const char PASSWORD[] = "T8-Arduino";
///////////////////////////////////

// Create AsyncWebServer object on port 80
//AsyncWebServer server(80);
// Create event source on /events
//AsyncEventSource events("/events");
// Create a separate WiFi server for manual socket handling


WiFiServer manualServer(80); // Changed from server.available()

// json variable to hold sensor readings
JSONVar readings;

// timer variables 
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;
// Routing the LEDPIN to the onboard LED on ESP32=
// create a sensor object
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp; //BMP280 connect to ESP =32 I2C (GPIO 21 = SDA, GPIO)

const byte LEDPIN = LED_BUILTIN; 
String ledState;

//---------------------------------------------
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
  Serial.println("Connected to ");
  Serial.println(SSID);
  Serial.print("Use http://");
  Serial.println(WiFi.localIP());
}

// Return the state of the physical LED when called on
// change
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
  Serial.begin(115200);
  while (!Serial)
  {
    delay(5000); // to let serial catch up
  }

  initWifi();
  //server.begin();
  manualServer.begin(); // Start the manual socket server
  initBMP();
  pinMode(LEDPIN, OUTPUT);
  delay(5000);
}

// loop for manual client handling
void loop()
{ //check if anyone connects to the ESP32
  WiFiClient client = manualServer.available(); // Changed to match correct server
 // Serial.println(client);
  if (client)
  {
    Serial.println("new client");
    //varialbe to hold any incoming data from the browser/client
    String currentLine = "";
    
    //while they are connected
    while (client.connected())
    {
      if (client.available())
      {
        // another variable to hold any incoming data from browser/client
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
            client.println("Refresh: 5"); //auto-refresh page every 5 seconds
            client.println();
            client.println("<!DOCTYPE HTML>"); 
            //HERE
            client.println("<html>"); //html structure
            client.println("<head>"); //html structure
            //<head contains title and styles
            client.println("<meta charset='UTF-8'>");
            client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
            client.println("<title>PC AMBIENT MONITOR</title>");
            
            client.println("<style>");

            client.println("body {background-color:#1e1e2f; font-family: Arial; color: white; display: flex; flex-direction: column; align-items: center; padding: 20px; }");
            client.println(".box { background-color: #2a2a40; padding: 20px border-radius: 12px; box-shadow: 0 4px 10px rgba(0,0,0,0.3); text-align center; width: 300px; }");
            client.println(".reading { font-size: 28px; margin: 10px 0; }");

            client.println("</style>");
            client.println("</head>"); //html structure
            
            client.println("<body>"); //html structure
            
            client.println("<div> class ='navbar'><h1>PC Ambient Monitor</h1></div>");
            client.println(".navbar {width: 100%: background-color: #3a3a55}; padding: 10px 0; text-align: center; box-shadow: 0 2px 6px rgba(0,0,0,0.3); }");
            client.println(".navbar h1 {margin:0; font-size: 24px: color: #ffffff; }");
            
            client.println("<div class= 'box'>");
            client.println("<h2>Enviroment</h2>");
            
    
            float temp = bmp.readTemperature();
            float pres = bmp.readPressure() / 100.0F;
            client.printf("<div class='reading'>BMP280: %.2f &deg;C</div>", temp);
            client.printf("<div class='reading'>Pressure: %.1f hPa</div>", pres);
            client.println("</div>");   // close .box
            
            
            client.println("</body>");
            client.println("</html>");
            
            //HERE
           
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
      }
    }
    client.stop();
    Serial.println("client disconnected");
  }
}
 
