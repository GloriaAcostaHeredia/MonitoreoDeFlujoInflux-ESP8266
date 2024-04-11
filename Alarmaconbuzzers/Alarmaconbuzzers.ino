#include "pitches.h"
#include <ESP8266WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// Definir el pin del buzzer
#define BUZZER_PIN D2

// Definir la frecuencia de la alarma
#define ALARM_FREQUENCY NOTE_C4

// Definir umbral de bytes para activar la alarma
#define ALARM_THRESHOLD 10

// WiFi AP SSID y contraseña
#define WIFI_SSID "MARTAGONSYTEM"
#define WIFI_PASSWORD "M@rt4g0nSy5"

// InfluxDB configuración
#define INFLUXDB_URL "http://192.168.0.102:8888"
#define INFLUXDB_TOKEN "ubuntu:ubuntu"
#define INFLUXDB_ORG "org upv"
#define INFLUXDB_BUCKET "ntop"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// Instancia del cliente InfluxDB
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);  // Configurar el pin del buzzer como salida

  // Setup WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("WiFi connected");

  // Sincronizar tiempo
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Verificar conexión al servidor InfluxDB
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
  // Construir la consulta Flux
  String query = "from(bucket: \"" INFLUXDB_BUCKET "\") |> range(start: -24h) |> filter(fn: (r) => r._measurement == \"host:traffic\") |> limit(n: 10)";

  Serial.println("==== List results ====");
  
  // Imprimir la consulta
  Serial.print("Querying with: ");
  Serial.println(query);

  // Enviar la consulta al servidor y obtener el resultado
  FluxQueryResult result = client.query(query);

  // Procesar el resultado
  long bytesReceived = processResult(result);

  // Procesar datos y activar/desactivar la alarma según el umbral
  procesarDatos(bytesReceived);

  // Esperar 10 segundos
  delay(10000);
}

long processResult(FluxQueryResult result) {
  long totalBytes = 0;

  Serial.println("IP\t\tBytes Received\tBytes Sent");

  // Iterar sobre los resultados
  while (result.next()) {
    // Obtener los valores de la fila actual
    long bytesReceived = result.getValueByName("_value").getRawValue().toInt();

    String ip = result.getValueByName("host").getRawValue();

    // Imprimir los valores de la fila
    Serial.print(ip);
    Serial.print("\t\t");
    Serial.println(bytesReceived);

    // Sumar los bytes recibidos al total
    totalBytes += bytesReceived;
  }

  return totalBytes;
}

void procesarDatos(long bytesRecibidos) {
  // Verificar si la cantidad de bytes recibidos alcanza el umbral para activar la alarma
  if (bytesRecibidos >= ALARM_THRESHOLD) {
    // Activar la alarma
    tone(BUZZER_PIN, ALARM_FREQUENCY);
  } else {
    // Desactivar la alarma
    noTone(BUZZER_PIN);
  }
}
