#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4
SoftwareSerial GSM(2,3);               //RX, TX

void setup() {
        GSM.begin(115200);
        Serial.begin(115200);
}

void loop() {
       while(!GSM.available()) {
                Serial.println("no signal");
       }
        while(GSM.available()) {
                Serial.write(GSM.read());
        }
        while(GSM.available()) {
                GSM.write(Serial.read());
        }
        

}
