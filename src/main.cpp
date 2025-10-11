#include <Arduino.h>
#include <WiFi.h>
#include <Arduino_JSON.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AHTX0.h> 
#include <TFT_eSPI.h>  // Use TFT_eSPI for ESP32 reverse TFT

//////////////////////////////////
//Wifi Network Configuration
//change these to match your nework
const char SSID[] = "AX3000";  
const char PASSWORD[] = "LemontIselin49BI"; 
///////////////////////////////////

WiFiServer manualServer(80); 
// Creates a simple socket server on port 80. Unlike AsyncWebServer,
// this requires manually handling all HTTP parsing.


// json variable to hold sensor readings
JSONVar readings;

// create a sensor object
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp; //BMP280 connect to ESP =32 I2C (GPIO 21 = SDA, GPIO)

const byte LEDPIN = LED_BUILTIN; //Built-in LED (onboard ESP32 LED)
String ledState;

//Set up the Fan on Pin 13
const byte FANPIN = 13; //GPIO 13 for a fan
String fanState;


//---------------------------------------------
//Initialise BMP280 sensor
void initBMP()
{
  //0x77 is the defualt I2C address for the BMP280 sensor
  // If wiring is wrong or the sensor is unconnected,.begin() will fail
  if (!bmp.begin(0x77)) 
  {
    Serial.println("Couldn't find the BMP280 sensor, check wiring!"); 
  } else {
    Serial.println("BMP280 sensor found and initialised.");
    bmp.readTemperature();
  } //no infinite loop here so program will just continue without a BMP280 sensor
}

// Initialise AHT sensor
void initAHT() {
  if (!aht.begin()) {
    Serial.println("Couldn't find the AHT sensor, check wiring!");
  } else {
    Serial.println("AHT sensor found and initialised.");
    sensors_event_t humidity, temperature;
    aht.getEvent(&humidity, &temperature); // dummy read (to help the sensor wake up first)
  }
}

// Retrieve Sensor readings and return JSON object
String getSensorReadings()
{
  readings["temperature"] = String(bmp.readTemperature()); //Needs to be divided by 2, unsure why
  readings["pressure"] = String(bmp.readPressure() / 100.0F); // BMP280 pressure
  
  //Add AHT humitidy and temperature to JSON
  sensors_event_t humidity, temperature;
  aht.getEvent(&humidity, &temperature);
  readings["aht_temp"] = String(temperature.temperature);
  readings["humidity"] = String(humidity.relative_humidity);
  
  String jsonString = JSON.stringify(readings);     
  return jsonString;
}

// initialize Wifi
void initWifi()
{
  WiFi.mode(WIFI_STA);    //Station mode, where the ESP32 connects to a router
  WiFi.begin(SSID, PASSWORD); //SSID and password that were stored as constants
  Serial.print("Connecting to Wifi...."); 
  while (WiFi.status() != WL_CONNECTED) //Loop until connected
  {
    Serial.print('.');   //every 1 second paste a "." so it shows its waiting still trying to connect
    delay(1000);
  } //Once connected print the SSID, and device IP address
  Serial.println("Connected to ");  
  Serial.println(SSID);
  Serial.print("Use http://"); //URL to paste into browser
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

//------------------------------------------------
// run start up code and server requests
void setup()
{
  pinMode(FANPIN, OUTPUT);  //Set fan pin as output
  digitalWrite(FANPIN, LOW); // start with fan off

  pinMode(LEDPIN, OUTPUT); //Set LED pin as output
  Serial.begin(115200); //Start serial monitor for debugging
  while (!Serial)
  {
    delay(5000); // to let serial catch up
  }

  initWifi(); // Connect to Wifi
  //server.begin();
  manualServer.begin(); // Start the manual socket server
  initBMP(); // Initialise BMP280
  initAHT(); // Initialise AHT sensor
  
  delay(5000); //Startup delay for reliability
}

//---------------------------------------
// loop for manual client handling
void loop()
{ //check if anyone has cconnected to the ESP32
  WiFiClient client = manualServer.available(); // Changed to match correct server
 

  // Serial.println(client);
  if (client)
  {
    Serial.println("new client");
    //varialbe to hold any incoming data from the browser/client
    String currentLine = ""; //Holds the currentline of HTTP request
      
    
    //while they are connected
    while (client.connected())
    {
      if (client.available())
      {
        // another variable to hold any incoming data from browser/client
        char c = client.read(); //Read on character at a time
        Serial.write(c); // Print raw requestr to serial for debugging

        ///////////////////////////////////////
        // if browser sent a new line character
        if (c == '\n')
         { 
          //Check if request was for toggling LED or FAN
        if (currentLine.indexOf("GET /led/toggle") >= 0) {
           digitalWrite(LEDPIN, !digitalRead(LEDPIN)); //Toggle LED state
        }
        if (currentLine.indexOf("GET /fan/toggle") >= 0) {   //Toggle fan state
        digitalWrite(FANPIN, !digitalRead(FANPIN));
        }
        //If line is empty, it means end of HTTP request header
          if (currentLine.length() == 0)
          {
            // HTTP code for a webpage after some initial search
            client.println("HTTP/1.1 200 OK");  
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println("Refresh: 5"); //auto-refresh page every 5 seconds
            client.println();
            client.println("<!DOCTYPE HTML>"); 
            
            //HtML webpage
            client.println("<html>"); //html structure
            client.println("<head>"); //html structure
            //<head contains title and styles
            client.println("<meta charset='UTF-8'>");
            client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
            client.println("<title>PC AMBIENT MONITOR</title>");
    
            //CSS styling 
            client.println("<style>"); //html structure
            client.println("body {font-family: Arial; margin:0; min-height: 100vh; display: flex; flex-direction: column; justify-content: center; align-items: center; transition: 0.3s, color 0.3s }");
            //Dark and light ui options
            client.println(".dark { background-color: #1e1e2f; color: white; }");
            client.println(".light { background-color: #f4f4f4; color: black; }");
            client.println(".navbar {width: 100%; padding: 20px; border-radius: 12px; box-shadow: 0 4px 10px rgba(0,0,0,0.3); text-align: center; width: 300px; }");
            client.println(".box { background-color: #2a2a40; color: white; padding: 20px; border-radius: 12px; box-shadow: 0 4px 10px rgba(0,0,0,0.3); text-align: center; width: 300px; }");
            client.println(".reading { font-size: 28px; margin: 10px 0; }");
            client.println("button {padding: 10px 20px; margin: 10px 0; }");
            client.println("</style>"); //html
            client.println("</head>"); //html structure
            
            // Start of HTML body (defualts to dark theme)
            client.println("<body class='dark'>");
            client.println("<div class='navbar'><h1>PC Ambient Monitor</h1></div>");
            client.println("<div class='box'>");
            
            // Display BMP280 readings
            float temp = bmp.readTemperature();
            float pres = bmp.readPressure() / 100.0F;
            client.printf("<div class='reading'>BMP280: %.2f &deg;C</div>", temp);
            client.printf("<div class='reading'>Pressure: %.1f hPa</div>", pres);

            // Display AHT readings
            sensors_event_t humidity, temperature;
            aht.getEvent(&humidity, &temperature);  // populate temp and humidity objects
            client.printf("<div class='reading'>AHT Temp: %.2f &deg;C</div>", temperature.temperature);
            client.printf("<div class='reading'>Humidity: %.1f %%</div>", humidity.relative_humidity);
            
            //Buttons for LED theme, and FAN
            client.println("<button onclick=\"toggleLED()\">Toggle LED</button>");
            client.println("<button onclick=\"toggleTheme()\">Toggle Theme</button>");
            client.println("<button onclick=\"toggleFan()\">Toggle Fan</button>");
            client.println("</div>");

            // JavaScript for button actions
            client.println("<script>"); //====SCRIPT====
            client.println("function toggleFan(){ fetch('/fan/toggle'); }");
            client.println("function toggleLED(){ fetch('/led/toggle'); }");
            client.println("function toggleTheme(){ document.body.classList.toggle('dark'); localStorage.setItem('theme', document.body.className); }");
            client.println("window.onload = function(){ let theme = localStorage.getItem('theme'); if(theme){ document.body.className = theme; } }");
            client.println("</script>"); //==SCRIPT====

            client.println("</body>");
            client.println("</html>");
            
          
           
            break;  //Exit loop after serving page
          }
          else
          {
            currentLine = ""; // Reset line buffer
          }
        }
        else if (c != '\r')
        {
          currentLine += c; // Add character to currentLine (building the HTTP request )
        }
      }
    }
    client.stop(); // Close connection after serving response
    Serial.println("client disconnected");
  }
}
