

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
//#include <Adafruit_BME280.h>
#include "time.h"
#include <Firebase_ESP_Client.h>

//#include <esp_now.h>
//#include <esp_wifi.h>
//#include <WiFi.h>
//#include <Adafruit_Sensor.h>
#include <DHT.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "red_interna"
#define WIFI_PASSWORD "backontheair."

// Insert Firebase project API Key
#define API_KEY "AIzaSyCwJBXyeJSIGcxfviMFx6p5j5RFqekhtZk"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "iot@gmail.com"
#define USER_PASSWORD "12345678"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://esp32-iot-54d37-default-rtdb.firebaseio.com/"

//sensor DHT-11
#define DHTPIN 4 
#define DHTTYPE    DHT11
DHT dht(DHTPIN, DHTTYPE);

unsigned long previousMillis = 0;  // Stores last time temperature was published
const long interval = 10000; 

//Lecturas enviadas
unsigned int readingId = 0;
//////

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
/**String tempPath = "/temperature";
String humPath = "/humidity";
String presPath = "/pressure";
**/

String timePath = "/timestamp";

// Sensor DHT-11-NODOS
String tempPath = "/temperature";
String humPath = "/humidity";



// Parent Node (to be updated in every loop)
//losparentPathes el nodo principal 
//que se actualizará en cada ciclo con la marca de tiempo actual.
//estructura del Url = UserData/<user_uid*>/timestamp/temperature
String parentPath;

int timestamp;
FirebaseJson json;

//Solicitaremos la hora desde  pool.ntp.org , que es un grupo
// de servidores de hora que cualquiera puede usar para solicitar la hora.
const char* ntpServer = "pool.ntp.org";

// BME280 sensor
//Adafruit_BME280 bme; // I2C
/**
float temperature;
float humidity;
float pressure;
**/

//DHT-11 SENSOR
float temperature;
float humidity;

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
// 3 minutos 000 /4
unsigned long timerDelay = 180000;


//Metodos del sensor dht 11

// temperatura regresa float 
float readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return 0;
  }
  else {
    Serial.println(t);
    return t;
  }
}
// humedad regresa float 
float readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return 0;
  }
  else {
    Serial.println(h);
    return h;
  }
}





// Initialize BME280

// void initBME(){
//   if (!bme.begin(0x76)) {
//     Serial.println("Could not find a valid BME280 sensor, check wiring!");
//     while (1);
//   }
// }

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup(){
  Serial.begin(115200);

  // Initialize DHT-11 sensor
   dht.begin();

  // Initialize BME280 sensor
  //initBME();
  initWiFi();
  configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

void loop(){

  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath= databasePath + "/" + String(timestamp);

    //sensor bm
    /**json.set(tempPath.c_str(), String(bme.readTemperature()));
    json.set(humPath.c_str(), String(bme.readHumidity()));
    json.set(presPath.c_str(), String(bme.readPressure()/100.0F));
    json.set(timePath, String(timestamp));
    **/
     //sensor dht-11
    json.set(tempPath.c_str(), String(readDHTTemperature()));
    json.set(humPath.c_str(), String(readDHTHumidity()));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}