//1 ImportarLibrerias
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <PubSubClient.h>
#include <DHT.h>

// #include "ESP8266WiFi.h"
// #include "PubSubClient.h"
// #include "DHT.h"
//-------------------VARIABLES GLOBALES--------------------------
//2 configuración de Red
const char* ssid = "iot";      //Usuario red WIFI.
const char* password = "12345678";  //Contraseña WIFI.
//Direccion del Broker.
const char* mqtt_server = "test.mosquitto.org";  //Broker MQTT de prueba
//3 variables y valores iniciales para el cliente WIFI.
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastSend;
int status = WL_IDLE_STATUS;
//4 configuración de sensor
// #define DHTPIN 2       // Pin fisico esp8266
// #define DHTTYPE DHT22  // Tipo de sensor
// DHT dht(DHTPIN, DHTTYPE);

//sensor DHT-11
#define DHTPIN 26
#define DHTTYPE    DHT11
DHT dht(DHTPIN, DHTTYPE);

float h, t;  //variables temperatura y humedad
//5 definición de la variable para publicar el mensaje
String payload;
//Esta función reconecta su ESP8266 al Broker (MQTT)
void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
      //Serial.println("Canectado");
    } else {
      Serial.print("Fallo, rc =");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevamente en 5 segundos");
      delay(1000);
    }
  }
}
//6 configuración de Red
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conexión WiFi - Dirección IP De La ESP8266 : ");
  Serial.println(WiFi.localIP());
}
//7 configuración de la función (setup)
void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Serial.println("hola");
  dht.begin();
  lastSend = 0;
  delay(3000);
}
//Función Loop
void loop() {
  if (!client.connected()) {  //Si no se encuentra el cluiente conectado
    reconnect();              //Se reconectan al Broker MQTT.
  }
  if (millis() - lastSend > 1000) {  // Actializa la lectura y envio cada segundo
    GetSensor();                     // se adquiere y publica en el broker los datos adquiridos por el sensor cada segundo.
    CrearJson();
    PublicarMensaje();
    //Serial.println(payload|);
    //CrearJson(datos);
    lastSend = millis();
  }
  client.loop();  //Mantiene la conexion permanent con el Broker.
}
void GetSensor() {
  // Lee la temperatura o humedad aproximadamente cada 250 milisegundos!
  h = dht.readHumidity();
  // Lee la temperatura en grados Celsius.
  t = dht.readTemperature();
  // Verifica que no existe un error al leer el sensor.
  if (isnan(h) || isnan(t)) {
    Serial.println("fallo al leeer el sensor DHT !");
    return;
  }
}
//configuración formato Json
void CrearJson() {
  String temperatura = String(t);
  String humedad = String(h);
  payload = "{";
  payload += "\"temperatura\":";
  payload += temperatura; payload += ",";
  payload += "\"humedad\":"; payload += humedad;
  payload += "}";
    
  }

void PublicarMensaje() {
  char attributes[100];
  payload.toCharArray(attributes, 100);
  client.publish( "topico/mooc_IoT", attributes );
  Serial.println(attributes);
}
