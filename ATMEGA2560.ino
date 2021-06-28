#include <max6675.h>
#include <ArduinoJson.h>

int thermoDO = 26;
int thermoCS = 24;
int thermoCLK = 22;

MAX6675 thermocouple (thermoCLK, thermoCS, thermoDO);
// PIN relay
int relay = 52; // *

float suhu;
int setPointOn = 40;
int setPointOff = 50;

String statusRelay;

unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 5 seconds
const long interval = 5000;

void relaySet() {
  if (suhu >= setPointOff) {
    digitalWrite(relay, LOW);
    statusRelay = "Relay mati";
    delay(100);
  } else if (suhu <= setPointOn) {
    digitalWrite(relay, HIGH);
    statusRelay = "Relay hidup";
    delay(100);
  }
}

void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);

  // Relay mode
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
}

void loop() {
  relaySet();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    StaticJsonDocument<200> doc;
    suhu = thermocouple.readCelsius();
    doc["suhu"] = suhu;
    //    kirim ke ESP8266
    serializeJson(doc, Serial3);
    serializeJson(doc, Serial);
    Serial.println();
    Serial.println();
    Serial.println(statusRelay);
  }

  if (Serial3.available()) {
    // Allocate the JSON document
    // This one must be bigger than for the sender because it must store the strings
    StaticJsonDocument<300> fromesp;
    DeserializationError errr = deserializeJson(fromesp, Serial3);
    if (errr == DeserializationError::Ok)
    {
      // Print the values
      Serial.println("mega Talking now => ");
      setPointOn = fromesp["setPointOn"];
      setPointOff = fromesp["setPointOff"];
      Serial.println(setPointOn);
      Serial.println(setPointOff);

    }
  }

}
