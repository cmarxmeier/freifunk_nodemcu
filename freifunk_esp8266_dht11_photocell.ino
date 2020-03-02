/* 
 * ESP8266 NodeMCU DHT11 - Humidity Temperature and photocell
 * wifi connects to Freifunk and spans http service
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h> 
#include "DHTesp.h"

#include <AddrList.h>

#define DHTpin 14    //D5 of NodeMCU is GPIO14

ESP8266WebServer server(80);
DHTesp dht;

float temperature;
float humidity;

/*Put your SSID & Password*/
const char*  STASSID="Freifunk";             // << kann bis zu 32 Zeichen haben
const char*  STAPSK="";      // << mindestens 8 Zeichen jedoch nicht länger als 64 Zeichen

/* Lightresistor on port A0*/
int sensorValue=0;
float voltage=0;

String linkLocal, ipv6, ipv4;

bool DEBUG=0;
 
void setup()
{
  Serial.begin(115200);

  // Keep wifi alive
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  
  // Autodetect is not working reliable, don't use the following line
  // dht.setup(17);
 
  // use this instead: 
  dht.setup(DHTpin, DHTesp::DHT11); //for DHT11 Connect DHT sensor to GPIO 17
  //dht.setup(DHTpin, DHTesp::DHT22); //for DHT22 Connect DHT sensor to GPIO 17
  Serial.println();
  // Serial.println(ESP.getFullVersion());
 Serial.println("Connecting to: ");
  Serial.println(STASSID);
  // Verbindungsaufbau zunächst mit IPv4
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println("\nconnected IPv4");
  
#if LWIP_IPV6
// Warten auf die Globale IPv6
  for (bool configured = false; !configured;) {
    for (auto addr : addrList)
      if (configured = addr.isV6() && !addr.isLocal()) {
        break;
      }
    Serial.print('.');
    delay(500);
  }
  Serial.println("\nconnected IPv6\n");
  for (auto a : addrList) {
    a.isV6() ? a.isLocal() ? linkLocal = a.toString() : ipv6 = a.toString() : ipv4 = a.toString();
  }
//URLs für den Browser ausgeben
  Serial.printf("IPv4 address: http://%s\n", ipv4.c_str());
  Serial.printf("IPv6 local: http://[%s]\n", linkLocal.c_str());
  Serial.printf("IPv6 global: http://[%s]\n", ipv6.c_str());
#else
  Serial.printf("IPV6 ist nicht aktiviert\n");
#endif

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}
 
void loop()
{
  //delay(dht.getMinimumSamplingPeriod());
  delay(300);
  humidity = dht.getHumidity();
  temperature = dht.getTemperature();
  if (DEBUG){
    Serial.println();
    Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
    Serial.print(dht.getStatusString());
    Serial.print("\t");
    Serial.print(humidity, 1);
    Serial.print("\t\t");
    Serial.print(temperature, 1);
    Serial.print("\t\t");
    Serial.print(dht.toFahrenheit(temperature), 1);
    Serial.print("\t\t");
    Serial.print(dht.computeHeatIndex(temperature, humidity, false), 1);
    Serial.print("\t\t");
    Serial.println(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);
  }
  // read the input on analog pin 0:
  sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  voltage = sensorValue * (5.0 / 1023.0);
  
  server.handleClient();
}

void handle_OnConnect() {

  server.send(200, "text/html", SendHTML(temperature,humidity)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float Temperaturestat,float Humiditystat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta charset=\"utf-8\"  name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Freifunk NodeMCU Stats</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<img src='http://freifunk.net/wp-content/themes/freifunk_base/img/logo_freifunknet.png'<br>";
  ptr +="<h1>Freifunk NodeMCU Stats</h1>\n";
  
  if (Temperaturestat < 1000){
    ptr +="<p>Temperatur: ";
    ptr +=(int)Temperaturestat;
    ptr +="°C</p>";
    ptr +="<p>Luftfeuchtigkeit: ";
    ptr +=(int)Humiditystat;
    ptr +="%</p>";
  } else {
    ptr +="<p>kein DHT11 Sensor angeschlossen</p>";
  }
  if (voltage >0){
     ptr +="<p>Spannung Photozelle: ";
     ptr +=voltage;
     ptr +=" V ->";
     if (voltage >0.9){
      ptr +="Tag";
     }else{
      ptr +="Nacht";
     }
     ptr +="</p>";
  } else {
    ptr +="<p>kein Photowiderstand angeschlossen</p>";
  }
   if (DEBUG){
    ptr +="<br><p>MAC-Adresse: ";
    ptr +=WiFi.macAddress();
    ptr +="</p>";
    ptr +="<p>OS : ";
    ptr +=ESP.getFullVersion();
    ptr +="</p>";
   }
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
