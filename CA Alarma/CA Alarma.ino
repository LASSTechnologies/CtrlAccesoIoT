#include "BluetoothSerial.h"
#include "SPI.h"
#include "SD.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

int pirPin = 27;  //Pin del sensor PIR
int pirPin02 = 33;
int ledPin01 = 26;  //Led parpadeo de acciones
int ledPin02 = 25;
int buzzPin = 32;
bool estado = false;
const int buttonPinA = 34;
const int buttonPinD = 35;
int chipSelect = 5;

File datosCSV;

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);

  pinMode(pirPin, INPUT);
  pinMode(pirPin02, INPUT);
  pinMode(ledPin01, OUTPUT);
  pinMode(ledPin02, OUTPUT);
  pinMode(buzzPin, OUTPUT);
  pinMode(buttonPinA, INPUT_PULLUP);
  pinMode(buttonPinD, INPUT_PULLUP);

  SerialBT.begin("Esclavo-Alarma");
  Serial.println("The device started, now you can pair it with bluetooth!");

  // Inicializar el módulo SD
  if (!SD.begin(chipSelect)) {
    Serial.println("Error al inicializar el módulo SD");
    return;
  }
  Serial.println("Módulo SD inicializado correctamente");
}

void apagarTodo() {
  digitalWrite(ledPin01, LOW);
  digitalWrite(ledPin02, LOW);
  digitalWrite(buzzPin, LOW);
  Serial.println("estado desactivado");
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }

  bool onButtonState = digitalRead(buttonPinA) == LOW;
  bool offButtonState = digitalRead(buttonPinD) == LOW;

  int movimiento01 = digitalRead(pirPin);
  int movimiento02 = digitalRead(pirPin02);

  unsigned long timestamp = millis();

  if (onButtonState) {
    estado = true;
  } else if (offButtonState) {
    estado = false;
    apagarTodo();
  }

  //validar mensaje de activacion de alarma
  if (SerialBT.available()) {
    String message = SerialBT.readString();  // Leer mensaje Bluetooth
    Serial.println(message);
    message.trim();        // Imprimir mensaje recibido en Monitor Serial
    if (message == "a") {  //Si el mensaje es "alarma"
      estado = true;
      Serial.println("estado cambiado");
    }
    if (message == "d") {  //Si el mensaje es "alarma"
      estado = false;
      apagarTodo();
    }
  }

  //ALARMA
  if (estado) {
    Serial.println("Alarma activada!");

    // Detector de Movimiento Sensor 1
    if (digitalRead(pirPin) == HIGH) {                   //Si se detecta movimiento
      SerialBT.println("Movimiento Detectado,Entrada");  //Enviar mensaje a través de Bluetooth

      // Escribir los datos en el archivo CSV
      datosCSV = SD.open("/datosSensor.csv", FILE_APPEND);
      if (datosCSV) {
        if (movimiento01 == HIGH) {
          datosCSV.print(timestamp);
          datosCSV.print(",");
          datosCSV.println("Cochera: Movimiento detectado!");
          Serial.println("Datos guardados");
        }
        datosCSV.close();
      } else {
        Serial.println("Error al abrir el archivo CSV");
      }

      digitalWrite(ledPin01, HIGH);
      digitalWrite(buzzPin, HIGH);
      delay(1000);  // wait for 500 milliseconds
    }

    // Detector de Movimiento Sensor 1
    if (digitalRead(pirPin02) == HIGH) {               //Si se detecta movimiento
      SerialBT.println("Movimiento Detectado,Patio");  //Enviar mensaje a través de Bluetooth
      Serial.println("sensor2 activada!");

      // Escribir los datos en el archivo CSV - SENSOR 2
      datosCSV = SD.open("/datosSensor.csv", FILE_APPEND);
      if (datosCSV) {
        if (movimiento02 == HIGH) {
          datosCSV.print(timestamp);
          datosCSV.print(",");
          datosCSV.println("Patio: Movimiento detectado!");
          Serial.println("Datos guardados");
        }
        datosCSV.close();
      } else {
        Serial.println("Error al abrir el archivo CSV");
      }

      digitalWrite(ledPin02, HIGH);
      digitalWrite(buzzPin, HIGH);
      delay(1000);  // wait for 500 milliseconds
    }
  }

  delay(1000);
}
