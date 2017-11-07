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
const byte NodeID = 0;                  //Node 0 represent HOME
float NodeData = 0;                     //
const int Max_Nodes = 20;               //max 30 nodes

/* create a data structure to store the data sent from transmitter */
typedef struct {
  byte ID;                              //Node ID number
  byte path [Max_Nodes];                //The path to go down    
                                        //up to 256 differnt node names only a path of 31
  byte Place_In_Path;                   //Where in the array are we
  byte cmd;                             //go to sleep, other odd commands
  bool return_flag;                     //Return to home node, go from ++ to --
  float sensor1;                        //value of sensor data
}MsgData;

  MsgData My_Data;                      //data stored in HOME node
  MsgData Received_Data;                //data received from LEAF

int TransAMOUNT=5;
int DataTRANS=false;
int i;
unsigned long Timeout=5000;             //waiting 5s to listen echo from the leafnode
int count=0;                            //count what?
unsigned long start_time;               //begin waiting time
bool old_Data=true;

void setup() {
  Serial.begin(9600);
  client.begin(9600);
  delay(500);

  _clear_data_struct();                 //zero-out all variable inside the struct
      My_Data.sensor1 = NodeData;       //populate the sensor value

  radio.begin();
    radio.setAutoAck(false);
    radio.openReadingPipe(1,pipe);
    radio.startListening();
    
  //initSIM();
  //connectGPRS();
  //connectHTTP();                      //get the path and parse it to LEAF
}

/**
 * WARNING: We just be able to get path when the setup fn is executed
 * We need to move connectHTTP to the loop() and call it whenver we need
 */

void _clear_data_struct() {
  for( i=0; i<Max_Nodes; i++){
      My_Data.path[i] = 0;
  }
      My_Data.return_flag=0;
      My_Data.Place_In_Path=1;
}


void loop() {
  serial_logger();                      //decide what to transmit
    //display the data received from the target node
    if((Received_Data.return_flag == 1)
           &&(Received_Data.path[Received_Data.Place_In_Path]==0)){
      Serial.print("Data recieved: "); 
      Serial.print(Received_Data.ID);
      Serial.print(", ");Serial.println(Received_Data.sensor1);
      old_Data=true;
    }
}

void serial_logger() {
  _clear_data_struct();                 //clear path

  /* after get the path, place it here*/


  /* ping the tartget LEAF in the path we got before */
  transmit(My_Data);
    /* wait 5s for the echo from LEAF nodes: timeout 5s*/
    delay(20);
    start_time=millis();
      Serial.println("---Listening For Response---");
      while(start_time+Timeout>millis()){
        receive();
      }
  
}

void receive() {
  radio.openReadingPipe(1,addresses[0]);
    radio.startListening();  
       if(radio.available()){  
          radio.read(&Received_Data, sizeof(MsgData));  //byte value
       }
  return;
}

void transmit(MsgData Transmit_Msg){                    //Transmit Data to Another Node
    radio.openWritingPipe(addresses[0]);
    radio.stopListening();
    
  for(i=0; i<TransAMOUNT; i++){  
        radio.write(&Transmit_Msg, sizeof(MsgData));
        delay(5);
  }
           Serial.println("\nTransmitted Data");
           
           Serial.print("ID: ");
           Serial.println(Transmit_Msg.ID);

           Serial.print("Place_In_Path: ");
           Serial.println(Transmit_Msg.Place_In_Path);

           Serial.print("Path: ");
           for (i=0;i<Max_Nodes;i++){
             Serial.print(Transmit_Msg.path[i]);
             Serial.print(", ");
           }
           Serial.println("");

           Serial.print("Return_Flag: ");
           Serial.println(Transmit_Msg.return_flag);
        delay(5);
        Serial.print("Path: ");
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

/**
 * use AT command to communicate with the website
 * POST and GET data here
 * */
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
    _convert_Str_to_IntArray(reading);
}

/**
 * This method converts the path (retrieved from the website) in string array
 * to int array
 */
void _convert_Str_to_IntArray(String readingHold) {
  int8_t k = 0;
  for (int8_t j=0; j<sizeof(readingHold); j++) {
    if (isDigit(reading.charAt(j))) {
      received_path[k] = readingHold[j] - '0';      //Should use Int() to cast?
      //Serial.println(reading[j]);
      Serial.println(received_path[k]);
      k++;
    }
  }
}




