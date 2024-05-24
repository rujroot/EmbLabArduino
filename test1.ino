#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "realme 2 Pro";
const char* password = "15041999";
const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "1fc76177-b771-4646-978f-2854bebe45b0";
const char* mqtt_username = "193yLF5zM3a7PgWQYi7NrubcfB3qPekp";
const char* mqtt_password = "2h23bJYMNJ8MFsFCPWi8nFamePcK89tz";

WiFiClient espClient;
PubSubClient client(espClient);
EspSoftwareSerial::UART testSerial(D7, D8);

char msg[1000];
long lastMsg = 0;
int value = 0;
String light = "500";
String distance = "20";
String sound = "2000";
String place = "Device 1";
String LED = "0";
String LED_SET = "2";
int mode = 2;

void setup() {
  Serial.begin(9600);
  testSerial.begin(115200);
//testSerial.begin(115200, EspSoftwareSerial::SWSERIAL_8N1, D7, D8, false, 95, 11);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("@msg/led");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println(message);
  if(String(topic) == "@msg/led") {
    LED_SET = message;
    mode = (LED_SET == "0") ? 0 : ((LED_SET == "1") ? 1 : 2);
  } 
}

void readData(){
   while (testSerial.available()) {
    String receivedData = testSerial.readStringUntil('\n');

    // Split the received data by comma
    String ReceivedValue[4];
    ReceivedValue[3] = "";
    int commaIndex = 0, i = 0;
    while ((commaIndex = receivedData.indexOf(',')) >= 0) {
        String part = receivedData.substring(0, commaIndex);
        receivedData = receivedData.substring(commaIndex + 1);
        ReceivedValue[i] = part;
        ++i;
    }
    
    // Print the last part
    if (receivedData.length() > 0) {
        ReceivedValue[i] = receivedData;
    }

    light = ReceivedValue[0];
    sound = ReceivedValue[1];
    distance = ReceivedValue[2];
    LED = ReceivedValue[3];

    int intLight = light.toInt();
    int intSound = sound.toInt();
    int intDistance = distance.toInt();

    light = String(min(1500, max(intLight, 0)));
    sound = String(min(4000, max(intSound, 2000)));
    distance = String(min(800 , max(intDistance, 0)));

    if(LED == "") LED = "0";
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  readData();

  testSerial.write(mode);
  testSerial.println(String(mode));

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    ++value;
    String data = "{\"data\": {\"light\":" + String(light) + ", \"distance\":" + String(distance) + ", \"sound\":" + String(sound) + ", \"place\":\"" + String(place) + "\", \"led\":" + String(LED) + "}}";
    Serial.println(data);
    data.toCharArray(msg, (data.length() + 1));
    client.publish("@shadow/data/update", msg);

  }
  delay(1);
}