#include "DHT.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "WiFi.h"

#define buzzer 2
#define DHTPIN 15
DHT dht(DHTPIN, DHT11);
float heat;
float humidity;
float temperature;

const char* SSID = "Pixel";
const char* PASS = "12345678";
const char* host = "maker.ifttt.com";
const char* apiKey = "ljDp0I5nwX8IDsRtigiYh6yIN9nfXm8DKymsgdUpKw9";
const char* event = "tempExceeded";
unsigned long previousMillis = 0;
unsigned long interval = 30000;

AsyncWebServer server(80);

//***************************************************************************************************************************************************************
void send_email();

String readTemp() {
  temperature = dht.readTemperature();
  while (isnan(temperature)) {
    temperature = dht.readTemperature();
  }
if(temperature > 30){
   digitalWrite(buzzer, HIGH);
   send_email();
   delay(2000);
   digitalWrite(buzzer, LOW);
 }
  return String(temperature);
}

String readHumidity() {
 humidity = dht.readHumidity();
 while (isnan(humidity)) {
    humidity = dht.readHumidity();
  }  
 return String(humidity);
}

String readHeat() {
  heat = dht.computeHeatIndex(temperature, humidity, false);
  return String(heat);
}




//***************************************************************************************************************************************************************


//*******************************************************************************************************************************************
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {
      font-family: Arial; 
      display: inline-block; 
      text-align: center;
    }
    h2 {
      font-size: 3.0rem;
    }
    p {
      font-size: 3.0rem;
    }
    body {
      max-width: 600px; 
      margin:0px auto; 
      padding-bottom: 25px;
    }
    .switch {
      position: relative; 
      display: inline-block; 
      width: 120px; 
      height: 68px;
    } 
    .switch input {
      display: none;
    }
    .slider {
      position: absolute; 
      top: 0; 
      left: 0; 
      right: 0; 
      bottom: 0; 
      background-color: #ccc; 
      border-radius: 6px;
    }
    .slider:before {
      position: absolute; 
      content: ""; 
      height: 52px; 
      width: 52px; 
      left: 8px; 
      bottom: 8px; 
      background-color: #fff; 
      -webkit-transition: .4s; 
      transition: .4s; 
      border-radius: 3px
    }
    input:checked+.slider {
      background-color: #b30000;
    }
    input:checked+.slider:before {
      -webkit-transform: translateX(52px); 
      -ms-transform: translateX(52px); 
      transform: translateX(52px)
    }
  </style>
</head>
<body>
  <h1>ESP Web Server</h1>
  <p>
    <span>Temperature</span> 
    <span id="temp">%TEMPERATURE%</span>
  </p>
  <p>
    <span>Humidity</span> 
    <span id="humidity">%HUMIDITY%</span>
  </p>
  <p>
    <span>Heat Index</span> 
    <span id="heat">%HEAT%</span>
  </p>
  <script>
    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("temp").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/temp", true);
      xhttp.send();
    }, 2000 ) ;

    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("humidity").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/humidity", true);
      xhttp.send();
    }, 2000 ) ;

    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("heat").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/heat", true);
      xhttp.send();
    }, 2000 ) ;

    </script>
</body>
</html>)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  if (var == "TEMPERATURE"){
    return readTemp();
  }
  else if (var == "HUMIDITY"){
    return readHumidity();
  }
  else if (var == "HEAT"){
    return readHeat();
  }
  return String();
}

//*******************************************************************************************************************************************


void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500); 
  pinMode(buzzer,OUTPUT);
  dht.begin();
  delay(1500);

 WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  Serial.println("Connected to WiFi");
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readTemp().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readHumidity().c_str());
  });
  server.on("/heat", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readHeat().c_str());
  });

  // Start server
  server.begin();


}

void loop() {
unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting
if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
  Serial.print(millis());
  Serial.println("Reconnecting to WiFi...");
  WiFi.disconnect();
  Serial.println("Disconnected from WiFi...");
  WiFi.reconnect();
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi Again...");
  Serial.println(WiFi.localIP());
  previousMillis = currentMillis;
}
  }

void send_email(){
      Serial.print("connecting to ");
      Serial.println(host);
      WiFiClient client;
      const int httpPort = 80;
      if (!client.connect(host, httpPort)) {
        Serial.println("connection faibuzzer");
        return;
      }
      
    String url = "/trigger/";
    url += event;
    url += "/with/key/";
    url += apiKey;
  Serial.print("Requesting URL: ");
  Serial.println(url);
     // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
while(client.connected())
  {
    if(client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    } 
    else {
      // No data yet, wait a bit
      delay(50);
    };
  }
  Serial.println();
  Serial.println("closing connection");
  client.stop();
}
