#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// WiFi AP SSID
//#define WIFI_SSID "MARTAGONSYTEM"
#define WIFI_SSID "INFINITUMFFBA"
// WiFi password
//#define WIFI_PASSWORD "M@rt4g0nSy5"
#define WIFI_PASSWORD "Haz2g8Uh4t"

// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
// InfluxDB 1.8+  (v2 compatibility API) server url, e.g. http://192.168.1.48:8086
//#define INFLUXDB_URL "http://192.168.0.102:8888"
#define INFLUXDB_URL "http://192.168.1.87:8888"
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)
// InfluxDB 1.8+ (v2 compatibility API) use form user:password, eg. admin:adminpass
#define INFLUXDB_TOKEN "ubuntu:ubuntu"
// InfluxDB v2 organization name or id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )
// InfluxDB 1.8+ (v2 compatibility API) use any non empty string
#define INFLUXDB_ORG "org upv"
// InfluxDB v2 bucket name (Use: InfluxDB UI -> Load Data -> Buckets)
// InfluxDB 1.8+ (v2 compatibility API) use database name
#define INFLUXDB_BUCKET "ntop"

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

void setup() {
  Serial.begin(115200);

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  // Accurate time is necessary for certificate validation
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
  // Construct a Flux query
  // Query will list RSSI for last 24 hours for each connected WiFi network of this device type
  String query = "from(bucket: \"" INFLUXDB_BUCKET "\") |> range(start: -24h) |> filter(fn: (r) => r._measurement == \"host:traffic\") |> limit(n: 10)";

  Serial.println("==== List results ====");
  
  // Print composed query
  Serial.print("Querying with: ");
  Serial.println(query);

  // Send query to the server and get result
  FluxQueryResult result = client.query(query);

  // Process the result
  processResult(result);

  //Wait 10s
  Serial.println("Wait 10s");
  delay(10000);
}

void processResult(FluxQueryResult result) {
  String ipWithMaxBytes;
  long maxBytes = 0;

  Serial.println("IP\t\tBytes Received\tBytes Sent");

  // Itera sobre los resultados
  while (result.next()) {
    // Obtiene los valores de la fila actual
    long bytesReceived = result.getValueByName("_value").getRawValue().toInt();
    long bytesSent = result.getValueByName("_value").getRawValue().toInt();

    String ip = result.getValueByName("host").getRawValue();

    // Imprime los valores de la fila
    Serial.print(ip);
    Serial.print("\t\t");
    Serial.print(bytesReceived);
    Serial.print("\t\t");
    Serial.println(bytesSent);

    // Compara los valores de bytes y actualiza la IP con la mayor cantidad de bytes
    if (bytesReceived > maxBytes) {
      maxBytes = bytesReceived;
      ipWithMaxBytes = ip;
    }
    if (bytesSent > maxBytes) {
      maxBytes = bytesSent;
      ipWithMaxBytes = ip;
    }
  }

  // Imprime la IP con la mayor cantidad de bytes
  Serial.print("IP with max bytes: ");
  Serial.println(ipWithMaxBytes);
}

