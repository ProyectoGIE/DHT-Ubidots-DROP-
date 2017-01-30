// Created by Ty Tower in June 11 2015
// Modified by GIE
/***********************************************************************
***********************************************************************
 *           
 *           **********      **********     **********
 *           **********      **********     **********
 *           **                  **         **
 *           **                  **         **
 *           **   *****          **         **********
 *           **   *****          **         **********
 *           **      **          **         ** 
 *           **      **          **         **
 *           **********  **  ********** **  **********
 *           **********  **  ********** **  **********
 *               GRUPO DE INNOVACIÓN EDUCATIVA
************************************************************************ 
************************************************************************
                    ESTACIÓN METEOROLÓGICA
     ESTE PROYECTO HA SIDO REALIZADO PARA EL PROYECTO GLOBE - CLIMA
                        AGOSTO DE 2016
*/
//  Este código lee los datos de un sensor DHT22 y los envía a la web Ubidots.com
//  Para este programa, debemos tener creada previamente una cuenta en Ubidots.com

#include <DHT.h>
#include <ESP8266WiFi.h>

#define errorPin 16
#define DHTPIN D4 //conectamos el pin de datos del DHT22 en el pin D4
#define DHTTYPE DHT22

// Inicializamos el DHT22
DHT dht(DHTPIN, DHTTYPE);

// Definimos e inicializamos las constantes que vamos a usar
const int sleepTimeS = 20;  // Tiempo de reposo (en segundos) entre los envíos a Ubidots
long lastReadingTime = 0;
WiFiClient client;
char results[4];

// Después de crear una cuenta en Ubidots, necesitaremos configurar las variables donde 
// guardaremos los datos. Para ello,
// necesitaremos sus "ID" que nos da el propio Ubidots
//De las siguientes ID, activaremos las que procedan dependiendo del sensor que estemos programando
String idvariable1 = "57963fb37625422a29d02d00"; 
String idvariable2 = "57963f1676254222d255c918";

// Además, necesitaremos el API token, que es el que se usa para impedir que otros usuarios
// de Ubidots publiquen sus datos en nuestras variables. Es decir, es el identificativo de nuestra cuenta
String token = "m5WnWGTBovaCyGCza5O9Fd9fYQViKq";

// También tendremos que poner los datos de nuestra red Wi-Fi
const char* ssid = "NombreSSID";    //Sustituir por el nombre de la red
const char* password = "PASSWORD";  //Sustituir por el password de la red

//////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
void ubiSave_value(String, String);

// La función Setup se ejecuta una vez por el ESP8266 cuando se enciende o resetea
void setup()
{
  pinMode(errorPin, OUTPUT); // Establece el pin como salida para informar de su estado
  // mediante parpadeos del pin, podemos saber en qué lugar de la función setup estamos
  for (int i=0;i<4; i++)
  {
    digitalWrite(errorPin ,HIGH);
    delay(200);
    digitalWrite(errorPin ,LOW);
    delay(200);
  }

  // Inicia la comunicación serie (USB) que se utilizará para enviar mensajes de compilación
  // al ordenador
  Serial.begin(115200);
  
  // Comienza la comunicación con el DHT sensor 
  dht.begin();
  // Colocamos un descanso mientras el sensor se inicia
  delay(10);

  // Mensajes indicando que está conectando a la red
  Serial.println();
  Serial.println();
  Serial.print("Buscando ");
  Serial.println(ssid);

  // Utiliza la función scanNetworks para buscar cualquier red disponible
  // Si no encuentra ninguna, hiberna
  int n = WiFi.scanNetworks();

  Serial.println("Búsqueda terminada"); 
  if (n == 0){
    Serial.println("No se han encontrado redes");
    Serial.println("Poniendo en reposo");
// ESP.deepSleep(sleepTimeS * 1000000);
  }

  // Si encuentra nuestra wifi, intenta la conexión
  WiFi.begin(ssid, password);

  // Hasta que la conexión no se logre, sigue intentándolo
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Cuando la conexión tiene éxito, lo comunica mediante mensaje al monitor serie
  Serial.println("");
  Serial.println("Wi-Fi conectada");

}

////////////////////////////////////////////////////////////////////////////////

void loop(){
  // Leemos la temperatura y humedad actuales provistas por el sensor
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Llama a nuestra función ubiSave_value y pasa las medidas 
  // a las correspondientes variables según sus ID de Ubidots
  ubiSave_value(String(idvariable1), String(temp));
  ubiSave_value(String(idvariable2), String(hum));

  // Envía información de estado al monitor serie
  Serial.println("ubidots data");
  Serial.println("temperatura: "+String(temp));
  Serial.println("humedad: "+String(hum));
  Serial.println("En reposo temporalmente!" );

  // El tiempo deepSleep time está definido en microsegundos. Multiplicamos los segundos por 1e6
  //ESP.deepSleep(sleepTimeS * 1000000);//
  
  // Esperamos un tiempo determinado para enviar nuevos datos y no saturar el programa
  delay(300000);  //Envía una medida cada 5 minutos
}

////////////////////////////////////////////////////////////////////////////////

// Agrupamos las medidas de temperatura y humedad para enviarlas a Ubidots
// dentro de la función ubiSave_value
void ubiSave_value(String idvariable, String value) {

  // Preparamos los valores que serán enviados a Ubidots y tomamos la longitud de la cadena 
  // que va a ser enviada
  int num=0;
  String var = "{\"value\": " + String(value)+"}"; // Pasamos los datos a formato JSON
  num = var.length();

  // Si tenemos una conexión con éxito al API de Ubidots
  if (client.connect("things.ubidots.com", 80)) {
    Serial.println("Conectado a Ubidots");
    delay(100);

    // Cosntruimos el POST que vamos a publicar
    client.println("POST /api/v1.6/variables/"+idvariable+"/values HTTP/1.1");
    // También usamos el terminal serie para mostrar cómo queda esta información
    Serial.println("POST /api/v1.6/variables/"+idvariable+"/values HTTP/1.1");
    // Especificamos que el tipo de contenido coincida con el formato de datos (JSON)
    client.println("Content-Type: application/json");
    Serial.println("Content-Type: application/json");
    // Especificamos la longitud del contenido
    client.println("Content-Length: "+String(num));
    Serial.println("Content-Length: "+String(num));
    // Usamos nuestra propia API token para poder publicar los datos
    client.println("X-Auth-Token: "+token);
    Serial.println("X-Auth-Token: "+token);
    // Especificamos el host
    client.println("Host: things.ubidots.com\n");
    Serial.println("Host: things.ubidots.com\n");
    // Enviamos los datos
    client.print(var);
    Serial.print(var+"\n");
  }
  else {
    // Si no podemos establecer conexión con el servidor:
    Serial.println("La conexión a Ubidots ha fallado ");
  }
  // Si perdemos la conexión Wi-Fi
  if (!client.connected()) {
    Serial.println("No Conectado");
    Serial.println("desconectando ubidots.");
    client.stop();
    // no hacer nada más:
    for(;;);
  }
  // Si nuestra conexión a Ubidots is buena, leemos la respuesta de Ubidots
  // y la imprimos en nuestro Serial Monitor
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
}
