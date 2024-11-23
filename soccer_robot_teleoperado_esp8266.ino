/*

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <Servo.h>


Servo Dservo;  //creamos un objeto servo 
Servo Iservo;

// Variables
unsigned long previousMillis = 0;
const long interval = 100;
int sliderValue = 0;
int joystickXValue = 0;
int joystickYValue = 0;
bool switchValue = false;
bool lastSwitchState = false;
int rawX = 0;
int rawY = 0;
int deadZone = 40; // Defines the dead zone size

// Create a Web Server instance
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  // Configure WiFiManager
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");

  Serial.println("Waiting for Wi-Fi connection...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  Dservo.attach(12);  // asignamos el pin 3 al servo.
  Iservo.attach(13);  // asignamos el pin 3 al servo.

  
  // Set up routes
  server.on("/", handleRoot);
  server.on("/slider", handleSlider);
  server.on("/joystick", HTTP_GET, handleJoystick);
  server.on("/switch", handleSwitch);

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  int valueX = rawX;
  int valueY = rawY;

  // Apply dead zone
  if (abs(valueX) < deadZone) valueX = 0;
  if (abs(valueY) < deadZone) valueY = 0;

  // Control motors based on joystick position
  if (valueY < 0) {
    forward();
  } else if (valueX > 0) {
    right();
  } else if (valueX < 0) {
    left();
  } else if (valueX == 0 && valueY == 0) {
    stopMotors();
  }

  

  // Handle incoming client requests
  server.handleClient();
}

// Handlers for different routes

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><style>body { display: flex; justify-content: center; align-items: center; height: 100vh; flex-direction: column; }</style><script>";
  html += "function updateJoystick(event) {";
  html += "  var joystick = document.getElementById('joystick');";
  html += "  var joystickDot = document.getElementById('joystickDot');";
  html += "  var rect = joystick.getBoundingClientRect();";
  html += "  var x = event.clientX - rect.left;";
  html += "  var y = event.clientY - rect.top;";
  html += "  var radius = joystick.clientWidth / 2;";
  html += "  var angle = Math.atan2(y - radius, x - radius);";
  html += "  var distance = Math.min(radius, Math.hypot(x - radius, y - radius));";
  html += "  var newX = radius + distance * Math.cos(angle);";
  html += "  var newY = radius + distance * Math.sin(angle);";
  html += "  joystickDot.style.left = newX + 'px';";
  html += "  joystickDot.style.top = newY + 'px';";
  html += "  var normalizedX = (newX - radius) / radius * 100;";
  html += "  var normalizedY = (newY - radius) / radius * 100;";
  html += "  fetch('/joystick?x=' + normalizedX + '&y=' + normalizedY);";
  html += "}";
  html += "</script></head><body>";
  html += "<h2>Soccer Controller</h2>";
  html += "<p>Slider: <input type='range' id='slider' name='slider' min='0' max='255' onchange='fetch(\"/slider?value=\" + this.value)'></p>";
  html += "<div class='joystick' id='joystick' style='width: 200px; height: 200px; background-color: #f0f0f0; border-radius: 50%; position: relative;' onmousemove='updateJoystick(event)'>";
  html += "  <div id='joystickDot' style='width: 20px; height: 20px; background-color: #5B196A; border-radius: 50%; position: absolute; top: " + String(joystickYValue) + "%; left: " + String(joystickXValue) + "%; transform: translate(-50%, -50%);'></div>";
  html += "</div>";
  html += "<p>Switch: <input type='checkbox' id='switch' name='switch' onchange='fetch(\"/switch?value=\" + (this.checked ? 1 : 0))'></p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSlider() {
  sliderValue = server.arg("value").toInt();  // Get the value of the slider
  server.send(200, "text/plain", "Slider value updated");  // Send a response to the client
}

void handleJoystick() {
  rawX = server.arg("x").toInt();  // Get the raw X value of the joystick
  rawY = server.arg("y").toInt();  // Get the raw Y value of the joystick
  server.send(200, "text/plain", "Joystick value updated");  // Send a response to the client
}

void handleSwitch() {
  switchValue = server.arg("value").toInt() == 1;  // Get the value of the switch
  server.send(200, "text/plain", "Switch value updated");  // Send a response to the client
}

// Motor control functions

void forward() {
  Iservo.write(150);
  Dservo.write(150);


}

void right() {
  Iservo.write(30);
  Dservo.write(150);
}

void left() {
  
  Iservo.write(150);
  Dservo.write(30);
}

void stopMotors() {
  Iservo.write(90);
  Dservo.write(90);
}








*/






//codigo con velocidad en slider y ap point esp12e
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

Servo Dservo;  // Servo derecho
Servo Iservo;  // Servo izquierdo

// Variables
unsigned long previousMillis = 0;
const long interval = 100;
int sliderValue = 0; // Velocidad
int joystickXValue = 0;
int joystickYValue = 0;
int deadZone = 40; // Zona muerta para el joystick

// Crear instancia del servidor web
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  // Configurar modo Access Point
  WiFi.softAP("1ESP_SOCCER", "12345678"); // Nombre y contraseña del AP
  Serial.println("Access Point iniciado");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  // Configurar los servos
  Dservo.attach(12);  // Servo derecho en pin 12
  Iservo.attach(13);  // Servo izquierdo en pin 13

  // Rutas del servidor
  server.on("/", handleRoot);
  server.on("/slider", handleSlider);
  server.on("/joystick", HTTP_GET, handleJoystick);

  // Iniciar servidor
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  // Aplicar zona muerta al joystick
  int valueX = joystickXValue;
  int valueY = joystickYValue;
  if (abs(valueX) < deadZone) valueX = 0;
  if (abs(valueY) < deadZone) valueY = 0;

  // Controlar dirección y velocidad de los servos
  if (valueY > 0) {          // Hacia adelante
    forward();
  } else if (valueY < 0) {   // Hacia atrás
    reverse();
  } else if (valueX > 0) {   // Girar a la derecha
    right();
  } else if (valueX < 0) {   // Girar a la izquierda
    left();
  } else {                   // Detenerse
    stopMotors();
  }

  // Manejar solicitudes del servidor
  server.handleClient();
}

// Ruta principal
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><style>body { display: flex; justify-content: center; align-items: center; height: 100vh; flex-direction: column; }</style><script>";
  html += "function updateJoystick(event) {";
  html += "  var joystick = document.getElementById('joystick');";
  html += "  var joystickDot = document.getElementById('joystickDot');";
  html += "  var rect = joystick.getBoundingClientRect();";
  html += "  var x = event.clientX - rect.left;";
  html += "  var y = event.clientY - rect.top;";
  html += "  var radius = joystick.clientWidth / 2;";
  html += "  var angle = Math.atan2(y - radius, x - radius);";
  html += "  var distance = Math.min(radius, Math.hypot(x - radius, y - radius));";
  html += "  var newX = radius + distance * Math.cos(angle);";
  html += "  var newY = radius + distance * Math.sin(angle);";
  html += "  joystickDot.style.left = newX + 'px';";
  html += "  joystickDot.style.top = newY + 'px';";
  html += "  var normalizedX = ((newX - radius) / radius * 100).toFixed(0);";
  html += "  var normalizedY = ((newY - radius) / radius * 100).toFixed(0);";
  html += "  fetch('/joystick?x=' + normalizedX + '&y=' + normalizedY);";
  html += "}";
  html += "</script></head><body>";
  html += "<h2>Soccer robot</h2>";
  html += "<p>Velocidad: <input type='range' id='slider' name='slider' min='0' max='180' onchange='fetch(\"/slider?value=\" + this.value)'></p>";
  html += "<div id='joystick' style='width: 200px; height: 200px; background-color: #f0f0f0; border-radius: 50%; position: relative;' onmousemove='updateJoystick(event)'>";
  html += "  <div id='joystickDot' style='width: 20px; height: 20px; background-color: #5B196A; border-radius: 50%; position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%);'></div>";
  html += "</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Manejar deslizador
void handleSlider() {
  sliderValue = server.arg("value").toInt();
  server.send(200, "text/plain", "Valor de velocidad actualizado");
}

// Manejar joystick
void handleJoystick() {
  joystickXValue = server.arg("x").toInt();
  joystickYValue = server.arg("y").toInt();
  server.send(200, "text/plain", "Valores del joystick actualizados");
}

// Funciones de control de motores
void forward() {
  Iservo.write(90 + sliderValue); // Servo izquierdo avanza
  Dservo.write(90 - sliderValue); // Servo derecho avanza
}

void reverse() {
  Iservo.write(90 - sliderValue); // Servo izquierdo retrocede
  Dservo.write(90 + sliderValue); // Servo derecho retrocede
}

void right() {
  Iservo.write(90 + sliderValue); // Servo izquierdo avanza
  Dservo.write(90);               // Servo derecho se detiene
}

void left() {
  Iservo.write(90);               // Servo izquierdo se detiene
  Dservo.write(90 - sliderValue); // Servo derecho avanza
}

void stopMotors() {
  Iservo.write(90); // Detener servo izquierdo
  Dservo.write(90); // Detener servo derecho
}
