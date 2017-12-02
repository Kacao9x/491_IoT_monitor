#include<SoftwareSerial.h>
#include <SPI.h>
#include "RF24.h"
#include "LowPower.h"

char reading[] = {5,3,2,4,1};               //change to dynamic array then?
/* Variables for 2G connection */
SoftwareSerial client_2G(7,8);             //2G network pin 7:Rx (CE), pin 8(CSN): Tx, pin 9: power Up/Down for SW reset

/* Variable for RF communication */
RF24 radio(2,3);
const uint64_t pipe = 0xE8E8F0F0E1LL; //channel to recieve
byte addresses[][6] = {"1Node","2Node"};

/* data structure for the tranmission */
//unsigned long msg;
const byte NodeID = 0;                      //Node ID for home station
float NodeData = 9999999;                   //Home_node.NodeData = time sleep for low power mode.
const int Max_Nodes = 20;

typedef struct {
  byte ID;                                  //Node ID number
  byte path [Max_Nodes];                    //The path to go down    //up to 256 differnt node names only a path of 31
  byte Place_In_Path;                       //Where in the array are we
  byte cmd;                                 //go to sleep, other odd commands
  bool return_flag;                         //Return to home node, go from ++ to --
  float sensor1;                            //set time for sleep (shared by HOME + OPNODE)
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

/* variable for sleep state, flag*/
int8_t POST_done_flag = 0;


void setup() {
  /* Set up 2G network here*/
  client_2G.begin(19200);                   //the GPRS baud rate
  Serial.begin(19200);                      // Serial monitor baud rate
  delay(500);

  /* Set up RF radio reception here*/
    _clear_data_struct();
     My_Data.Place_In_Path=1;               //CHANNGEE ----------------------------------??

    My_Data.sensor1 = NodeData;             //sleep time for low-power mode. Sent out to Op-Node to make sure compatibility
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

      /**
       * GET: checking a new path from website 
       * if the path != 0 or flag == 1 execute performance
       * else return
       * 
       *    ...<if> read_path, store in My_Data.path[]
       *    Integrate with T&S tranceiver
       *      Zero_out the path [0,0,0,0]
       *    POST Received.data + flag == done?
       *    
       *    return
       *    
       */
      
      //zero_out the path
      /*
      Submit_HTTP_request();
      _CastString_to_Int_Array();
      if (web_flag == 0) return;
      else {
        //Tim+Steve transmission
        //zero_out path
        Post_HTTP_request();
      } */
      connect_GPRS();
      memset(reading, 0, sizeof(reading));

      /* fake reading -------------------------------------------- CHANGE me when the web is done*/
      for(i=1; i<6; i++) {
        reading[i-1] = i;
      }
      Submit_HTTP_request();
      Serial.println("read the website");
      /* Need to check the condition, cannot exit when reading =0 */
      if(reading[0] == 0) {
        continue;         //never
      }
      else{
        serial_logger();
      }

      /*POST data to website*/
      while( POST_done_flag == 0 ) {
        Post_HTTP_request();
        Submit_HTTP_request();
        POST_done_flag = 1;               // ---------------------- ASK Webteam to construct the value once they get the data
      }

      power_off();
      My_Data.cmd = 1;                    // sleep command
      My_Data.sensor1 = NodeData;

      
      //if (flag == 1) continue; else ....
    }
    return;
  } else {  //perform power_cycle
    //If no power at the beginning, works fine. But if mannually turn off, never back on???
    Serial.println(" Loop_no_Connected - power reset");
    power_cycle();
    delay(1000);
    //connect_GPRS();
    Serial.println("No need enable GPRS");                //Run once
  }
}

void ShowSerialData() {

  int8_t w = 0;
  while( client_2G.available() != 0)
    Serial.write( client_2G.read() );
    reading[w] = client_2G.read();
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

void power_off() {
    client_2G.println("AT+HTTPTERM");        //terminate the HTTP, cause POWER RESET ----------------- CAUTION
    delay(1000);
    ShowSerialData();
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
  client_2G.println("AT+HTTPPARA=\"URL\",\"HTTP://sensorweb.ece.iastate.edu/api/4/path\"");//Public server IP address
    delay(1000);
    ShowSerialData();

  client_2G.println("AT+HTTPPARA=\"CID\",1");      //HTTP://sensorweb.ece.iastate.edu/api/4/path
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


void Post_HTTP_request()
{
  /* this function specifically extract only the path and not the other response retrieved from
  HTTPREAD response
  Reading and extracting the desired path from the api end here*/

  /* make a post to the server with sensor value */
  client_2G.println("AT+HTTPINIT");           //init the HTTP request
    delay(2000);
    ShowSerialData();
  
  client_2G.println("AT+HTTPPARA=\"URL\",\"HTTP://posttestserver.com/post.php?dir=Homenodetestiastate\"");//Public server IP address
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


  client_2G.println("AT+HTTPTERM");        //terminate the HTTP, cause POWER RESET ----------------- CAUTION
    delay(1000);
    ShowSerialData();
}

void _CastString_to_Int_Array() {

  return;
}


void serial_logger(){
    Serial.println("enter serial_logger");
    //clear path
    _clear_data_struct();
     My_Data.Place_In_Path=1;         //CHANGE ME ---------------------??

    //reading[] = {5,3,2,4,1}; populate My_data.path[] by the array
    My_Data.path[0] = 0;
    for( int8_t k = 0; k< sizeof(reading); k++) {
      My_Data.path[k+1] = int( reading[k] );
    }
    
    //CHANGE ME
//    int c = int(Serial.read());
//    if ( c == '1'){ 
//       //path to 1
//        My_Data.path[0] = 0;
//        My_Data.path[1] = 1;     
//      Serial.println(F("Calling node 1"));
//   }
//
//    else if ( c == '2'){ 
//        //path to 2
//        My_Data.path[0] = 0;
//        My_Data.path[1] = 1;
//        My_Data.path[2] = 2;   
//      Serial.println(F("Calling node 2"));
//   }
//     else{
//    return;
//   }

    Serial.print("print mydata struct: ");
   for( int8_t w = 0; w< sizeof(reading); w++) {
      Serial.print(My_Data.path[w]);
    }
    Serial.println(" ");
    Serial.println("-------------");
  
   transmit(My_Data);
    //RECIEVE
    delay(20);
    start_time=millis();
     Serial.println("---Listening For Response---");
     while(start_time+Timeout>millis()){
      receive();
     }
  Serial.println("restart loop");

}


void receive(){                                                             //Recieve Data from another node
    radio.openReadingPipe(1,addresses[0]);
    radio.startListening();  
       if(radio.available()){  
            radio.read(&Received_Data, sizeof(MsgData));  //byte value
          /*
            Serial.println("\nRecieved Data");
           
           Serial.print("ID: ");
           Serial.println(Received_Data.ID);
           Serial.print("Place_In_Path: ");
           Serial.println(Received_Data.Place_In_Path);
           Serial.print("Path: ");
           for (int i=0;i<Max_Nodes;i++){
             Serial.print(Received_Data.path[i]);
             Serial.print(", ");
           }
           Serial.println("");
           Serial.print("Return_Flag: ");
           Serial.println(Received_Data.return_flag);
            delay(5);
          */
      }
    return;
}

void transmit(MsgData Transmit_Msg){                                        //Transmit Data to Another Node
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
 * Helper method to zero out the Msg_Data struct
 */
void _clear_data_struct() {
    for( i=0; i<Max_Nodes; i++){
      My_Data.path[i] = 0;
    }
     My_Data.return_flag=0;
}

