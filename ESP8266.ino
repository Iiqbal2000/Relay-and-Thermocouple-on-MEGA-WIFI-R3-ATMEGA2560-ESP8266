#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

const char* ssid     = "THERMOCONTROL";
const char* password = "123456789";

// current temperature
float t = 0.0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

float suhu;

// Set point Default value
int setPointOff = 50;
int setPointOn = 40;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    *,
    *::before,
    *::after {
      box-sizing: border-box;
    }   
    html {
       font-family: Arial;
       display: inline-block;
       margin: 0px auto;
    }
    h2, p {
      font-size: 3.0rem;
      text-align: center;
    }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    form {
      text-align: center;
    }
    input[type=number] {
      padding: 7px;
      margin: 10px auto;
      box-sizing: border-box;
      display: block;
      -moz-appearance: textfield;
    }
    input[type=submit] {
      margin-top: 10px;
      width: 100px;
      padding: 5px;
    }
    /* Chrome, Safari, Edge, Opera */
    input::-webkit-outer-spin-button,
    input::-webkit-inner-spin-button {
      -webkit-appearance: none;
      margin: 0;
    }
  </style>
</head>
<body>
  <h2>ESP8266 THERMOCOUPLE</h2>
  <p>
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <form method="post" action="/">
    <label for="set_point_on">Setpoint for Relay ON:</label>
    <input type="number" id="set_point_on" name="set_point_on" value="%SET_POINT_ON%">
    <label for="set_point_off">Setpoint for Relay OFF:</label>
    <input type="number" id="set_point_off" name="set_point_off" value="%SET_POINT_OFF%">
    <input type="submit" value="Submit">
  </form>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var) {
  if (var == "TEMPERATURE") {
    return String(t);
  } else if (var == "SET_POINT_ON") {
    return String(setPointOn);
  } else if (var == "SET_POINT_OFF") {
    return String(setPointOff);
  }
  return String();
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(9600);

  // Inisialisasi Access Point
  WiFi.mode(WIFI_AP);
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);
  delay(100);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/", HTTP_POST, [](AsyncWebServerRequest * request) {
    String message1, message2;
    if (request->hasParam("set_point_on", true) || request->hasParam("set_point_off", true)) {
      message1 = request->getParam("set_point_on", true)->value();
      message2 = request->getParam("set_point_off", true)->value();
      // Convert string to integer and assign new setpoint value
      setPointOn = atoi(message1.c_str());
      setPointOff = atoi(message2.c_str());

      StaticJsonDocument<200> fromesp;
      fromesp["setPointOn"] = setPointOn;
      fromesp["setPointOff"] = setPointOff;
      // kirim ke Arduino
      serializeJson(fromesp, Serial);
    } else {
      message1 = "No message sent";
      message2 = "No message sent";
    }
    Serial.print("Set Point ON: ");
    Serial.println(setPointOn);
    Serial.print("Set Point OFF: ");
    Serial.println(setPointOff);
    request->redirect("/");
  });

  // Start server
  server.begin();
}

void loop() {
  if (Serial.available()) {
    StaticJsonDocument<400> doc;
    DeserializationError err = deserializeJson(doc, Serial);
    if (err == DeserializationError::Ok)
    {
      // Print the values
      // (we must use as<T>() to resolve the ambiguity)
      Serial.println("ESP8266 Talking now => ");
      Serial.print("suhu = ");
      Serial.println(doc["suhu"].as<float>());
      suhu = doc["suhu"];
      if (isnan(suhu)) {
        Serial.println("Failed to read from DHT sensor!");
      }
      else {
        t = suhu;
      }

    }
    else
    {
      // Print error to the "debug" serial port
      Serial.print("deserializeJson() returned ");
      Serial.println(err.c_str());

      // Flush all bytes in the "link" serial port buffer
      while (Serial.available() > 0)
        Serial.read();
    }
  }

  delay(500);
}
