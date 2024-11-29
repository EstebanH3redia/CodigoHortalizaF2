#include <PinChangeInterrupt.h>
#include <Wire.h>
#include <ArduinoJson.h>

// Definición de pines de sensores de flujo
int PinSensorA1 = 2; // Sensor de flujo para la cama 1
int PinSensorA2 = 3; // Sensor de flujo para la cama 2

// Variables de flujo de agua para la cama 1
volatile int NumPulsosA1 = 0; // Pulsos recibidos
float factor_conversionA1 = 6.75; // Conversión de frecuencia a caudal
float volumenA1 = 0;
unsigned long tiempoInicioRiegoA1 = 0; //Tiempo del inicio de la lectura
unsigned long tiempoUltimoPulsoA1 = 0; // Tiempo de la última lectura
bool riegoActivoA1 = false; // Indicador de si el riego está activo

// Variables de flujo de agua para la cama 2
volatile int NumPulsosA2 = 0; // Pulsos recibidos
float factor_conversionA2 = 6.75; // Conversión de frecuencia a caudal
float volumenA2 = 0;
unsigned long tiempoInicioRiegoA2 = 0; //Tiempo del inicio de la lectura
unsigned long tiempoUltimoPulsoA2 = 0; // Tiempo de la última lectura
bool riegoActivoA2 = false; // Indicador de si el riego está activo

// Función para contar pulsos del sensor de flujo de la cama 1
void ContarPulsosA1() {
  if (!riegoActivoA1) {
    tiempoInicioRiegoA1 = millis(); // Registra el tiempo al inicio del riego
  }

  NumPulsosA1++;
  riegoActivoA1 = true; // El riego está activo
  tiempoUltimoPulsoA1 = millis(); // Actualizar el tiempo del último pulso
}

// Función para contar pulsos del sensor de flujo de la cama 2
void ContarPulsosA2() {
  if (!riegoActivoA2) {
    tiempoInicioRiegoA2 = millis(); // Registra el tiempo al inicio del riego
  }
  NumPulsosA2++;
  riegoActivoA2 = true; // El riego está activo
  tiempoUltimoPulsoA2 = millis(); // Actualizar el tiempo del último pulso
}

// Función para obtener la frecuencia de pulsos del sensor de la cama 1
int ObtenerFrecuenciaA1() {
  int frecuenciaA1 = NumPulsosA1;
  NumPulsosA1 = 0;
  return frecuenciaA1;
}

// Función para obtener la frecuencia de pulsos del sensor de la cama 2
int ObtenerFrecuenciaA2() {
  int frecuenciaA2 = NumPulsosA2;
  NumPulsosA2 = 0;
  return frecuenciaA2;
}

void setup() {
  Serial.begin(9600);

  // Configuración de pines de sensores de flujo
  pinMode(PinSensorA1, INPUT);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(PinSensorA1), ContarPulsosA1, RISING);
  
  pinMode(PinSensorA2, INPUT);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(PinSensorA2), ContarPulsosA2, RISING);
}

void loop() {
  unsigned long currentMillis = millis();
  JsonDocument sensorReading;

  // Cálculo del volumen de agua para la cama 1
  if (riegoActivoA1) {
    if (currentMillis - tiempoUltimoPulsoA1 <= 5000) { // Solo calcular si ha habido pulsos recientes
      float frecuenciaA1 = ObtenerFrecuenciaA1();
      float caudalA1_L_m = frecuenciaA1 / factor_conversionA1;
  
      volumenA1 += (caudalA1_L_m / 60.0); // Volumen incrementado en litros
    } else {
      // Riego detenido, mostrar el volumen y reiniciar
      riegoActivoA1 = false;
      unsigned long tiempoTranscurridoA1 = currentMillis - tiempoInicioRiegoA1; // Tiempo total en milisegundos
      if(volumenA1 >= 0.054568){
      // ENVIO DE INFORMACION DEL FLUJO DE AGUA DE LA CAMA 1
      sensorReading["Sensor"] = "flujoAgua";
      sensorReading["NombreSembrado"] = "Cama tradicional 1";
      sensorReading["CultivoId"] = 3;
      sensorReading["Volumen"] = volumenA1,3;
      sensorReading["TiempoTranscurridoMilis"] = tiempoTranscurridoA1 - 5000; // Tiempo de riego en milisegundos
      serializeJson(sensorReading, Serial);
      sensorReading.clear();
      Serial.println();

     }
     
      volumenA1 = 0; // Reiniciar para la próxima vez
    }
  }

  // Cálculo del volumen de agua para la cama 2
  if (riegoActivoA2) {
    if (currentMillis - tiempoUltimoPulsoA2 <= 5000) { // Solo calcular si ha habido pulsos recientes
      float frecuenciaA2 = ObtenerFrecuenciaA2();
      float caudalA2_L_m = frecuenciaA2 / factor_conversionA2;
      volumenA2 += (caudalA2_L_m / 60.0); // Volumen incrementado en litros
      
    } else {
      // Riego detenido, mostrar el volumen y reiniciar
      riegoActivoA2 = false;
      unsigned long tiempoTranscurridoA2 = currentMillis - tiempoInicioRiegoA2; 
       if (volumenA2 >= 0.054568) { // Verificar si el caudal es mayor a 0.005
      // ENVIO DE INFORMACION DEL FLUJO DE AGUA DE LA CAMA 2
      sensorReading["Sensor"] = "flujoAgua";
      sensorReading["NombreSembrado"] = "Cama tradicional 2";
      sensorReading["CultivoId"] = 4;
      sensorReading["Volumen"] = volumenA2,3;
      sensorReading["TiempoTranscurridoMilis"] = tiempoTranscurridoA2; 
      serializeJson(sensorReading, Serial);
      sensorReading.clear();
      Serial.println();
       
      }
      
      volumenA2 = 0; // Reiniciar para la próxima vez
    }
  }

  delay(1000); // Espera 1 segundo antes de la siguiente lectura
}
