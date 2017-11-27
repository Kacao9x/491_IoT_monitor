#include<SoftwareSerial.h>
#include <SPI.h>
#include "RF24.h"

SoftwareSerial client(7,8);
RF24 radio(4,5);
const uint64_t pipe = 0xE8E8F0F0E1LL; //channel to recieve
byte addresses[][6] = {"1Node","2Node"};

int TransAMOUNT=5;
int DataTRANS=false;
int i;
unsigned long Timeout=5000;
int count=0;
unsigned long start_time;
bool old_Data=true;
//don't forget to inclued the following 
/*the transciver setu
 * byte addresses[][6] = {"1Node","2Node"};
 * 
 */
//

String reading ="{\"Homenode\":\"Testing\":\"SensorID\":\"123 456-7890\"}"; //this is just for testing purposes
String reading2 = "";       //the values to be posted to the web are converted to a string and stored in reading
String reading1 = "";       //the path is converted to string and then used to modify the path in the struct
int reading3[5];            //this is just a quick fix to modify the path in struct
byte readingHold[46];       //this would hold the response of the AT+HTTPREAD  
byte readingHold2[11];      //This contains only the data which is the path e.g[5,4,3,2,1]
int a =0;                   //this would hold the value for byte

//This line of codes modifies which node to obtain sensor data
String nodeNum = "4";       //this number can be modified
String apiPart1 = "AT+HTTPPARA=\"URL\",\"http://sensorweb.ece.iastate.edu/api/";
String apiPart2 = "/path\"";
String api = apiPart1 + nodeNum + apiPart2;
//This line of code which modifies api node number ends here;

//this is the struct that would be sent between the homenode and the sensor nodes
//the values needed for the struct
//unsigned long msg;
const byte NodeID = 0;          //for the homenode the ID is 0.
float NodeData = 0;             //This would be the sesnordata
const int Max_Nodes = 20;

typedef struct {
  byte ID;                      //Node ID number
  byte path [Max_Nodes];        //The path to go down    //up to 256 differnt node names only a path of 31
  byte Place_In_Path;           //Where in the array are we
  byte cmd;                     //go to sleep, other odd commands
  bool return_flag;             //Return to home node, go from ++ to --
  float sensor1;
}MsgData;

MsgData My_Data;                //this would contain the path
MsgData Received_Data;          //this would contain the sensor values
//the struct ends here


void setup()
{
  Serial.begin(9600);
  client.begin(9600);
  delay(500);
  //the part below is modified for the transciever
  radio.begin();
    radio.setAutoAck(false);
    radio.openReadingPipe(1,pipe);
    radio.startListening();
    pinMode(4,INPUT_PULLUP);  
    pinMode(5,INPUT_PULLUP); 
    pinMode(6,INPUT_PULLUP);
    //the part ends here 
  
  //initSIM();
  connectGPRS();
  connectHTTP();
}

void loop()
{
  
}

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
  ShowSerialData();
  
  client.println("AT+HTTPPARA=\"CID\",1");    
  delay(1000);
  ShowSerialData();

  
  //this is a modification to obtain path from api 
  client.println("AT+HTTPPARA=\"URL\",\"http://sensorweb.ece.iastate.edu/api/4/path\"");//Public server IP address where the path would be read
  delay(1000);
  ShowSerialData();

  client.println("AT+HTTPACTION=0");
  delay(1000);
  ShowSerialData();

 //The lines of code below sends a command to the module to read the reply from the api
  client.println("AT+HTTPREAD");
  delay(1000);
  ShowSerialData1(); //this function specifically extract only the path and not the other response included in the AT+HHTPREAD response
  //Reading and extracting the desired path from the api ends here. 

  //the test code starts here. The aim of the test code is to ensure that the ShowSerial function is working properly. 
  Serial.println("");
  int l=0;
  for(l=0;l<11;l++)             
  {
    Serial.write(readingHold2[l]);
  }

  //converts the bytes to string so it can be posted to the web for testing (it is used to modify the struct My_Data)
  int temp = 0;
  for(l=0;l<11;l++)
  {
    if(readingHold2[l]== 91)
    {
      reading1 += "[";
    }
    if(readingHold2[l] == 44)
    {
      reading1 += ",";
    }
    if(readingHold2[1] == 93)
    {
      reading1 +="]";
    }
    if(readingHold2[l] == 49)
    {
       reading1 +="1";
       reading3[temp]=1;
       temp+=1;
    }
    if(readingHold2[l] == 50)
    {
       reading1 +="2";
       reading3[temp]=2;
       temp+=1;
    }
     if(readingHold2[l] == 51)
    {
       reading1 +="3";
       reading3[temp]=3;
       temp+=1;
    }
     if(readingHold2[l] == 52)
    {
       reading1 +="4";
       reading3[temp]=4;
       temp+=1;
    }
     if(readingHold2[l] == 53)
    {
       reading1 +="5";
       reading3[temp]=5;
       temp+=1;
    }
    
  }
  Serial.println("");
  //end of the test code

  //
 //The code to send data(path) to the leafnode and also to recieve sensor values should be put here
 for(l=0;l<reading1.length();l++) //we are using 11 because the arraying readingHold2 contains elements of type byte
 {
   My_Data.path[l]=reading3[l]; //sets the path to the part from the api 
 }
 My_Data.ID = 0; //just set the Id to the homenode ID
 //
 //



//the code to transmit the My_Data to the sensornodes should be posted here
 transmit(My_Data);
    //RECIEVE
    delay(20);
    start_time=millis();
     Serial.println("---Listening For Response---");
     while(start_time+Timeout>millis()){
      receive();
     }
//MY_Data now contains all the data needed for transmision to sensor nodes
//
//the code ends here




  //this line of code posts to a server
  client.println("AT+HTTPPARA=\"URL\",\"http://posttestserver.com/post.php?dir=Homenodetestiastate\"");//Public server IP address where the data would be posted
  ShowSerialData();

  client.println("AT+HTTPPARA=\"CONTENT\",\"application/jason");
  delay(1000);
  ShowSerialData();


  client.println("AT+HTTPDATA=" + String(reading1.length()) + ",100000");
  delay(1000);
  ShowSerialData();


  client.println("AT+HTTPACTION=1");
  delay(1000);
  ShowSerialData();


  client.println("AT+HTTPREAD");
  delay(1000);
  ShowSerialData();
  

  client.println("AT+HTTPTERM");
  delay(1000);
  ShowSerialData();

  
}

void ShowSerialData()  //this function only outputs whatever the 2g module sends to the arudiuno
{
  a = 0;
  while(client.available()!=0)
  {
      Serial.write(client.read());
      a +=1;
      delay(100);
  }
  Serial.println("");
}

void ShowSerialData1() //this function is used to extract only the data from the response of the AT+HTTPREAD. It is very specific and works with only sensorweb api. This is because communication is done in bytes
{
  a=0;
  while(client.available()!=0)
  {
      readingHold[a]= byte(client.read());
      a+=1;
  }
  
  //inorder to understand this line of code, we have to understand the exact response of the AT+command Httpread
   int n = 0;
   int m = 29;
   for(n = 0;n<11;n++)
   {
      readingHold2[n] = readingHold[m];
      m+=1;
   }
}

//the code to transmit My_data to the sensor nodes
void transmit(MsgData Transmit_Msg){                                        //Transmit Data to Another Node
    radio.openWritingPipe(addresses[0]);
    radio.stopListening(); 
    for(i=0; i<TransAMOUNT; i++){  
        radio.write(&Transmit_Msg, sizeof(MsgData));
        delay(5);
    }
}
//the code to transmit ends here

//the code to recieve starts here
void receive(){                                                             //Recieve Data from another node
    radio.openReadingPipe(1,addresses[0]);
    radio.startListening();  
       if(radio.available()){  
            radio.read(&Received_Data, sizeof(MsgData));  //byte value
      }
    return;
}
