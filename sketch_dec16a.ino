#include <WiFi.h>
#include <PubSubClient.h>

// ---- WIFI ----
const char* ssid = "M15-F962";
const char* password = "bvaya36348";

// ---- THINGSBOARD ----
const char* mqttServer = "eu.thingsboard.cloud";
const int mqttPort = 1883;
const char* accessToken = "20jKliBuVBSY6qCwfreP";

WiFiClient espClient;
PubSubClient client(espClient);

// ---- Simulering av sensor ---- 
long simulatedDistance() {
  return random(5, 120); // slumpvärde mellan 5–120 cm
}

String getStatusText(long d) {
  return (d < 30) ? "occupied" : "free";
}

// ---- MQTT Connect ----
void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to ThingsBoard...");
    if (client.connect("ESP32_SIMULATED", accessToken, NULL)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" - trying again in 2 seconds");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n=== Parking Sensor ESP32 ===");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer, mqttPort);
  Serial.print("ThingsBoard server: ");
  Serial.println(mqttServer);
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }

  client.loop();

  long distance = simulatedDistance();
  String status_text = getStatusText(distance);

  // Försäkra att alla värden skickas som strängar, ThingsBoard accepterar det dynamiskt
  String payload = "{\"distance\":\"" + String(distance) + "\"" +
                   ", \"status\":\"" + status_text + "\"" +
                   ", \"status_num\":\"" + String((distance < 30) ? 0 : 1) + "\"}";

  Serial.println("\n--- Sending Data ---");
  Serial.println("Distance: " + String(distance) + " cm");
  Serial.println("Status: " + status_text);
  Serial.println("Full payload: " + payload);

  // Skicka telemetri till ThingsBoard
  if (client.publish("v1/devices/me/telemetry", payload.c_str())) {
    Serial.println("Telemetry sent successfully!");
  } else {
    Serial.println("Telemetry failed!");
  }

  delay(5000); // skicka var 5:e sekund
}
