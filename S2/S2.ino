#include <WiFi.h>                   // Biblioteca para conectar o ESP32 ao Wi-Fi
#include <WiFiClientSecure.h>       // Biblioteca para conexão segura (TLS)
#include <PubSubClient.h>           // Biblioteca para usar MQTT (publicar/assinar tópicos)

const char* SSID = "FIESC_IOT_EDU"; // Nome da rede Wi-Fi
const char* PASS = "8120gv08";      // Senha da rede Wi-Fi

const char* BROKER = "e6607d8d5fec40a9974cfc1552a13e2f.s1.eu.hivemq.cloud"; // Endereço do broker MQTT
const int PORT = 8883;              // Porta segura MQTT (usa TLS)

const char* BROKER_USER = "leonardo_martins"; // Usuário do broker
const char* BROKER_PASS = "Integrante4";      // Senha do broker

// Pinos dos sensores ultrassônicos
#define TRIG1 12
#define ECHO1 25
#define TRIG2 18
#define ECHO2 21

#define PINO_LED 4                  // LED controlado via MQTT

// Tópicos MQTT usados pelo ESP32
const char* TOPICO_PUB_DIST1 = "S2/Distancia1"; // Publica detecção do sensor 1
const char* TOPICO_PUB_DIST2 = "S2/Distancia2"; // Publica detecção do sensor 2
const char* TOPICO_PUB_ILU   = "S2/Cmd";        // Recebe comando para acender/apagar LED
const char* TOPICO_ENVIO_S3  = "S3/Cmd";        // (Reservado, não usado no código)

WiFiClientSecure espClient;         // Cliente seguro para TLS
PubSubClient mqtt(espClient);       // Cliente MQTT usando espClient seguro

// Função para medir distância do ultrassônico
long medirDistancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);       // Garante nível baixo antes do pulso
  delayMicroseconds(5);

  digitalWrite(trigPin, HIGH);      // Pulso de 10us no TRIG
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 30000); // Tempo do eco
  long distancia = duracao * 0.034 / 2;         // Converte tempo em cm
  return distancia;
}

// Função chamada quando chega mensagem MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem = "";

  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];   // Monta a string recebida
  }

  Serial.print("[CMD] ");
  Serial.println(mensagem);

  if (mensagem == "acender") {      // Se comando for "acender"
    digitalWrite(PINO_LED, HIGH);   // Liga LED
  } 
  else if (mensagem == "apagar") {  // Se comando for "apagar"
    digitalWrite(PINO_LED, LOW);    // Desliga LED
  }
}

// Conexão ao Wi-Fi
void conectaWiFi() {
  WiFi.begin(SSID, PASS);           // Inicia conexão Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");              // Esperando conectar
  }
  Serial.println("WiFi conectado!");
}

// Conexão ao MQTT com TLS
void conectaMQTT() {
  mqtt.setServer(BROKER, PORT);     // Define broker e porta
  mqtt.setCallback(callback);       // Define função que lida com mensagens
  espClient.setInsecure();          // Aceita TLS sem certificado (modo simples)

  while (!mqtt.connected()) {       // Tenta conectar até conseguir
    String clientId = "S2_" + String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), BROKER_USER, BROKER_PASS)) {
      mqtt.subscribe(TOPICO_PUB_ILU);  // Inscreve para receber comandos
    } else {
      delay(1000);                     // Tenta de novo após 1s
      Serial.print('.');
    }
  }
  Serial.println("MQTT conectado!");
}

void setup() {
  Serial.begin(115200);             // Inicializa o monitor serial
  espClient.setInsecure();          // TLS sem validação de certificado

  // Configura pinos dos sensores
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  pinMode(PINO_LED, OUTPUT);        // LED como saída
  digitalWrite(PINO_LED, LOW);      // Começa apagado

  conectaWiFi();                    // Conecta ao Wi-Fi
  conectaMQTT();                   // Conecta ao MQTT
}

void loop() {
  if (!mqtt.connected()) {          
    conectaMQTT();                  // Reconnecta ao MQTT se cair
  }

  // ---- Sensor 1 ----
  long duracao1;
  long distancia1;

  digitalWrite(TRIG1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG1, LOW);

  duracao1 = pulseIn(ECHO1, HIGH);          // Lê eco
  distancia1 = duracao1 * 0.034 / 2;        // Converte para cm

  if (distancia1 < 10) {                    // Se objeto < 10 cm
    mqtt.publish(TOPICO_PUB_DIST1, "Detectado"); // Envia detecção
  }

  // ---- Sensor 2 ----
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

  mqtt.loop();                     // Mantém MQTT funcionando
}
