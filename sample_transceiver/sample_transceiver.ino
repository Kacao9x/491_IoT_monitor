#include <Arduino.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

int SUM=500;

long msg[1];
RF24 radio(9,10);
const uint64_t pipe = 0xE8E8F0F0E1LL; //channel to recieve
int LED1 = 3;
int i=0;
int j;
int data[500]={0};     //array size must be same as SUM size
double avg;


void setup(void){
    Serial.begin(9600);
    radio.begin();
    radio.openReadingPipe(1,pipe);
    radio.startListening();
    pinMode(LED1, OUTPUT);}

void loop(void){
    if (radio.available()) {
        bool done = false;
        while (!done) {
            done = radio.read(msg, 4);  //byte value
            if (!done) {
                //Serial.println(msg[0]);}
            }
            avg = 0;
            data[i] = 1;
            i++;
            if (i > (SUM-1)) {     //check if i>SUM-1
                i = 0;
            }
            for (j=0;j<SUM;j++) {
                avg += data[j];
            }
            Serial.print(avg/5);
            Serial.print(", ");
        }
    }
    else{
        //Serial.println("No radio available");
        avg=0;
        data[i]=0;
        i++;
        if(i>(SUM-1)){
            i=0;
            }
        for(j=0;j<SUM;j++){
                avg+=data[j];
            }
        Serial.print(avg/5);
        Serial.print(", ");
        }
}
