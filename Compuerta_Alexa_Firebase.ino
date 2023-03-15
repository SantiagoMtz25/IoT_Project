/*Alexa -- Librerias */
#include <Arduino.h>
#include <ESP8266WiFi.h>      //Libreria WiFi
#include "SinricPro.h"        // Libreria Sinric
#include "SinricProSwitch.h"  //Libreria Sinric Switch
#include <map>                //se guarda pines en map

/*Ultrasonic -- Librerias */
#include <HCSR04.h>  //Libreria para ultrasonico

/*Firebase -- Librerias*/
#include <Firebase_ESP_Client.h>  //Firebase
#include <addons/RTDBHelper.h>    //RTDB helper functions

/*Alexa -- Definiciones*/
//Definiendo red para node y appkey y appsecret para sinric
#define WIFI_SSID "iPhone de Demian"
#define WIFI_PASS "71027102"
#define APP_KEY "2e37b19f-d2c5-4543-8c25-9892838c79b2"
#define APP_SECRET "88d997e8-e356-4714-9c28-6f81fd97c5a6-ff611bf9-e3a4-4fa1-81f9-5bb9ff878074"
#define device_ID_1 "63646d90333d12dd2ae52c0d"  //Definiendo ids de devices (se obtinen de sinric)

/*Ultrasonic -- Definiciones*/
float distance = 0;
#define ledCompuertapin D2    //Led compuerta verde
#define ledSwitchpin D3       //Led switch rojo
#define botonCompuertapin D1  // Botón para abrir compuerta
#define botonSwitchpin D0     // utilizar boton manual o sensor
const byte triggerPin = D7;   //Pin para trigger ultrasonic
const byte echoPin = D8;      //Pin para echo ultrasonic
//Variables que controlan los estados
int botonStateSwitch = 1;             //Lee el valor del switch
int botonStateCompuerta = 1;          //Lee el valor de la compuerta
bool botonOrcompuerta = true;         //Estado que controla si esta en la compuerta o   con el sensor
bool estado_switch_led = true;        //Controla el estado del led switch
bool estado_compuerta_led = false;    //Controla el estado del led compuerta
bool estado_boton_compuerta = false;  //Estado que controla la compuerta
//Initializa ultrasonic
UltraSonicDistanceSensor distanceSensor(triggerPin, echoPin);

/*Firebase -- Definiciones*/
#define API_KEY "AIzaSyADnaKrjbK4650L0P-YNrB-Y4UHT-lkV7g"
#define DATABASE_URL "https://fir-1-5ea1e-default-rtdb.firebaseio.com/"
#define USER_EMAIL "a01368643@tec.mx"
#define USER_PASSWORD "123456"
FirebaseData fbdo;      //Initialize Firebase objects
FirebaseAuth auth;      //Initialize Firebase objects
FirebaseConfig config;  //Initialize Firebase objects
unsigned long sendDataPrevMillis = 0;
bool tempEstadoCompuerta = false;  //Para set
bool tempEstadoSwitch = false;     //Para set
bool switchFirebase = false;
bool tempswitchFirebase = false;
bool compuertaFirebase = false;
bool tempcompuertaFirebase = false;
String messageObject = "";

/*============Funciones setup=========*/
/*Alexa -- funcion para obtener estado de Alexa*/
bool onPowerState(String deviceId, bool &state) {
  Serial.printf("%s: %s\r\n", deviceId.c_str(), state ? "on" : "off");
  int relayPIN = ledSwitchpin;  // get the relay pin for corresponding device
  if (state == 1) {
    botonOrcompuerta = true;
    Serial.printf("Encendido desde Alexa");
  }
  if (state == 0) {
    botonOrcompuerta = false;
    Serial.printf("Apagado desde Alexa");
  }
  return true;
}
/*Alexa -- funcion para setup de wifi   */
void setupWiFi() {
  Serial.printf("\r\n[Wifi ALEXA]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  Serial.printf("connected[ALEXA]!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}
/*Alexa -- funcion para setup sinric, conexion de devices*/
void setupSinricPro() {
  SinricProSwitch &mySwitch = SinricPro[device_ID_1];
  mySwitch.onPowerState(onPowerState);

  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
}

/*Firebase -- funcion para setup*/
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

//=============setup()================//
void setup() {

  Serial.begin(9600);

  /*Ultrasonic*/
  //Declarando botones
  pinMode(botonCompuertapin, INPUT);
  pinMode(botonSwitchpin, INPUT);
  //Declarando leds
  pinMode(ledCompuertapin, OUTPUT);
  pinMode(ledSwitchpin, OUTPUT);

  /*Alexa*/
  //Llamando a funciones para setup de Wifi y sinric
  setupWiFi();
  setupSinricPro();

  /*Firebase*/
  //Llamnado funcion para setup
  setupFirebase();
  //Llamando funcion para get de compuerta y switch
}
//=============loop()================//
void loop() {
  /*Ultrasonic*/
  openCompuertaWithUltrasonic();

  /*Alexa*/
  SinricPro.handle();

  /*Firebase set*/
  setUltraFirebase();
  getSwitchAndCompuertaFromFirebase();
}


void openCompuertaWithUltrasonic() {
  // Obtiene el estado del switch
  botonStateSwitch = digitalRead(botonSwitchpin);  //OBTIENE EL VALOR DEL BOTÓN
  delay(250);
  if (botonStateSwitch == 1) {
    if (botonOrcompuerta == true) {
      botonOrcompuerta = false;
    } else {
      botonOrcompuerta = true;
    }
  }

  //Obtiene distancia
  distance = distanceSensor.measureDistanceCm();

  //Compuerta funciona con sensor (botonOrcompuerta == true)
  if (botonOrcompuerta == true) {
    estado_switch_led = true;
    if (distance >= 15 && distance <= 40) {
      estado_compuerta_led = true;
    } else {
      estado_compuerta_led = false;
    }
  }

  //Compuerta funciona manualmente--con boton--((botonOrcompuerta == false))
  if (botonOrcompuerta == false) {
    estado_switch_led = false;
    botonStateCompuerta = digitalRead(botonCompuertapin);
    delay(250);
    if (botonStateCompuerta == 1) {
      if (estado_boton_compuerta == true) {
        estado_boton_compuerta = false;
      } else {
        estado_boton_compuerta = true;
      }
    }

    if (estado_boton_compuerta == true) {
      estado_compuerta_led = true;
    }
    if (estado_boton_compuerta == false) {
      estado_compuerta_led = false;
    }
  }


  //Prende led de la compuerta
  if (estado_compuerta_led == true) {
    digitalWrite(ledCompuertapin, 1);
  }
  if (estado_compuerta_led == false) {
    digitalWrite(ledCompuertapin, 0);
  }
  //Prende led del switch
  if (estado_switch_led == true) {
    digitalWrite(ledSwitchpin, 1);
  }
  if (estado_switch_led == false) {
    digitalWrite(ledSwitchpin, 0);
  }
}

void setUltraFirebase() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)) {

    if (estado_compuerta_led != tempEstadoCompuerta) {
      tempEstadoCompuerta = estado_compuerta_led;

      messageObject = estado_compuerta_led ? "Hay Objeto" : "No hay Objeto";

      if (botonOrcompuerta) {
        Serial.printf("Set Ultra... %s\n", Firebase.RTDB.setString(&fbdo, F("/test/Ultra"), messageObject) ? "ok" : fbdo.errorReason().c_str());
      } else {
        Serial.printf("Set Ultra... %s\n", Firebase.RTDB.setString(&fbdo, F("/test/Ultra"), "Ultrasonido Apagado") ? "ok" : fbdo.errorReason().c_str());
      }
    }
    if (botonOrcompuerta != tempEstadoSwitch) {
      tempEstadoSwitch = botonOrcompuerta;

      Serial.printf("Set Ultra... %s\n", Firebase.RTDB.setString(&fbdo, F("/test/Ultra"), "Ultrasonido Apagado") ? "ok" : fbdo.errorReason().c_str());
    }
  }
}

void getSwitchAndCompuertaFromFirebase() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)) {

    if (Firebase.RTDB.getBool(&fbdo, F("/test/Switch"))) {
      switchFirebase = fbdo.to<bool>();
    } else {
      Serial.println(fbdo.errorReason().c_str());
    }
    if (switchFirebase != tempswitchFirebase) {
      tempswitchFirebase = switchFirebase;
      botonOrcompuerta = switchFirebase;
    }

    if (!botonOrcompuerta) {
      if (Firebase.RTDB.getBool(&fbdo, F("/test/Compuerta"))) {
        compuertaFirebase = fbdo.to<bool>();
      } else {
        Serial.println(fbdo.errorReason().c_str());
      }
      if (compuertaFirebase != tempcompuertaFirebase) {
        tempcompuertaFirebase = compuertaFirebase;
        estado_boton_compuerta = compuertaFirebase;
      }
    }
  }
}
