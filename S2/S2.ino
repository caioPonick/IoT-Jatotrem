#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>


const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";


const char* BROKER = "e6607d8d5fec40a9974cfc1552a13e2f.s1.eu.hivemq.cloud";
const int PORT = 8883;

const char* BROKER_USER = "leonardo_martins";
const char* BROKER_PASS = "Integrante4";

#define TRIG1 12
#define ECHO1 25
#define TRIG2 18
#define ECHO2 21
#define PINO_LED 4

// + e -  -> 3v3 e GND da placa


const char* TOPICO_PUB_DIST1 = "S2/Distancia1";
const char* TOPICO_PUB_DIST2 = "S2/Distancia2";
const char* TOPICO_PUB_ILU   = "S2/Cmd";
const char* TOPICO_ENVIO_S3  = "S3/Cmd";

//iluminacao     


WiFiClientSecure espClient;
PubSubClient mqtt(espClient);


long medirDistancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 30000);
  long distancia = duracao * 0.034 / 2;
  return distancia;
}


void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem = "";

  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.print("[CMD] ");
  Serial.println(mensagem);

  // Comandos recebidos do tópico S2/Cmd
  if (mensagem == "acender") {
    digitalWrite(PINO_LED, HIGH);
  } 
  else if (mensagem == "apagar") {
    digitalWrite(PINO_LED, LOW);
  }
}


void conectaWiFi() {
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado!");
}


void conectaMQTT() {
  mqtt.setServer(BROKER, PORT);
  mqtt.setCallback(callback);
  espClient.setInsecure();

  while (!mqtt.connected()) {
    String clientId = "S2_" + String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), BROKER_USER, BROKER_PASS)) {
      mqtt.subscribe(TOPICO_PUB_ILU);
      //todos os tópicos para se inscrever
    } else {
      delay(1000);
      Serial.print('.');
    }
  }
  Serial.println("MQTT conectado!");
}


void setup() {
  Serial.begin(115200);
  espClient.setInsecure();
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  pinMode(PINO_LED, OUTPUT);
  digitalWrite(PINO_LED, LOW);

  conectaWiFi();
  conectaMQTT();
}


void loop() {
if (!mqtt.connected()) {
    conectaMQTT();
  }

  // --- Ler ultrassonico 1 ---
  long duracao1;
  long distancia1;

  digitalWrite(TRIG1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG1, LOW);

  duracao1 = pulseIn(ECHO1, HIGH);
  distancia1 = duracao1 * 0.034 / 2; 

  if (distancia1 < 10) {
    mqtt.publish(TOPICO_PUB_DIST1, "Detectado");
  }


  long duracao2;
  long distancia2;

  digitalWrite(TRIG2, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG2, LOW);

  duracao2 = pulseIn(ECHO2, HIGH);
  distancia2 = duracao2 * 0.034 / 2;

  if (distancia2 < 10) {
    mqtt.publish(TOPICO_PUB_DIST2, "Detectado");
  }
  mqtt.loop();
}