#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* ssid = "peridot";
const char* password = "logdate7152";

WebServer server(80);
Servo myServo;
const int servoPin = 26;

bool isNumeric(String str) {
  if (str.length() == 0) return false;
  for (unsigned int i = 0; i < str.length(); i++) {
    if (!isDigit(str.charAt(i))) return false;
  }
  return true;
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Kontrol Servo ESP32</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; }
    input[type=range] { width: 80%; max-width: 350px; }
  </style>
</head>
<body>
  <h2>Kendali Servo ESP32</h2>
  <p>Sudut: <span id="posVal">90</span>&deg;</p>
  <input type="range" min="0" max="180" value="90" oninput="document.getElementById('posVal').innerText=this.value" onchange="updateServo(this.value)">
  <p id="statusMsg"></p>

  <script>
    function updateServo(val) {
      document.getElementById('posVal').innerText = val;
      fetch('/servo?pos=' + val)
        .then(r => r.text().then(t => ({ status: r.status, body: t })))
        .then(x => document.getElementById('statusMsg').innerText = 'Status ' + x.status + ': ' + x.body)
        .catch(e => document.getElementById('statusMsg').innerText = 'Gagal menghubungi ESP32');
    }
  </script>
</body>
</html>
)rawliteral";

void handleServo() {
  if (!server.hasArg("pos") || !isNumeric(server.arg("pos"))) {
    server.send(400, "text/plain", "Input tidak valid (harus angka 0-180)");
    return;
  }
  
  int pos = server.arg("pos").toInt();
  if (pos < 0 || pos > 180) {
    server.send(400, "text/plain", "Sudut di luar batas (0-180)");
    return;
  }
  
  myServo.write(pos);
  server.send(200, "text/plain", "Servo bergerak ke " + String(pos) + " derajat");
}

void setup() {
  Serial.begin(115200);
  
  myServo.setPeriodHertz(50);
  myServo.attach(servoPin, 500, 2400);
  myServo.write(90);

  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  
  Serial.println("\nTerhubung! IP Address: " + WiFi.localIP().toString());

  server.on("/", []() {
    server.send(200, "text/html", index_html);
  });
  server.on("/servo", handleServo);
  
  server.begin();
}

void loop() {
  server.handleClient();
}
