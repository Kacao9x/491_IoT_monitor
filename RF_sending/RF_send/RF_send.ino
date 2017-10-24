#include <SPI.h>  
#include "RF24.h"

/* set up communication pipe:
Address: 0 | channel: 115 | Max_power | data rate: 250kb (lowest - better range)

Speed options: RF24_250KBPS | RF24_1MBPS | RF24_2MBPS
Power consump: PA_MIN | PA_LOW | PA_HIGH | PA_MAX
*/
RF24 myRadio (7, 8);                        //ce,csn pin
byte addresses[][6] = {"0"};

/* create a data structure to store the data that needed to transmit */
struct package
{
  int id=1;
  float temperature = 18.3;
  char  text[100] = "Temperature value here";
};

typedef struct package Package;
Package data;

void setup()
{
  Serial.begin(115200);                     //Set serial speed
  delay(1000);
  myRadio.begin();  
  myRadio.setChannel(115);                  //Communication channel: 115 
  myRadio.setPALevel(RF24_PA_MAX);          //Max power to enhance comm range
  myRadio.setDataRate( RF24_250KBPS ) ;     //250kB lowest data transfer rate --> achieve better range
  myRadio.openWritingPipe( addresses[0]);   //open pipe to send data
  delay(1000);
}

void loop()
{
  myRadio.write(&data, sizeof(data));       //populate the data structure

  Serial.print("\nPackage:");
  Serial.print(data.id);
  Serial.print("\n");
  Serial.println(data.temperature);
  Serial.println(data.text);
  data.id = data.id + 1;
  data.temperature = data.temperature+0.1;
  delay(1000);

/* CALCULATE how long to transmit data package*/
}
