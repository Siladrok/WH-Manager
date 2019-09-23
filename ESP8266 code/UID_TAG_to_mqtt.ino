#define SS_PIN 4  //D2
#define RST_PIN 5 //D1

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";

// Create MFRC522 instance.
MFRC522 mfrc522(SS_PIN, RST_PIN);   

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() 
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client")) 
    {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("room/lamp");
      } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() 
{
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();          // Initiate  SPI bus
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  mfrc522.PCD_Init();   // Initiate MFRC522
}

void loop() 
{
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client");
    
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  
  //Show UID on serial monitor
  Serial.println();
  //Serial.print(" UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
 
  content.toUpperCase();
  //Serial.println(content);
  //Serial.println(content.substring(1));
  String UIDtagstr = content.substring(1);
  static char UIDtag[20];
  UIDtagstr.toCharArray(UIDtag,20);
  client.publish("warehouse/rfid", UIDtag);
  Serial.print("Published on topic warehouse/rfid.  Payload: ");
  Serial.println(UIDtag);
  delay(1000);
} 
