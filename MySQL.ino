//librerias firebase
#include <Firebase_ESP_Client.h>
#include <addons/RTDBhelper.h>

// Librerías para la conexión con WiFi y Http
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
//red wifi
const char* ssid = "tilin";
const char* password = "holaaaaa";
// Cliente web
HTTPClient httpClient;
WiFiClient wClient;
//Link a tabla base de datos
String URL = "http://192.168.175.84:6000/api/sensorLog";

// FIREBASE DEFINICIONES
#define API_KEY "AIzaSyADnaKrjbK4650L0P-YNrB-Y4UHT-lkV7g"
#define DATABASE_URL "https://fir-1-5ea1e-default-rtdb.firebaseio.com/"
#define USER_EMAIL "a01368643@tec.mx"
#define USER_PASSWORD "123456"
FirebaseData fbdo;      //Initialize Firebase objects
FirebaseAuth auth;      //Initialize Firebase objects
FirebaseConfig config;  //Initialize Firebase objects
unsigned long sendDataPrevMillis = 0;
String messageObject = "No hay Objeto";
String tempmessageObject = "";
String message = "";
String tempmessage = "";

//Firebase -- funcion para setup
void setupFirebase() {
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  fbdo.setBSSLBufferSize(2048, 2048);
  fbdo.setResponseSize(2048);
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
  config.timeout.serverResponse = 10 * 1000;
}

void setup() {
  Serial.begin(9600);
  Serial.println("***Inicializando conexión a My SQL***");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid, password);
  Serial.print("Conectando a red WiFi \"");
  Serial.print(ssid);
  Serial.print("\"");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConectado! IP: ");
  Serial.println(WiFi.localIP());
  delay(500);

  setupFirebase();  //SETUP FIREBASE
}

void loop() {
  getFromfirebase();

  if(messageObject!=tempmessageObject) {
    tempmessageObject = messageObject;
    if (messageObject == "Hay Objeto") {
      logIntento(messageObject);
    }

  }

}

void logIntento(String messageObject) {
  if (WiFi.status() == WL_CONNECTED) {
    httpClient.begin(wClient, URL);  //indica tabla base de datos
    String data;
    data = "{\"idSensor\":\"1\",\"valueLog\":\"";
    data = data + messageObject;
    data = data + "\"}";
    Serial.println(data);

    httpClient.addHeader("Content-Type", "application/json");
    //int httpResponseCode = httpClient.POST("{\"idSensor\":\"1\",\"valueLog\":\"24.25\",\"dateLog\":\"2022-11-28 10:43:18\"}");
    int httpResponseCode = httpClient.POST(data);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    httpClient.end();
  }
  return;
}

void getFromfirebase() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)) {

    if (Firebase.RTDB.getString(&fbdo, F("/test/Ultra"))) {
      message = fbdo.to<String>();
    } else {
      Serial.println(fbdo.errorReason().c_str());
    }

    if (message != tempmessage) {
      tempmessage = message;

      if (message != "Ultrasonido Apagado") {
        messageObject = message;
      }
    }
  }
}