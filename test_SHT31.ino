#include "Adafruit_SHT31.h"

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <ezTime.h>

ESP8266WiFiMulti WiFiMulti;

bool enableHeater = false;
uint8_t loopCnt = 0;

Adafruit_SHT31 sht31 = Adafruit_SHT31();

// definir parametros de la wifi local
const char* ssid = "MOVISTAR_23DF";
const char* password = "avDUjM5GSbuoWSHPKRzG";

String sensorPosition = "pruebas";

// Api Endpoint
// ubuntu
// String serverName = "http://192.168.1.35:8080/data";
// raspberry
String serverName = "http://192.168.1.68:8080/data";

void setup() {
  Serial.begin(9600);

  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");

  // inicializacion de la conexion con la wifi
  WiFi.begin(ssid, password);
  while( WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WIFI connected!");

  // Wait for ezTime to get its time synchronized
  waitForSync();
  // Set NTP polling interval to 60 seconds. Way too often, but good for demonstration purposes.
  setInterval(60);

}


void loop() {
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();

  if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.print(t); Serial.print("\t\t");
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
  } else { 
    Serial.println("Failed to read humidity");
  }

  delay(1000);

  // Toggle heater enabled state every 30 seconds
  // An ~3.0 degC temperature increase can be noted when heater is enabled
  if (loopCnt >= 30) {
    enableHeater = !enableHeater;
    sht31.heater(enableHeater);
    Serial.print("Heater Enabled State: ");
    if (sht31.isHeaterEnabled())
      Serial.println("ENABLED");
    else
      Serial.println("DISABLED");

    loopCnt = 0;
  }

  // check if is not a number
  if (! isnan(t) && ! isnan(h)) {
    if(WiFi.status()== WL_CONNECTED){
        HTTPClient http;
        WiFiClient client;
        String serverPath = serverName;

        // Your Domain name with URL path or IP address with path
        http.begin(client, serverPath.c_str());

       Serial.println("UTC: " + UTC.dateTime());
       Serial.println("Temperatura" + String(t));
       Serial.println("Humedad" + String(h));
       Serial.println("Position: " + sensorPosition);

       // Definir el timezone para España
       Timezone Spain;
       Spain.setLocation("Europe/Madrid");

        // If you need an HTTP request with a content type: application/json, use the following:
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.POST("{\"date\":\" " + Spain.dateTime(ISO8601)
                                        + "\",\"temperature\":\"" + String(t)
                                        + "\",\"humidity\":\"" + String(h)
                                        + "\",\"location\":\"" + sensorPosition +"\" }");

        if (httpResponseCode>0) {
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
          String payload = http.getString();
          Serial.println(payload);
        }
        else {
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
        }
        // Free resources
        http.end();
    }
    else {
        Serial.println("WiFi Disconnected");
    }
  }
  loopCnt++;
}
