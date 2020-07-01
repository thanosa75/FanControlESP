#include <DHTesp.h>


#include <ESP8266WebServer.h>

#include <AutoConnect.h>

ESP8266WebServer Server;
AutoConnect      Portal(Server);

DHTesp dht;
float humidity = 0.0;
float temperature = 0.0;
int relay = 16;
volatile byte relayState = LOW;
long lastBounce = 0;
long bounceDelay = 10000;
long dhtMeasure = 0;
long dhtMeasureDelay = 15000;


#define DHT_DEBUG_PRINT 1

void rootPage(){

  char content[] = "<html><body>Hello world - temperature and humidity at <a href=\"/temp\">this link</a></body>";
  Server.send(200, "text/html", content);
}

void tempPage() {
  String content = String( 
  "<html><head><meta http-equiv=\"refresh\" content=\"30\"></head><body><p>Temperature(C) :<div id=\"t\">" + String(temperature) + "</div></p>"+ 
  "<p>Humidity(%) : <div id=\"h\">" + String(humidity) + "</div></p>" +
  "<p>Fans: <div id=\"r\">" + (relayState == HIGH ? "ON" : "OFF")+ "</div></p></body></html>");
  Server.send(200, "text/html", content);
}


void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println("setup phase");

  Server.on("/", rootPage);
  Server.on("/temp", tempPage);
  Portal.begin();
  Serial.println("Web server at " + WiFi.localIP().toString());
    
  dht.setup(2, DHTesp::DHT22); // Connect DHT sensor to GPIO 2
  Serial.println();
  Serial.println();

  //led setup
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
  lastBounce = millis();
  dhtMeasure = millis();
  Serial.println("setup complete");
#ifdef DHT_DEBUG_PRINT
//  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
  Serial.println("Status\tHumidity (%)\tTemperature (C) \tFANS");
#endif
}

void loop() {
  Portal.handleClient();
  computeTempHum();
  checkAndActivateFANS();
}

void checkAndActivateFANS() {
  if (temperature > 0.0 && (millis() - lastBounce > bounceDelay)) {
    if (temperature >= 29.0 && relayState == LOW) {
      Serial.println("RELAY HIGH - Temperature " + String(temperature,1) ); //means connected / ON
      digitalWrite(relay, HIGH);
      relayState = HIGH;      
    } else if (temperature < 27.0 && relayState == HIGH) {
      Serial.println("RELAY LOW - Temperature " + String(temperature,1)); //means disconnected / OFF
      digitalWrite(relay, LOW);
      relayState = LOW;      
    }
    lastBounce = millis();
    
  }
  
}

void computeTempHum() {
  digitalWrite(LED_BUILTIN, HIGH);
#ifdef DHT_DEBUG_PRINT  
  delay(dht.getMinimumSamplingPeriod());
#endif

  if (millis() - dhtMeasure > dhtMeasureDelay) {
    humidity = dht.getHumidity();
    temperature = dht.getTemperature();
    dhtMeasure = millis();
  } else {
    Serial.print("W-");
    delay(1000);
  }
  digitalWrite(LED_BUILTIN, LOW);

#ifdef DHT_DEBUG_PRINT
  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);

  if (relayState == HIGH) {
    Serial.println("\tRelay ON");
  } else {
    Serial.println("\tRelay OFF");
  }
//  Serial.print("\t\t");
//  Serial.print(dht.toFahrenheit(temperature), 1);
//  Serial.print("\t\t");
//  Serial.print(dht.computeHeatIndex(temperature, humidity, false), 1);
//  Serial.print("\t\t");
//  Serial.println(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);
#endif  
}

  


  
