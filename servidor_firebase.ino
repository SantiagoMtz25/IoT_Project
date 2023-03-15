/*Servidor Web -- Librerias*/
#include <ESP8266WiFiMulti.h>
#include "data.h"

/*Firebase -- Librerias*/
#include <Firebase_ESP_Client.h>  //Firebase
#include <addons/RTDBHelper.h>    //RTDB helper functions

/*Servidor Web -- Definiciones*/
ESP8266WiFiMulti wifiMulti;
WiFiServer server(80);
String messageObject = "No hay Objeto";
String message = "";
String tempmessage = "";
bool objectExist = false;  //True si hay objeto, False si hay objeto

/*Firebase -- Definiciones*/
#define API_KEY "AIzaSyADnaKrjbK4650L0P-YNrB-Y4UHT-lkV7g"
#define DATABASE_URL "https://fir-1-5ea1e-default-rtdb.firebaseio.com/"
#define USER_EMAIL "a01368643@tec.mx"
#define USER_PASSWORD "123456"
FirebaseData fbdo;      //Initialize Firebase objects
FirebaseAuth auth;      //Initialize Firebase objects
FirebaseConfig config;  //Initialize Firebase objects
unsigned long sendDataPrevMillis = 0;
bool botonOrcompuerta = false;
bool tempbotonOrcompuerta = false;
bool estado_boton_compuerta = false;
bool tempestado_boton_compuerta = false;

/*Servidor Web -- funcion setup*/
void setupWiFi() {
  // Conectandose al Wifi
  Serial.println();
  Serial.println();
  Serial.print("[Servidor Web ]Conectandose a");
  wifiMulti.addAP(ssid_1, password_1);
  WiFi.mode(WIFI_STA);
  while (wifiMulti.run(5000) != WL_CONNECTED) {
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("[Servidor Web] WiFi Conectado ");
  Serial.print("SSID:");
  Serial.print(WiFi.SSID());
  Serial.print(" ID:");
  Serial.print("Usa esta URL para comunicar al ESP: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  server.begin();
  Serial.println("Servidor iniciado");
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

  /*setup WiFi*/
  setupWiFi();

  /*Firebase setup*/
  setupFirebase();
}
//=============loop()================//
void loop() {
  /*Hostea servidor y crea pagina web*/
  servidorWeb();

  /*set en firebase dependiendo de pagina web*/
  setFromwebpage();
  /*get from firebase and change webpage*/
  getFromfirebase();
}

void servidorWeb() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  Serial.println("Nuevo cliente");
  while (!client.available()) {
    delay(1);
  }

  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Relacionamos la solicitud
  //Control de Switch
  if (request.indexOf("/Ultrasonic=ON") != -1) {
    botonOrcompuerta = true;
  }
  if (request.indexOf("/Ultrasonic=OFF") != -1) {
    botonOrcompuerta = false;
  }
  //Control de compuerta
  if (request.indexOf("/Compuerta=ON") != -1) {
    if (botonOrcompuerta == false) {
      estado_boton_compuerta = true;
    }
  }

  if (request.indexOf("/Compuerta=OFF") != -1) {
    if (botonOrcompuerta == false) {
      estado_boton_compuerta = false;
    }
  }

  // if (request.indexOf("/Ultra") != -1) {
  //   if (objectExist) {
  //     messageObject = "Hay Objeto";
  //   } else {
  //     messageObject = "No hay Objeto";
  //   }
  // }

  paginaWeb(client);

  delay(1);
  Serial.println("Cliente desconectado");  //Imprimimos que terminó el proceso con el cliente desconectado
  Serial.println("");
}

void paginaWeb(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");          // La respuesta empieza con una linea de estado
  client.println("Content-Type: text/html");  //Empieza el cuerpo de la respuesta indicando que el contenido será un documento html
  client.println("");                         // Ponemos un espacio
  client.println("<!DOCTYPE HTML>");          //Indicamos el inicio del Documento HTML
  client.println("<html lang=\"en\">");
  client.println("<head>");
  client.println("<meta charset=\"UTF-8\">");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");  //Para que se adapte en móviles
  client.println("<title>Compuerta Inteligente</title>");
  client.println("</head>");
  client.println("<body>");
  client.println("<br><br>");

  client.println("<h1 style=\"text-align: center;\">Compuerta Inteligente</h1>");

  client.println("<p style=\"text-align: center;\">");

  client.println("<button onclick=location.href=\"/Ultrasonic=ON\"> Activar Ultrasonico</button> <br> <br>");  // Botón sencillo que contiene hipervínculo
  client.println("<button onclick=location.href=\"/Ultrasonic=OFF\" >Desactivar Ultrasonico </button> <br> <br>");
  client.println("<button onclick=location.href=\"/Compuerta=ON\"> Abrir Compuerta</button> <br> <br>");  // Botón sencillo que contiene hipervínculo
  client.println("<button onclick=location.href=\"/Compuerta=OFF\" >Cerrar Compuerta</button> <br> <br>");
  // client.println("<button onclick=location.href=\"/Ultra\">Lectura Ultrasonico</button> <br> <br>");

  client.println("<label for=\"fname\">Ultrasonico:</label>");
  client.println("<input style=\"text-align: center;\" type=\"text\"  value=\'");
  client.print(messageObject);
  client.print("'><br>");

  client.println("</p>");
  client.println("</body>");

  client.println("</html>");  //Terminamos el HTML
}

void setFromwebpage() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0)) {

    if (botonOrcompuerta != tempbotonOrcompuerta) {
      tempbotonOrcompuerta = botonOrcompuerta;
      Serial.printf("Set Switch... %s\n", Firebase.RTDB.setBool(&fbdo, F("/test/Switch"), botonOrcompuerta) ? "ok" : fbdo.errorReason().c_str());
    }


    if (estado_boton_compuerta != tempestado_boton_compuerta) {
      tempestado_boton_compuerta = estado_boton_compuerta;
      Serial.printf("Set Compuerta... %s\n", Firebase.RTDB.setBool(&fbdo, F("/test/Compuerta"), estado_boton_compuerta) ? "ok" : fbdo.errorReason().c_str());
    }
  }
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
