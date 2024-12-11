#include <WebServer.h>
#include <WiFiManager.h>
#include <Deneyap_Servo.h>
#include <math.h>

// Definir los servos
Servo servoIzquierda;
Servo servoDerecha;

// Variables para almacenar el estado actual del control deslizante, joystick y el interruptor
int sliderValue = 0;
int joystickXValue = 0;
int joystickYValue = 0;
int rawX = 0;
int rawY = 0;
bool switchValue = false;
int zona_muerta = 50; // Definir el tamaño de la zona muerta

WebServer server(80);  // Crear un objeto de servidor web que escucha solicitudes HTTP en el puerto 80

// Pines a los que están conectados los servos
const int pinServoIzquierda = 0;
const int pinServoDerecha = 2;

// Ángulos de movimiento
int anguloMin = 95;
int anguloMax = 165;
int anguloCentro = 130;

// Variables para la frecuencia y amplitud
float frecuencia = 1.0; // Frecuencia base
float amplitudIzquierda = 1.0; // Amplitud base para el ala izquierda
float amplitudDerecha = 1.0;   // Amplitud base para el ala derecha


void setup() {
  Serial.begin(115200);

  // Configuración de WiFiManager
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");

  Serial.println("");

  // Esperar a que el Wi-Fi se conecte
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // Adjuntar los servos a los pines correspondientes
  servoIzquierda.attach(pinServoIzquierda);
  servoDerecha.attach(pinServoDerecha,1);

  // Inicializar los servos en la posición central
  servoIzquierda.write(anguloCentro);
  servoDerecha.write(anguloCentro);

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  // Imprimir la IP en el monitor serial

  server.on("/", handleRoot);  // Llamar a la función 'handleRoot' cuando un cliente solicita la URI "/"
  server.on("/slider", handleSlider);  // Llamar a la función 'handleSlider' cuando un cliente solicita la URI "/slider"
  server.on("/joystick", HTTP_GET, handleJoystick);  // Llamar a la función 'handleJoystick' cuando un cliente solicita la URI "/joystick"
  server.on("/switch", handleSwitch);  // Llamar a la función 'handleSwitch' cuando un cliente solicita la URI "/switch"

  server.begin();  // Iniciar el servidor
  Serial.println("HTTP server started");
}

void loop() {
  // Leer los valores del joystick
  int valor_x = rawX;
  int valor_y = rawY;

  // Aplicar la zona muerta
  if (abs(valor_x) < zona_muerta) valor_x = 0;
  if (abs(valor_y) < zona_muerta) valor_y = 0;

  // Controlar la dirección del ornitóptero
  if (valor_y < 0) {
    adelante();
  }
  else if (valor_x > 0) {
    derecha();
  }
  else if (valor_x < 0) {
    izquierda();
  }
  else if (valor_x == 0 && valor_y == 0) {
    parar();
  }

  server.handleClient();  // Manejar una nueva solicitud del cliente


    moverAlas();
}

void moverAlas() {
    static unsigned long startTime = millis(); // Tiempo de inicio

  float elapsedTime = (millis() - startTime) / 1000.0; // Tiempo transcurrido en segundos

  // Movimiento sinusoidal para un período de 1 segundo
  float angleIzquierda = anguloCentro + (anguloMax - anguloCentro) * sin(2 * PI * frecuencia * elapsedTime) * amplitudIzquierda; 
  float angleDerecha = anguloCentro + (anguloMax - anguloCentro) * sin(2 * PI * frecuencia * elapsedTime + PI) * amplitudDerecha;

  servoIzquierda.write(angleIzquierda);
  servoDerecha.write(angleDerecha);
}

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
  html += "<h2>ESP32 C3 Controller</h2>";
  html += "<p>Slider: <input type='range' id='slider' name='slider' min='0' max='255' onchange='fetch(\"/slider?value=\" + this.value)'></p>";
  html += "<div class='joystick' id='joystick' style='width: 200px; height: 200px; background-color: #f0f0f0; border-radius: 50%; position: relative;' onmousemove='updateJoystick(event)'>";
  html += "  <div id='joystickDot' style='width: 20px; height: 20px; background-color: #5B196A; border-radius: 50%; position: absolute; top: " + String(joystickYValue) + "%; left: " + String(joystickXValue) + "%; transform: translate(-50%, -50%);'></div>";
  html += "</div>";
  html += "<p>Switch: <input type='checkbox' id='switch' name='switch' onchange='fetch(\"/switch?value=\" + (this.checked ? 1 : 0))'></p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSlider() {
  sliderValue = server.arg("value").toInt();  // Obtener el valor del control deslizante
  frecuencia = 3.0 + (sliderValue / 255.0) * (6.0 - 3.0); // Mapear el valor del slider a la frecuencia
  server.send(200, "text/plain", "Slider value updated");  // Enviar una respuesta al cliente
}

void handleJoystick() {
  rawX = server.arg("x").toInt();  // Obtener el valor X del joystick
  rawY = server.arg("y").toInt();  // Obtener el valor Y del joystick
  server.send(200, "text/plain", "Joystick value updated");  // Enviar una respuesta al cliente
}

void handleSwitch() {
  switchValue = server.arg("value").toInt() == 1;  // Obtener el valor del interruptor
  server.send(200, "text/plain", "Switch value updated");  // Enviar una respuesta al cliente
}

void adelante() {
  amplitudIzquierda = 1.0;   // Amplitud base para vuelo recto
  amplitudDerecha = 1.0;     // Amplitud base para vuelo recto
}

void derecha() {
  amplitudIzquierda = 1.0;   // Mantener amplitud para el ala izquierda
  amplitudDerecha = 0.6;     // Reducir amplitud para el ala derecha
}

void izquierda() {
  amplitudIzquierda = 0.6;   // Reducir amplitud para el ala izquierda
  amplitudDerecha = 1.0;     // Mantener amplitud para el ala derecha
}

void parar() {
  amplitudIzquierda = 0;   // Detener el aleteo del ala izquierda
  amplitudDerecha = 0;     // Detener el aleteo del ala derecha
}
