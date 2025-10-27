#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient client;            //cria o cliente wifi
PubSubClient mqtt(client);    //fala que o mqtt usa o cliene wifi

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

const int PORT = 1883;
const String URL = "test.mosquitto.org";

const String broker_user = "";
const String broker_pass = "";

const String MyTopic = "TopicoChat"; //define de onde vou receber as mensagem
const String OtherTopic = "TopicoChat"; //define para onde vou enviar as mensagens

const int LED = 2;

void setup() {
  Serial.begin(115200);
  Serial.println("Conectando ao WiFi");
  WiFi.begin(SSID,PASS);

  pinMode(LED,OUTPUT);

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }

  Serial.println("\nConectado!");
  Serial.println("IP:");
  Serial.print(WiFi.localIP());

  Serial.print("Conectando ao broker...");
  mqtt.setServer(URL.c_str(),PORT);

  while(!mqtt.connected()){
    String ID = "S3-";
    ID += String(random(0xffff),HEX);

    mqtt.connect(ID.c_str(),broker_user.c_str(),broker_pass.c_str());
    delay(200);
    Serial.print(".");
  }
  mqtt.subscribe(MyTopic.c_str());
  mqtt.setCallback(callback);
  Serial.println("\n Conecado ao broker com sucesso!");

}

void loop() {
  if (WiFi.status() != WL_CONNECTED){

    Serial.println("Conectando ao WiFi");
    WiFi.begin(SSID,PASS);

    while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(200);
    }

    Serial.println("\nConectado!");
    Serial.println("IP:");
    Serial.print(WiFi.localIP());
  }

  String mensagem = "Marco: ";

  if(Serial.available()>0){
    mensagem += Serial.readStringUntil('\n'); //lê mensagem digitada
    mqtt.publish(OtherTopic.c_str(),mensagem.c_str()); //envia a mensagem para o tópico
  }
  mqtt.loop();
  delay(500);
}

void callback(char* topic, byte* payload, unsigned int length){
  String mensagem = "";
  for(int i = 0; i < length; i++){
    mensagem += (char)payload[i];
  }
  Serial.print("Recebido: ");
  Serial.println(mensagem);

  if(mensagem == "Carlos: acender" || mensagem == "Enzo: acender" || mensagem == "Brayan: acender" || mensagem == "Vitor: acender" || mensagem == "Marco: acender"){
    digitalWrite(LED, HIGH);
    Serial.println("Led Ligado");
  }

  if(mensagem == "Carlos: apagar" || mensagem == "Enzo: apagar" || mensagem == "Brayan: apagar" || mensagem == "Vitor: apagar" || mensagem == "Marco: apagar"){
    digitalWrite(LED, LOW);
    Serial.println("Led Desligado");
  }
}