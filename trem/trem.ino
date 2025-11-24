#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFi.h>

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

WiFiClientSecure client; //cria cliente wifi
PubSubClient mqtt(client); //fala que mqtt usa o cliente wifi

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

const int PORT = 8883;
const String URL = "e6607d8d5fec40a9974cfc1552a13e2f.s1.eu.hivemq.cloud";

const String broker_user = "caioponick";
const String broker_pass = "Ca16102007";

const char* TOPICO_PUB_DIST1 = "S2/Distancia1";
const char* TOPICO_PUB_TEMP = "S1/Temperatura";
const char* TOPICO_PUB_UMI = "S1/Umidade";
const char* TOPICO_PUB_ILU = "S1/Iluminacao";



const byte LED_AZUL = 2;
const int TRIG = 22;
const int ECHO = 23;
const int LDR  = 34;

void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println("Conectando ao WiFi");
  WiFi.begin(SSID, PASS);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado!");
  client.setInsecure();
  Serial.println("Conectando ao broker...");
  mqtt.setServer(URL.c_str(),PORT);
  while(!mqtt.connected()){
    String ID = "S1-";
    ID += String(random(0xffff),HEX);
    mqtt.connect(ID.c_str(),broker_user.c_str(),broker_pass.c_str());
    delay(200);
    Serial.print(".");
  }
  mqtt.subscribe(TOPICO_PUB_ILU);
  mqtt.setCallback(callback);
  Serial.println("\n Conectado ao broker com sucesso!");

  pinMode(LED_AZUL, OUTPUT);
}

long lerUltrassonico() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duracao = pulseIn(ECHO, HIGH);
  long distancia = duracao * 0.034 / 2;  // cm
  return distancia;
}

void loop() {

  float temperatura = dht.readTemperature(); 
  mqtt.publish(TOPICO_PUB_TEMP, String(temperatura).c_str());
  float umidade = dht.readHumidity();
  mqtt.publish(TOPICO_PUB_UMI, String(umidade).c_str());

  long distancia = lerUltrassonico();
  if (distancia < 10) {
    mqtt.publish("sensor/movimento", "detectado");
  }

  int leituraLDR = analogRead(LDR);

  if (leituraLDR > 2700) {
    mqtt.publish(TOPICO_PUB_ILU, "acender");
  } else {
    mqtt.publish(TOPICO_PUB_ILU, "apagar");
  }

  mqtt.loop();
  delay(2000);
}

void callback(char* topic, byte* payload, unsigned int length){
  String mensagem = "";
  for(int i = 0; i < length; i++){
    mensagem += (char)payload[i];
  }
  Serial.print("Recebido: ");
  Serial.println(mensagem);

  if(mensagem=="acender"){
    digitalWrite(LED_AZUL, HIGH);
  } else if(mensagem == "apagar"){
    digitalWrite(LED_AZUL, LOW);
  }
}