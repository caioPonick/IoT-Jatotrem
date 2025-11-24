#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ESP32Servo.h>

#define PINO_LED 2
#define TRIG 26
#define ECHO 25
#define PINO_SERVO3 19
#define PINO_SERVO4 18
#define PINO_PRESENCA 14

WiFiClientSecure client;
PubSubClient mqtt(client);
Servo servo3;
Servo servo4;

const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

const char* BROKER_URL  = "e6607d8d5fec40a9974cfc1552a13e2f.s1.eu.hivemq.cloud";
const int   BROKER_PORT = 8883;

const char* BROKER_USER = "vitor_bucci";
const char* BROKER_PASS = "Integrante1";

// PUBLICAÇÃO S3
const char* TOPICO_PUB_DIST3 = "Projeto/S3/Presenca3";
const char* TOPICO_PUB_ULTRA = "Projeto/S3/Ultrassom3";

// ASSINATURA DA S2
const char* TOPICO_SUB_DIST1 = "Projeto/S2/Distancia1";
const char* TOPICO_SUB_DIST2 = "Projeto/S2/Distancia2";

// ILUMINAÇÃO (S1)
const char* TOPICO_ILUMINACAO = "S1/iluminacao";

unsigned long lastPublish = 0;
int publishInterval = 3000;

long medirDistancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 30000);
  long distancia = (duracao * 0.034) / 2;
  return distancia;
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem;

  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  // ILUMINAÇÃO
  if (String(topic) == TOPICO_ILUMINACAO) {
    if (mensagem == "acender") digitalWrite(PINO_LED, HIGH);
    else if (mensagem == "apagar") digitalWrite(PINO_LED, LOW);
  }

  // CONTROLE DOS SERVOS A PARTIR DA S2
  else if (String(topic) == TOPICO_SUB_DIST1) {
    if (mensagem == "objeto_proximo") servo3.write(90);
    else if (mensagem == "objeto_longe") servo3.write(45);
  }

  else if (String(topic) == TOPICO_SUB_DIST2) {
    if (mensagem == "objeto_proximo") servo4.write(90);
    else if (mensagem == "objeto_longe") servo4.write(45);
  }

  Serial.print("Mensagem recebida: ");
  Serial.println(mensagem);
}

void conectarWiFi() {
  Serial.print("Conectando ao WiFi...");
  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWiFi conectado!");
}

void conectarMQTT() {
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  client.setInsecure();
  mqtt.setCallback(callback);

  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker...");

    String clientId = "S3_" + String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), BROKER_USER, BROKER_PASS)) {
      Serial.println("Conectado!");

      mqtt.subscribe(TOPICO_ILUMINACAO);
      mqtt.subscribe(TOPICO_SUB_DIST1);
      mqtt.subscribe(TOPICO_SUB_DIST2);

      Serial.println("Inscrições concluídas.");

    } else {
      Serial.print("Falha. Código: ");
      Serial.println(mqtt.state());
      delay(1500);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PINO_LED, OUTPUT);
  pinMode(PINO_PRESENCA, INPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  servo3.attach(PINO_SERVO3);
  servo4.attach(PINO_SERVO4);

  servo3.write(0);
  servo4.write(0);

  conectarWiFi();
  conectarMQTT();
}

void loop() {
  if (!mqtt.connected()) conectarMQTT();
  mqtt.loop();

  long distancia = medirDistancia(TRIG, ECHO);
  Serial.println(distancia);

  // PUBLICAÇÃO DO ULTRASSOM
  if (distancia > 0 && distancia < 10)
    mqtt.publish(TOPICO_PUB_ULTRA, "objeto_proximo");
  else if (distancia > 10)
    mqtt.publish(TOPICO_PUB_ULTRA, "objeto_longe");

  // PUBLICAÇÃO DA PRESENÇA
  unsigned long agora = millis();
  if (agora - lastPublish >= publishInterval) {
    lastPublish = agora;

    int presenca = digitalRead(PINO_PRESENCA);
    mqtt.publish(TOPICO_PUB_DIST3, String(presenca).c_str());

    Serial.print("Presença publicada: ");
    Serial.println(presenca);
  }

  delay(20);
}
