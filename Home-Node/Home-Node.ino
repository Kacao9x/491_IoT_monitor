#include<SoftwareSerial.h>
#include <SPI.h>
#include "RF24.h"

/* Variables for 2G connection*/
SoftwareSerial client(2,3);             //2G network pin 2:Rx, pin 3: Tx
String reading ="";                     //save the path (from the website) into string array
int received_path[8];                   //save the path in int array

/* Varable for radio communication*/
/* set up communication pipe:
Address: 0 | channel: 115 | Max_power | data rate: 250kb (lowest - better range)

Speed options: RF24_250KBPS | RF24_1MBPS | RF24_2MBPS
Power consump: PA_MIN | PA_LOW | PA_HIGH | PA_MAX
*/

RF24 radio(7,8);                        //pin2: ce, pin 3: csn
const uint64_t pipe = 0xE8E8F0F0E1LL;   //channel to recieve
byte addresses[][6] = {"1Node","2Node"};
//unsigned long msg;
const byte NodeID = 0;
float NodeData = 0;
const int Max_Nodes = 20;

/* create a data structure to store the data sent from transmitter */
typedef struct {
  byte ID;                //Node ID number
  byte path [Max_Nodes];  //The path to go down    //up to 256 differnt node names only a path of 31
  byte Place_In_Path;     //Where in the array are we
  byte cmd;               //go to sleep, other odd commands
  bool return_flag;       //Return to home node, go from ++ to --
  float sensor1;
}MsgData;

  MsgData My_Data;
  MsgData Received_Data;

int TransAMOUNT=5;
int DataTRANS=false;
int i;
unsigned long Timeout=5000;
int count=0;
unsigned long start_time;
bool old_Data=true;

void setup() {
  Serial.begin(9600);
  client.begin(9600);
  delay(500);

  for( i=0; i<Max_Nodes; i++){
      My_Data.path[i] = 0;
  }
      My_Data.return_flag=0;
      My_Data.Place_In_Path=1;
      My_Data.sensor1 = NodeData;

  radio.begin();
    radio.setAutoAck(false);
    radio.openReadingPipe(1,pipe);
    radio.startListening();
    
  //initSIM();
  //connectGPRS();
  //connectHTTP();
}

void loop() {
  
}

/**
 * Not sure we need to initilize GPRS?
 * */
void connectGPRS()
{
  client.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  delay(1000);
  ShowSerialData();

  client.println("AT+SAPBR=3,1,\"APN\",\"TRACFONE-WFM\"");//APN
  delay(1000);
  ShowSerialData();

  client.println("AT+SAPBR=1,1");
  delay(1000);
  ShowSerialData();
  
  client.println("AT+SAPBR=2,1");
  delay(1000);
  ShowSerialData();
}


void connectHTTP()
{
  client.println("AT+HTTPINIT");
  delay(1000);
  //ShowSerialData();

  client.println("AT+HTTPPARA=\"CID\",1");      //http://sensorweb.ece.iastate.edu/api/4/path
  delay(1000);
  ShowSerialData();
  //this is a modification
  
  client.println("AT+HTTPPARA=\"URL\",\"http://sensorweb.ece.iastate.edu/api/4/path\"");//Public server IP address
  delay(1000);
  ShowSerialData();

  client.println("AT+HTTPACTION=0");
  delay(1000);
  ShowSerialData();
 
  client.println("AT+HTTPREAD");  //in this line of code we are trying to store the sensor data
  delay(1000);
  ShowSerialData();              //this code is in progress
  
  
  client.println("AT+HTTPPARA=\"URL\",\"http://posttestserver.com/post.php?dir=Homenodetestiastate\"");//Public server IP address
  ShowSerialData();

  client.println("AT+HTTPPARA=\"CONTENT\",\"application/jason");
  delay(1000);
  ShowSerialData();


  client.println("AT+HTTPDATA=" + String(reading.length()) + ",100000");
  delay(1000);
  ShowSerialData();

  client.println("AT+HTTPACTION=1");
  delay(1000);
  ShowSerialData();

  client.println("AT+HTTPREAD");
  delay(1000);
  //ShowSerialData();

  client.println("AT+HTTPTERM");
  delay(1000);
  //ShowSerialData();
}

/**
 * This method read read the data data display in the website
 * save the path into string array
 */
void ShowSerialData()
{
  int8_t i = 0;               //index for reading array
  while(client.available()!=0)
  {
    reading[i] = client.read();
    i++;
    delay(100);
  }
  //parse the path value and store into an array
  //WARNING: do this function simutanuously with client.read may cause latency
    extractArray(reading);
}

/**
 * This method converts the path (retrieved from the website) in string array
 * to int array
 */
void extractArray(String readingHold) {
  int8_t k = 0;
  for (int8_t j=0; j<sizeof(readingHold); j++) {
    if (isDigit(reading.charAt(j))) {
      received_path[k] = readingHold[j] - '0';            //value in string need to convert into int. Should use Int() to cast?
      //Serial.println(reading[j]);
      Serial.println(received_path[k]);
      k++;
    }
  }
}




