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
  char  text[100] ="";
};

byte addresses[][6] = {"00001", "00002"}; 

typedef struct package Package;
Package data;
Package data_send;

void setup() 
{
  Serial.begin(115200);
  delay(1000);

  myRadio.begin(); 
  myRadio.begin();  
  myRadio.setChannel(115);                  //Communication channel: 115 
  myRadio.setPALevel(RF24_PA_MIN);          //Max power to enhance comm range
  myRadio.setDataRate( RF24_250KBPS ) ;     //250kB lowest data transfer rate --> achieve better range
  myRadio.openReadingPipe(1, addresses[1]); //open pipe to read data from node 1 on 002
  myRadio.openWritingPipe( addresses[0]);   //open pipe to send data to node 1 from 002
}


void loop()  
{
  delay(5);
  int8_t count = 0;
  myRadio.startListening();
  if ( myRadio.available())                 // if RF on
  {
    while (myRadio.available())             // if RF still on
    {
      myRadio.read( &data, sizeof(data) );  //populate data into the Data structure
      //count++;
      //Serial.print("count: ");
      //Serial.print(count);
      
      if (count == 10) {
        Serial.println("start sending feedback");
        continue;
      } 
    }
    /* start sending feedback to 1 */
    delay(5);
    myRadio.stopListening();

    //data_send.text = "got new things";        cannot overwrite a string this way
    strncpy( data_send.text, "got a new thing", sizeof(data_send.text));
    myRadio.write(&data_send, sizeof(data_send));
    
    Serial.print("\nPackage:");
    Serial.print(data.id);
    Serial.print("\n");
    Serial.println(data.temperature);
    Serial.println(data.text);
  }

}
