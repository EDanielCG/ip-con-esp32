#include <WiFi.h>

// Credenciales de la red WiFi
const char *ssid = "INFINITUM35E4";      
const char *password = "Bx5Ff6Pm2s";

// Definir los pines de los LEDs y el ADC
const int ledPin1 = 26;
const int ledPin2 = 27;
const int adcPin = 34;
const int controlPin = 25;

// Crear un servidor web en el puerto 80
WiFiServer server(80);

// Variables para almacenar el valor del ADC y su conversión a voltios
int adcValue = 0;
float voltajeAjustado = 0.0;
const float UMBRAL_VOLTAGE = 1.0;

// Dirección IP estática
IPAddress staticIP(192, 168, 1, 184);

String htmlPage = R"rawliteral(
HTTP/1.1 200 OK
Content-type:text/html

<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Controlador de LEDs y ADC</title>
  <style>
    body { text-align: center; font-family: Arial, sans-serif; }
    button { background-color: slateblue; color: white; border: none; width: 80px; height: 40px; margin: 5px; }
    button:hover { background-color: darkslateblue; }
    h1 { color: slateblue; }
  </style>
  <script>
    function controlLed(led, state) {
      fetch(`/${led}/${state}`).then(() => location.reload());
    }
    function readADC() {
      fetch(`/read_adc`).then(response => response.text()).then(data => {
        document.getElementById("adcValue").innerText = data + " V";
      });
    }
  </script>
</head>
<body>
  <h1>ESP32 Control de LEDs y ADC</h1>
  <p>LED 1:
    <button onclick="controlLed('led1', 'on')">ON</button>
    <button onclick="controlLed('led1', 'off')">OFF</button>
  </p>
  <p>LED 2:
    <button onclick="controlLed('led2', 'on')">ON</button>
    <button onclick="controlLed('led2', 'off')">OFF</button>
  </p>
  <p>
    <button onclick="readADC()">Leer ADC</button>
  </p>
  <p>Voltaje actual: <span id="adcValue">%ADC_VALUE% V</span></p>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(controlPin, INPUT);

  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);

  // Conectar a WiFi con IP dinámica
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a WiFi. Dirección IP dinámica:");
  Serial.println(WiFi.localIP());

  // Mostrar IP estática en el monitor serial
  Serial.print("Dirección IP Estática configurada: ");
  Serial.println(staticIP);

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("Cliente conectado");
    String currentLine = "";
    String responseHtml = htmlPage;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        currentLine += c;

        if (c == '\n') {
          if (currentLine.length() == 0) {
            adcValue = analogRead(adcPin);
            voltajeAjustado = (adcValue / 4095.0) * 5.0;
            responseHtml.replace("%ADC_VALUE%", String(voltajeAjustado, 2));

            client.print(responseHtml);
            break;
          } else {
            currentLine = "";
          }
        }

        if (currentLine.endsWith("GET /led1/on")) {
          digitalWrite(ledPin1, HIGH);
        }
        if (currentLine.endsWith("GET /led1/off")) {
          digitalWrite(ledPin1, LOW);
        }
        if (currentLine.endsWith("GET /led2/on")) {
          digitalWrite(ledPin2, HIGH);
        }
        if (currentLine.endsWith("GET /led2/off")) {
          digitalWrite(ledPin2, LOW);
        }
        if (currentLine.endsWith("GET /read_adc")) {
          client.print(String(voltajeAjustado, 2));
          break;
        }
      }
    }

    client.stop();
    Serial.println("Cliente desconectado");
  }
}


