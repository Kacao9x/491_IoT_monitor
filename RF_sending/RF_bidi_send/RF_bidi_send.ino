#include <SPI.h>  
#include "RF24.h"

/* set up communication pipe:
Address: 0 | channel: 115 | Max_power | data rate: 250kb (lowest - better range)

Speed options: RF24_250KBPS | RF24_1MBPS | RF24_2MBPS
Power consump: PA_MIN | PA_LOW | PA_HIGH | PA_MAX
*/
RF24 myRadio (7, 8);                        //ce,csn pin
byte addresses[][6] = {"00001", "00002"};

/* create a data structure to store the data that needed to transmit */
struct package
{
  int id=1;
  float temperature = 0.0;
  char  text[100] = "";
};

typedef struct package Package;
Package data;
Package data_received;

void setup()
{
  Serial.begin(115200);                     //Set serial speed
  delay(1000);
  myRadio.begin();  
  myRadio.setChannel(115);                  //Communication channel: 115 
  myRadio.setPALevel(RF24_PA_MIN);          //Max power to enhance comm range
  myRadio.setDataRate( RF24_250KBPS ) ;     //250kB lowest data transfer rate --> achieve better range
  myRadio.openWritingPipe( addresses[1]);   //open pipe to send data to node 2
  myRadio.openReadingPipe(1, addresses[0]); //open pipe to receive from node 1
  delay(100);
}

void loop()
{
  delay(5);
  myRadio.stopListening();
  myRadio.write(&data, sizeof(data));       //populate the data structure
  if (data.id > 10) {
    data.id = 0;
    RF_listening();                         //start listening from node 2, sending hanging up.
  }
  /*
   * this is wrong because it take O(1) to send data
  for (int8_t i = 0; i < 100; i++) {
    myRadio.write(&data, sizeof(data));       //populate the data structure
  }*/
  
  delay(5);
  /*
  myRadio.startListening();
  while(!myRadio.available());                //infinite loop waiting for feedback from node 2
  myRadio.read( &data_received, sizeof(data_received) );        //read data package from node 2
  */
  //if(data_received == "bingo") do smt;
  /* print out and update data what we have sent*/
  Serial.print("\nPackage:");
  Serial.print(data.id);
  Serial.print("\n");
  Serial.println(data.temperature);
  Serial.println(data_received.text);
  data.id = data.id + 1;
  data.temperature = data.temperature+0.3;
  delay(1000);

/* CALCULATE how long to transmit data package*/
}

void RF_listening() {
  myRadio.startListening();
  Serial.println("listening");
  /*
  if ( myRadio.available())
  {
    while( myRadio.available())
    {
      myRadio.read( &data_received, sizeof(data_received) );        //read data package from node 2
    }
  }*/
  delay(1000);
}

