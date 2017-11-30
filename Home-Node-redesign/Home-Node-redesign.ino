#include<SoftwareSerial.h>
#include <SPI.h>
#include "RF24.h"

String reading = "";
/* Variables for 2G connection */
SoftwareSerial client_2G(7,8);             //2G network pin 7:Rx, pin 8: Tx, pin 9: power Up/Down for SW reset

/* Variable for RF communication */
RF24 radio(2,3);
const uint64_t pipe = 0xE8E8F0F0E1LL; //channel to recieve
byte addresses[][6] = {"1Node","2Node"};

/* data structure for the tranmission */
//unsigned long msg;
const byte NodeID = 0;                      //Node ID for home station
float NodeData = 0;
const int Max_Nodes = 20;

//char reading[] = {5,3,2,4,1};

typedef struct {
  byte ID; //Node ID number
  byte path [Max_Nodes]; //The path to go down    //up to 256 differnt node names only a path of 31
  byte Place_In_Path; //Where in the array are we
  byte cmd; //go to sleep, other odd commands
  bool return_flag;//Return to home node, go from ++ to --
  float sensor1;
}MsgData;

  MsgData My_Data;
  MsgData Received_Data;


//Initializing the Data in Structs.
//These can be altered Later by using Struct_name.Struct_access 

int TransAMOUNT=5;
int DataTRANS=false;
int i;
unsigned long Timeout=5000;
int count=0;
unsigned long start_time;
bool old_Data=true;


void setup() {
  /* Set up 2G network here*/
  client_2G.begin(19200);                  //the GPRS baud rate
  Serial.begin(19200);                    // Serial monitor baud rate
  delay(500);

  /* Set up RF radio reception here*/
  for( i=0; i<Max_Nodes; i++){
      My_Data.path[i] = 0;
    }
     My_Data.return_flag=0;
     My_Data.Place_In_Path=1;

    My_Data.sensor1 = NodeData;
    Serial.begin(9600);
    radio.begin();
    radio.setAutoAck(false);
    radio.openReadingPipe(1,pipe);
    radio.startListening();
    
  /* Run once to go end-to-end */
  connect_GPRS();
  Submit_HTTP_request();
  if (client_2G.println("AT+CSQ"))
      Serial.print("Connected_setup");
//  if( client_2G.available() )
//    Serial.print("Connected_setup");
//  else
//    Serial.print("NotConnected_setup");
}

void loop() {
  /**
   * if 2G network available: wait for radio reception and perform duty
   * else: power cycle by SW reset (power_cycle)
   */
  if( client_2G.available() ) {
    while( client_2G.available() ) {
      Serial.println("loop_Connected");
      delay(1000);
      Serial.println(" POST something ");

      //Submit_HTTP_request();
      /* if null path, exit the condition*/
      
    }
  
  } else {  //perform power_cycle
    //power_cycle();
    Serial.println(" Loop_no_Connected");
    delay(1000);
  }
}

void ShowSerialData() {

  int8_t i = 0;
  while( client_2G.available() != 0)
    Serial.write( client_2G.read() );
    reading[i] = client_2G.read();
    delay(100);
    i++;
    
}

/**
 * Software reset: re-initilize 2G module (solder JP for pin9)
 * This will be executed if there 2G cellular link lost connectivity
 */
void power_cycle() {
  Serial.println(" Software PowerUp ");
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  delay(1000);

  digitalWrite(9, HIGH);
  delay(2000);

  digitalWrite(9, LOW);
  delay(3000);
}

void connect_GPRS() {
 client_2G.println("AT+CSQ");                                 //Check signal quality
    delay(100);
    ShowSerialData();

  client_2G.println("AT+CGATT?");                             //Attach or Detach from GPRS Support
    delay(100);
    ShowSerialData();
    
  client_2G.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");     //setting the SAPBR, the connection type is using gprs
    delay(1000);
    ShowSerialData();

  client_2G.println("AT+SAPBR=3,1,\"APN\",\"TRACFONE-WFM\""); //setting the APN, Access point name string
    delay(4000);
    ShowSerialData();

  client_2G.println("AT+SAPBR=1,1");                          //setting the SAPBR
    delay(2000);
    ShowSerialData();
  
//  client_2G.println("AT+SAPBR=2,1");
//    delay(1000);
//    ShowSerialData();
}

void Submit_HTTP_request() {
  client_2G.println("AT+HTTPINIT");           //init the HTTP request
    delay(2000);
    ShowSerialData();
  
  //this is a modification to obtain path from api
  client_2G.println("AT+HTTPPARA=\"URL\",\"http://sensorweb.ece.iastate.edu/api/4/path\"");//Public server IP address
    delay(1000);
    ShowSerialData();

  client_2G.println("AT+HTTPPARA=\"CID\",1");      //http://sensorweb.ece.iastate.edu/api/4/path
    delay(1000);
    ShowSerialData();

  client_2G.println("AT+HTTPACTION=0");    //=0 (READ), =1 (POST)
    delay(6000);
    ShowSerialData();

  client_2G.println("AT+HTTPREAD");       //Send a command to read the HTTP server response, run AT+HTTPACTION in prior
    delay(300);
    ShowSerialData();
  client_2G.println("");
    delay(100);         
}


void Post_Http_request()
{
  /* this function specifically extract only the path and not the other response retrieved from
  HTTPREAD response
  Reading and extracting the desired path from the api end here*/

  /* make a post to the server with sensor value */
  client_2G.println("AT+HTTPINIT");           //init the HTTP request
    delay(2000);
    ShowSerialData();
  
  client_2G.println("AT+HTTPPARA=\"URL\",\"http://posttestserver.com/post.php?dir=Homenodetestiastate\"");//Public server IP address
    delay(1000);
    ShowSerialData();

  client_2G.println("AT+HTTPPARA=\"CONTENT\",\"application/json"); //create the data structure in json format
    delay(1000);
    ShowSerialData();


  client_2G.println("AT+HTTPDATA=" + String(sizeof(reading)) + ",100000"); //insert data into the json format
    delay(1000);
    ShowSerialData();

  client_2G.println("AT+HTTPACTION=1");    //=0 (READ), =1 (POST)
    delay(1000);
    ShowSerialData();


  client_2G.println("AT+HTTPTERM");        //terminate the HTTP
    delay(1000);
    ShowSerialData();
}


