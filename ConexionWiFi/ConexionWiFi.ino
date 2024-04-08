#include <ESP8266WiFi.h>

const char* ssid = "MARTAGONSYTEM";
const char* password = "M@rt4g0nSy5";

void setup() {
  Serial.begin(115200);
  delay(100);

  // Conectar a la red WiFi
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conexión WiFi establecida");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Tu código aquí
}
