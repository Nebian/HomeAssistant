#include <Arduino.h>
#include <Scheduler.h>
#include <Task.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "pitches.h"

const char* ssid = "Crotolamo";
const char* password = "CJ*ooC4c*9@Y@zq743*TXc";
const char* mqtt_server = "192.168.1.68";

const char* mqtt_username = "mqttest";
const char* mqtt_password = "@H3#tK4x6an3W!9*K**88B";

const int BUZZER_PIN = 4; // LED connected to GPIO4

int melody[] = {
  NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, REST,
  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, REST,
  NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, REST,
  NOTE_A4, NOTE_G4, NOTE_A4, REST,
  
  NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, REST,
  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, REST,
  NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, REST,
  NOTE_A4, NOTE_G4, NOTE_A4, REST,
  
  NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, REST,
  NOTE_A4, NOTE_C5, NOTE_D5, NOTE_D5, REST,
  NOTE_D5, NOTE_E5, NOTE_F5, NOTE_F5, REST,
  NOTE_E5, NOTE_D5, NOTE_E5, NOTE_A4, REST,
  
  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, REST,
  NOTE_D5, NOTE_E5, NOTE_A4, REST,
  NOTE_A4, NOTE_C5, NOTE_B4, NOTE_B4, REST,
  NOTE_C5, NOTE_A4, NOTE_B4, REST,
  
  NOTE_A4, NOTE_A4,
  //Repeat of first part
  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, REST,
  NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, REST,
  NOTE_A4, NOTE_G4, NOTE_A4, REST,
  
  NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, REST,
  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, REST,
  NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, REST,
  NOTE_A4, NOTE_G4, NOTE_A4, REST,
  
  NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, REST,
  NOTE_A4, NOTE_C5, NOTE_D5, NOTE_D5, REST,
  NOTE_D5, NOTE_E5, NOTE_F5, NOTE_F5, REST,
  NOTE_E5, NOTE_D5, NOTE_E5, NOTE_A4, REST,
  
  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, REST,
  NOTE_D5, NOTE_E5, NOTE_A4, REST,
  NOTE_A4, NOTE_C5, NOTE_B4, NOTE_B4, REST,
  NOTE_C5, NOTE_A4, NOTE_B4, REST,
  //End of Repeat
  
  NOTE_E5, REST, REST, NOTE_F5, REST, REST,
  NOTE_E5, NOTE_E5, REST, NOTE_G5, REST, NOTE_E5, NOTE_D5, REST, REST,
  NOTE_D5, REST, REST, NOTE_C5, REST, REST,
  NOTE_B4, NOTE_C5, REST, NOTE_B4, REST, NOTE_A4,
  
  NOTE_E5, REST, REST, NOTE_F5, REST, REST,
  NOTE_E5, NOTE_E5, REST, NOTE_G5, REST, NOTE_E5, NOTE_D5, REST, REST,
  NOTE_D5, REST, REST, NOTE_C5, REST, REST,
  NOTE_B4, NOTE_C5, REST, NOTE_B4, REST, NOTE_A4
};

int durations[] = {
  8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 8,
  
  8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 8,
  
  8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 8, 4, 8,
  
  8, 8, 4, 8, 8,
  4, 8, 4, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 4,
  
  4, 8,
  //Repeat of First Part
  8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 8,
  
  8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 8,
  
  8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 8, 4, 8,
  
  8, 8, 4, 8, 8,
  4, 8, 4, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 4,
  //End of Repeat
  
  4, 8, 4, 4, 8, 4,
  8, 8, 8, 8, 8, 8, 8, 8, 4,
  4, 8, 4, 4, 8, 4,
  8, 8, 8, 8, 8, 2,
  
  4, 8, 4, 4, 8, 4,
  8, 8, 8, 8, 8, 8, 8, 8, 4,
  4, 8, 4, 4, 8, 4,
  8, 8, 8, 8, 8, 2
};

WiFiClient espClient;
PubSubClient client(espClient);
volatile bool playMelody = false;

class MQTTClient : public Task {
protected:
    void setup() {
      Serial.begin(9600);
      pinMode(BUZZER_PIN, OUTPUT);

      WiFi.begin(ssid, password);

      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
      }

      Serial.println("WiFi connected.");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());

      client.setServer(mqtt_server, 1883);
      client.setCallback(callback);
      
      while (!client.connected()) {
        Serial.println("Connecting to MQTT broker...");
        if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
          Serial.println("Connected to MQTT broker.");
          client.subscribe("test/led");
        } else {
          Serial.print("Failed with state ");
          Serial.print(client.state());
          delay(2000);
        }
      }
    }
    void loop()  {
        client.loop();
    }
    static void callback(char* topic, byte* payload, unsigned int length) {
      String message = "";
      for (int i = 0; i < length; i++) {
        message += (char)payload[i];
      }
      Serial.print("Message received: ");
      Serial.println(message);
      
      if (message == "1") {
        playMelody = true; // Start playing the melody
      } else if (message == "0") {
        playMelody = false; // Stop playing the melody
        noTone(BUZZER_PIN);
      }
    } 
} MQTTClient;

class PlayMelody : public Task {
protected:
    void loop() {
      if (playMelody) {
        static int noteIndex = 0;
        int size = sizeof(durations) / sizeof(int);
        if (noteIndex < size) {
          int duration = 1000 / durations[noteIndex];
          tone(BUZZER_PIN, melody[noteIndex], duration);
          int pauseBetweenNotes = duration * 1.30;
          delay(pauseBetweenNotes);
          noteIndex++;
        } else {
          noteIndex = 0;
        }
      }
    }
private:
    uint8_t state;
} PlayMelody;

class MemTask : public Task {
public:
    void loop() {
        Serial.print("Free Heap: ");
        Serial.print(ESP.getFreeHeap());
        Serial.println(" bytes");

        delay(10000);
    }
} mem_task;

void setup() {
    Serial.begin(9600);

    Serial.println("");

    delay(1000);

    Scheduler.start(&MQTTClient);
    Scheduler.start(&PlayMelody);
    Scheduler.start(&mem_task);

    Scheduler.begin();
}

void loop() {}
