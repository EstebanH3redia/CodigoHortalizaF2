// Librería time
#include <TimeLib.h>
// Librería DHT
#include <DHT.h>
#include <PinChangeInterrupt.h> // Incluye la biblioteca para interrupciones de cualquier pin
#include <Adafruit_Sensor.h>        // Incluye la librería de sensores de Adafruit
#include <Adafruit_BMP085.h>        // Incluye la librería específica para el BMP180
#include <Wire.h>
#include <ArduinoJson.h>
// Definición de pin y tipo DHT22
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); // Instancia

//Declaramos los pines_ y el relay____________________________________
const int sensorPinCC = A0; // Cama Automatizado 1
const int sensorPinCG = A1; // Cama Automatizado 2
const int sensorPinCE = A2; // Cama Tradicional 1
const int sensorPinCF = A3; // Cama Tradicional 2

const int humedadAire = 550; // Valor máximo para aire seco
const int humedadAgua = 250;    // Valor mínimo para agua
const int relayPinCC = 2;
const int relayPinCG = 3;
//________________________________________________________________


int cama1 = 0, cama2 = 0;

unsigned long previousMillisCC = 0;
unsigned long previousMillisCG = 0;
unsigned long intervalCC = 1800000 ;  // 30 minutos=1800000 Automatico 1 ____1 minut0 = 60000____
unsigned long intervalCG = 300000 ; // 5 minutos=300000  Automatico 2


//____________________VARIABLES DEL FLUJO DE AGUA PARA LA CAMA AUNTOMATIZADO 1//________________________
int PinSensorA1 = 6; // Sensor de flujo para la cama 1
volatile int NumPulsosA1 = 0; // Pulsos recibidos
float factor_conversionA1 = 6.75; // Conversión de frecuencia a caudal
float volumenA1 = 0;


//__________________________________________________________________________________
//_________________________VARIABLES DEL FLUJO DE AGUA_DE LA CAMA AUTOMATIZADO 2________________________________
int PinSensorA2 = 4; // Sensor de flujo para la cama 2
volatile int NumPulsosA2 = 0; // Pulsos recibidos 
float factor_conversionA2 = 6.75; // Conversión de frecuencia a caudal
float volumenA2 = 0;

//_________________________________________________________________________________________________________________

//---Funcion Contar Pulsos de ambos Flujos---------------
void ContarPulsosA1() {

  NumPulsosA1++;

}

void ContarPulsosA2() {

  NumPulsosA2++;

}
//---Función para obtener frecuencia de los pulsos--------
int ObtenerFrecuenciaA1() {
  int frecuenciaA1 = NumPulsosA1;
  NumPulsosA1 = 0;
  return frecuenciaA1;
}

int ObtenerFrecuenciaA2() {
  int frecuenciaA2 = NumPulsosA2;
  NumPulsosA2 = 0;
  return frecuenciaA2;
}

//______________________________________________________________________________________________________________
//________________________CREAMOS OBJETO__________________________________________________________ 
Adafruit_BMP085 bmp;                // Crea un objeto BMP180
//__________________________________________________________________________________________

 
 //______________________CREAMOS UNA FUNCION__PARA EL REGISTRO DE LA HUMEDAD DE LAS CAMAS_______________________________________________________
 void ImprimirRegistroCama(JsonDocument sensorReading, int humedadCama, char* numeroCama) {
  sensorReading["Sensor"] = "humedadCama";
  sensorReading["NumeroCama"] = numeroCama;
  sensorReading["Humedad"] = humedadCama;
  serializeJson(sensorReading, Serial);
  sensorReading.clear();
  Serial.println();
}
//_________________________________________________________________________________


void setup() {
//mantenemos los pines con el relay______________________________________
  pinMode(sensorPinCC, INPUT);
  pinMode(sensorPinCG, INPUT);
  pinMode(sensorPinCE, INPUT);
  pinMode(sensorPinCF, INPUT);
  pinMode(relayPinCC, OUTPUT);
  pinMode(relayPinCG, OUTPUT);

  digitalWrite(relayPinCC, HIGH); // Relé apagado
  digitalWrite(relayPinCG, HIGH); // Relé apagado
//______________________________________________________________________

  dht.begin();
  Serial.begin(9600);


   // Fijamos la fecha y hora actuales
  setTime(13, 1, 0, 27, 11, 2024); // Hora, Min, Seg - Día, Mes, Año

  //___________INICIO PRESION BAROMETRICA______________________
   Serial.println(" Iniciando Sistema De Riego:");			// texto de inicio
  if ( !bmp.begin() ) {				// si falla la comunicacion con el sensor mostrar
    Serial.println("Error BMP280 no encontrado");	
  }

  //______________CONFIGURACION DE PIN SENSO FLUJO DE AGUA AUTOMATIZADO 1 y 2____________________________________________________________________
  // Configuración de pines de sensores de flujo
  pinMode(PinSensorA1, INPUT);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(PinSensorA1), ContarPulsosA1, RISING);
  
  pinMode(PinSensorA2, INPUT);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(PinSensorA2), ContarPulsosA2, RISING);
  //_______________________________________________________________________
  //_______________________________________________________________________
}
//Leemos la humedad_____________________________________________________________
int leerHumedad(int sensorPin) {
  int valorHumedad = analogRead(sensorPin);
  int porcentajeHumedad = map(valorHumedad, humedadAire, humedadAgua, 0, 100);
  if (porcentajeHumedad > 100) porcentajeHumedad = 100;
  if (porcentajeHumedad < 0) porcentajeHumedad = 0;
  return porcentajeHumedad;
}
//_________________________________________________________________________________


void loop() {
  unsigned long currentMillis = millis();
   JsonDocument sensorReading;
  // Sensor de medio ambiente
  float temperature = dht.readTemperature(); // Lee la temperatura en grados Celsius
  float humidity = dht.readHumidity(); // Lee la humedad relativa en porcentaje
  if (!isnan(temperature) && !isnan(humidity)) {
    // Crear JSON y asignar valores
    StaticJsonDocument<200> sensorReading;
    sensorReading["Sensor"] = "temperatura";
    sensorReading["Temperatura"] = temperature;
    sensorReading["Humedad"] = humidity;

    // Serializar el JSON a la salida serial
    serializeJson(sensorReading, Serial);
    Serial.println(); 
  } else {
    Serial.println("Error al obtener informacion del sensor de aire");
  }

  //_________PRESION BAROMETRICA__________________________
 sensorReading["Sensor"] = "presionBarometrica";
  sensorReading["Temperatura"] = bmp.readTemperature() ;
  sensorReading["Presion"] = bmp.readPressure() ;
  sensorReading["Altitud"] = bmp.readAltitude();

  serializeJson(sensorReading, Serial);
  sensorReading.clear();
  Serial.println();
  //_________________________________________________________
//_____________Declaramos el Porcentaje______________________________________________________________________
  int porcentajeCC = leerHumedad(sensorPinCC);
  int porcentajeCG = leerHumedad(sensorPinCG);
  int porcentajeCE = leerHumedad(sensorPinCE);
  int porcentajeCF = leerHumedad(sensorPinCF);
//IMPRIMIMOS EL VALOR DE PORCENTAJE DE LAS CAMAS 
  ImprimirRegistroCama(sensorReading, porcentajeCC, "1");
  ImprimirRegistroCama(sensorReading, porcentajeCG, "2");
 // ImprimirRegistroCama(sensorReading, porcentajeCE, "3");
 // ImprimirRegistroCama(sensorReading, porcentajeCF, "4");
//____________________________________________________________________________________________
   
  // Variables para las condiciones
  int hora = hour();

  // Condiciones para ambas camas
  if ((hora >= 6 && hora <=12) || (hora >= 18 && hora <=24)) {
    // Cama Automatizado 1__________________________________________________________________________
    if (porcentajeCC <= 50 && porcentajeCC > 0  && cama1 == 0) {
      digitalWrite(relayPinCC, LOW); // Encendemos la electrovalvula 1
      cama1 = 1;
      previousMillisCC = currentMillis;
     Serial.println("El riego de la cama Automatizado 1 está activo");
    }
    
    if (cama1 == 1 && currentMillis - previousMillisCC >= intervalCC) {
      digitalWrite(relayPinCC, HIGH); // Apagamos la electrovalvula 1
      cama1 = 0;
      Serial.println("El riego de la cama Automatizado 1 se apagó");
          
      //__________________________CALCULA EL VOLUMEN DEL AGUA_AUTOMATIZADO 1_________________________________

      
      float frecuenciaA1 = ObtenerFrecuenciaA1();
      float caudalA1_L_m = frecuenciaA1 / factor_conversionA1;
      volumenA1 += (caudalA1_L_m / 60.0); // Volumen incrementado en litros

      // ENVIO DE INFORMACION DEL FLUJO DE AGUA DE LA CAMA AUTOMATICO 1
     sensorReading["Sensor"] = "flujoAgua";
     sensorReading["TiempoTranscurridoMilis"] = intervalCC;
     sensorReading["NombreSembrado"] = "Cama 1 cilantro Automatico";
     sensorReading["CultivoId"] = 1;
     sensorReading["Volumen"] = volumenA1,3;
     serializeJson(sensorReading, Serial);
     sensorReading.clear();
     Serial.println();
     volumenA1 = 0; // Reiniciar para la próxima vez
       //___________________________________________________________________________________________

    }

    // Cama Automatizado 2_______________________________________________________________________________________
    if (porcentajeCG <= 50 && porcentajeCG > 0 && cama2 == 0) {
      digitalWrite(relayPinCG, LOW); // Encendemos la electrovalvula 2
      cama2 = 1;
      previousMillisCG = currentMillis;
      Serial.println("El riego de la cama Automatizado 2 está activo "); 
    //__________________________________________________________________________________________________________

    }

    if (cama2 == 1 && currentMillis - previousMillisCG >= intervalCG) {
      digitalWrite(relayPinCG, HIGH); // Apagamos la electrovalvula 2
      cama2 = 0;
      Serial.println("El riego de la cama Automatizado 2 se apagó ");
       
       //__________________________CALCULA EL VOLUMEN DEL AGUA_AUTOMATIZADO 2_________________________________

      float frecuenciaA2 = ObtenerFrecuenciaA2();
      float caudalA2_L_m = frecuenciaA2 / factor_conversionA2;
      volumenA2 += (caudalA2_L_m / 60.0); // Volumen incrementado en litros
       // ENVIO DE INFORMACION DEL FLUJO DE AGUA DE LA CAMA AUTOMATICO 2
       sensorReading["Sensor"] = "flujoAgua";
       sensorReading["TiempoTranscurridoMilis"] = intervalCG;
       sensorReading["NombreSembrado"] = "Cama 2 rabano Automatico";
       sensorReading["CultivoId"] = 2;
       sensorReading["Volumen"] = volumenA2,3;
       serializeJson(sensorReading, Serial);
       sensorReading.clear();
       Serial.println();
      volumenA2 = 0; // Reiniciar para la próxima vez
       //___________________________________________________________________________________________
       
    }
  } else {
    Serial.println("NO es momento de regar");
  }

  delay(60000); // Espera 1 minuto antes de la siguiente lectura
}