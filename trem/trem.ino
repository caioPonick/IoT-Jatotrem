#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFi.h>

WiFiClientSecure client;
PubSubClient mqtt(client);

const String SSID = "hivemq.webclient.1762179657783";
const String PASS = "@L1%hm,nE.63T4RlWYkv";

const int PORT = 1883;
const String URL = "e6607d8d5fec40a9974cfc1552a13e2f.s1.eu.hivemq.cloud";

const String broker_user = "";
const String broker_pass = "";

const String TOPIC_TEMP        = "S1/Temp";
const String TOPIC_UMID        = "S1/Umid";
const String TOPIC_PRESENCA    = "S1/Presenca";
const String TOPIC_ILUMINACAO  = "S1/Iluminacao";

const String TOPIC_CMD         = "S1/Cmd";

const byte LED_AZUL = 2;

void setup() {
  Serial.begin(115200);

  Serial.println("Conectando ao WiFi...");
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nWiFi conectado!");

  client.setInsecure();
  mqtt.setServer(URL.c_str(), PORT);

  Serial.println("Conectando ao broker...");
  while (!mqtt.connected()) {
    String ID = "S1-";
    ID += String(random(0xffff), HEX);
    mqtt.connect(ID.c_str(), broker_user.c_str(), broker_pass.c_str());
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nMQTT conectado!");

  mqtt.subscribe(TOPIC_CMD.c_str());

  mqtt.setCallback(callback);

  pinMode(LED_AZUL, OUTPUT);
}

void loop() {
  mqtt.loop();

  float temperatura = 23.5;
  float umidade     = 45.2;
  int presenca      = 0;  
  String ilum       = "Claro"; 

  mqtt.publish(TOPIC_TEMP.c_str(),        String(temperatura).c_str());
  mqtt.publish(TOPIC_UMID.c_str(),        String(umidade).c_str());
  mqtt.publish(TOPIC_PRESENCA.c_str(),    String(presenca).c_str());
  mqtt.publish(TOPIC_ILUMINACAO.c_str(),  ilum.c_str());

  delay(2000);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem = "";
  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.print("\n[MQTT] ");
  Serial.print(topic);
  Serial.print(" -> ");
  Serial.println(mensagem);

  if (mensagem == "Acender") {
    digitalWrite(LED_AZUL, HIGH);
  }
  else if (mensagem == "Apagar") {
    digitalWrite(LED_AZUL, LOW);
  }
}
