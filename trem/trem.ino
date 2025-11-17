#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFi.h>

WiFiClientSecure client; //cria cliente wifi
PubSubClient mqtt(client); //fala que mqtt usa o cliente wifi

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

const int PORT = 8883;
const String URL = "e6607d8d5fec40a9974cfc1552a13e2f.s1.eu.hivemq.cloud";

const String broker_user = "caioponick";
const String broker_pass = "Ca16102007";

const String Presenca = "";
const String Temperatura = "r";
const String Umidade = "";
const String Iluminacao = "";



const byte LED_AZUL = 2;

void setup() {
  Serial.begin(115200);
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
  mqtt.subscribe(Iluminacao.c_str());
  mqtt.setCallback(callback);
  Serial.println("\n Conectado ao broker com sucesso!");

  pinMode(LED_AZUL, OUTPUT);
}

void loop() {

  ler temp -> publicar (mqtt.publish(topicotemp,String(temp).c_str()));
  ler umida -> publicar

  let ultrassonico 
    se < 10
      publicar, "detectado"

  ler ldr
    se < 2700
      puvlicar "acender"
    senao
      publicar "apagar"

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

  if(mensagem=="Acender"){
    digitalWrite(LED_AZUL, HIGH);
  } else if(mensagem == "Apagar"){
    digitalWrite(LED_AZUL, LOW);
  }
}