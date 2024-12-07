

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


// Ángulos de movimiento
int anguloMin = 95;
int anguloMax = 165;
int anguloCentro = 130;

// Variables para la frecuencia y amplitud
float frecuencia = 1.0; // Frecuencia base
float amplitudIzquierda = 2.0; // Amplitud base para el ala izquierda
float amplitudDerecha = 2.0;   // Amplitud base para el ala derecha



void setup() {
  Serial.begin(115200);

  // Configurar modo Access Point
  WiFi.softAP("1flapping", "12345678"); // Nombre y contraseña del AP
  Serial.println("Access Point iniciado");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  // Configurar los servos
  Dservo.attach(0);  // Servo derecho en pin 12
  Iservo.attach(2);  // Servo izquierdo en pin 13

  // Inicializar los servos en la posición central
  Dservo.write(anguloCentro);
  Iservo.write(anguloCentro);

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

   moverAlas();
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
  html += "<h2>aero robot</h2>";
  html += "<p>Velocidad: <input type='range' id='slider' name='slider' min='0' max='180' onchange='fetch(\"/slider?value=\" + this.value)'></p>";
  html += "<div id='joystick' style='width: 200px; height: 200px; background-color: #f0f0f0; border-radius: 50%; position: relative;' onmousemove='updateJoystick(event)'>";
  html += "  <div id='joystickDot' style='width: 20px; height: 20px; background-color: #5B196A; border-radius: 50%; position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%);'></div>";
  html += "</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void moverAlas() {
    static unsigned long startTime = millis(); // Tiempo de inicio

  float elapsedTime = (millis() - startTime) / 1000.0; // Tiempo transcurrido en segundos

  // Movimiento sinusoidal para un período de 1 segundo
  float angleIzquierda = anguloCentro + (anguloMax - anguloCentro) * sin(2 * PI * frecuencia * elapsedTime) * amplitudIzquierda; 
  float angleDerecha = anguloCentro + (anguloMax - anguloCentro) * sin(2 * PI * frecuencia * elapsedTime + PI) * amplitudDerecha;

  Dservo.write(angleIzquierda);
  Iservo.write(angleDerecha);
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
  amplitudIzquierda = 1.0;   // Amplitud base para vuelo recto
  amplitudDerecha = 1.0;     // Amplitud base para vuelo recto
}

void reverse() {

}

void right() {
  amplitudIzquierda = 1.0;   // Mantener amplitud para el ala izquierda
  amplitudDerecha = 0.6;     // Reducir amplitud para el ala derecha
}

void left() {
  amplitudIzquierda = 0.6;   // Reducir amplitud para el ala izquierda
  amplitudDerecha = 1.0;     // Mantener amplitud para el ala derecha
}

void stopMotors() {
  amplitudIzquierda = 0;   // Detener el aleteo del ala izquierda
  amplitudDerecha = 0;     // Detener el aleteo del ala derecha
}
