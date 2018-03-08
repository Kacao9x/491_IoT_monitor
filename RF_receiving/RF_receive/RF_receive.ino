#include <SPI.h>  
#include "RF24.h" 

/* set up communication pipe:
Address: 0 | channel: 115 | Max_power | data rate: 250kb (lowest - better range)

Speed options: RF24_250KBPS | RF24_1MBPS | RF24_2MBPS
Power consump: PA_MIN | PA_LOW | PA_HIGH | PA_MAX
*/
RF24 myRadio (7, 8);                        //ce,csn pin
/* create a data structure to store the data sent from transmitter */
struct package
{
  int id=0;
  float temperature = 0.0;
  char  text[100] ="empty";
};

byte addresses[][6] = {"0"}; 

typedef struct package Package;
Package data;

void setup() 
{
  Serial.begin(115200);
  delay(1000);

  myRadio.begin(); 
  myRadio.begin();  
  myRadio.setChannel(115);                  //Communication channel: 115 
  myRadio.setPALevel(RF24_PA_MAX);          //Max power to enhance comm range
  myRadio.setDataRate( RF24_250KBPS ) ;     //250kB lowest data transfer rate --> achieve better range
  myRadio.openReadingPipe(1, addresses[0]); //open pipe to echo the data streaming to
  myRadio.startListening();
}


void loop()  
{
  if ( myRadio.available())                 // if RF on
  {
    while (myRadio.available())             // if RF still on
    {
      myRadio.read( &data, sizeof(data) );  //populate data into the Data structure
    }
    Serial.print("\nPackage:");
    Serial.print(data.id);
    Serial.print("\n");
    Serial.println(data.temperature);
    Serial.println(data.text);
  }

}
