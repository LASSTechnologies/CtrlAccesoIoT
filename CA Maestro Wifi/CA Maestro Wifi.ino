#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Keypad.h>
#include "BluetoothSerial.h"

#include "funciones.hpp"
#include "network.hpp"
#include "APIRest.hpp"

const int SERVO_PIN = 14;
const int puertaPin = 32;
int luzPin1 = 27;
int luzPin2 = 26;
int encendidoLed = 13;
int wifiLed = 35;
int blueLed = 34;
int contador = 0;

#define USE_NAME           // Comment this to use MAC address instead of a slaveName
const char *pin = "1234";  // Change this to reflect the pin expected by the real slave BT device

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;

#ifdef USE_NAME
String slaveName = "Esclavo-Alarma";  // Change this to reflect the real name of your slave BT device
#else
String MACadd = "AA:BB:CC:11:22:33";                          // This only for printing
uint8_t address[6] = { 0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33 };  // Change this to reflect real MAC address of your slave BT device
#endif

String myName = "Main-Maestro";

const byte ROWS = 4;  //four rows
const byte COLS = 4;  //three columns
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 23, 22, 21, 19 };  //connect to the row pinouts of the keypad
byte colPins[COLS] = { 18, 5, 4, 2 };     //connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void teclado() {
  char key = keypad.getKey();

  if (key) {
    Serial.println(key);

    switch (key) {
      case '1':
        abrirCochera();
        break;
      case '2':
        cerrarCochera();
        break;
      case '3':
        abrirPuerta();
        break;
      case '4':
        activarAlarma();
        break;
      case '5':
        desactivarAlarma();
        break;
      case '6':
        encenderLuces();
        break;
      case '7':
        apagarLuces();
        break;
      default:
        Serial.println("No se ha leido teclado ");
        break;
    }
  }
}

void abrirPuerta() {
  Serial.println("Puerta ");
  digitalWrite(puertaPin, HIGH);
  delay(3000);
  digitalWrite(puertaPin, LOW);
}

void activarAlarma() {
  Serial.println("Alarma activada");
  String activar = "aa";
  SerialBT.println(activar);
}

void desactivarAlarma() {
  Serial.println("Alarma desactivada");
  String desactivar = "dd";
  SerialBT.println(desactivar);
}

void encenderLuces() {
  digitalWrite(luzPin1, HIGH);
  digitalWrite(luzPin2, HIGH);
}

void apagarLuces() {
  digitalWrite(luzPin1, LOW);
  digitalWrite(luzPin2, LOW);
}

void setup() {
  Serial.begin(115200);
  servoMotor.attach(SERVO_PIN);  // attaches the servo on ESP32 pin

  bool connected;

  pinMode(puertaPin, OUTPUT);
  pinMode(luzPin1, OUTPUT);
  pinMode(luzPin2, OUTPUT);
  pinMode(encendidoLed, OUTPUT);
  pinMode(wifiLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  digitalWrite(encendidoLed, HIGH);

  conectarWiFi();
  digitalWrite(wifiLed, HIGH);

  SerialBT.begin(myName, true);
  Serial.printf("The device \"%s\" started in master mode, make sure slave BT device is on!\n", myName.c_str());

#ifndef USE_NAME
  SerialBT.setPin(pin);
  Serial.println("Using PIN");
#endif

// connect(address) is fast (up to 10 secs max), connect(slaveName) is slow (up to 30 secs max) as it needs
// to resolve slaveName to address first, but it allows to connect to different devices with the same name.
// Set CoreDebugLevel to Info to view devices Bluetooth address and device names
#ifdef USE_NAME
  connected = SerialBT.connect(slaveName);
  Serial.printf("Conectando con: \"%s\"\n", slaveName.c_str());
#else
  connected = SerialBT.connect(address);
  Serial.print("Connecting to slave BT device with MAC ");
  Serial.println(MACadd);
#endif

  SerialBT.connect();
  if (connected) {
    Serial.println("Conexión Exitosa!");
    digitalWrite(blueLed, HIGH);

  } else {
    while (!SerialBT.connected(10000)) {
      Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app.");
    }
  }
}

void loop() {
  WiFiClient client = server.available();
  teclado();
  if (contador == 50) {
    contador = 0;
  }
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    String message = SerialBT.readString();
    Serial.println("Mensaje recibido: " + message);
    message.trim();
    if (contador == 0) {
      Create(message);
    }
  }
  teclado();

  if (client) {
    Serial.println("Cliente nuevo .");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        if (currentLine.endsWith("GET /cochera&accion=abrir") || currentLine.endsWith("GET /cochera&accion=cerrar")) {
          if (currentLine.endsWith("GET /cochera&accion=abrir")) {
            abrirCochera();
          }
          if (currentLine.endsWith("GET /cochera&accion=cerrar")) {
            cerrarCochera();
          }
        } else if (currentLine.endsWith("GET /puerta&accion=abrir")) {
          abrirPuerta();
        } else if (currentLine.endsWith("GET /alarma&accion=activar") || currentLine.endsWith("GET /alarma&accion=desactivar")) {
          if (currentLine.endsWith("GET /alarma&accion=activar")) {
            activarAlarma();
          }
          if (currentLine.endsWith("GET /alarma&accion=desactivar")) {
            desactivarAlarma();
          }
        } else if (currentLine.endsWith("GET /iluminacion&accion=encender") || currentLine.endsWith("GET /iluminacion&accion=apagar")) {
          if (currentLine.endsWith("GET /iluminacion&accion=encender")) {
            encenderLuces();
          }
          if (currentLine.endsWith("GET /iluminacion&accion=apagar")) {
            apagarLuces();
          }
        }
      }
    }
    client.stop();
    Serial.println("Cliente desconectado.");
  }
  teclado();
  contador++;

}
