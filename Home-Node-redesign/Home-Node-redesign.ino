#include<SoftwareSerial.h>
#include <SPI.h>
#include "RF24.h"
#include "LowPower.h"

byte reading[6];               //change to dynamic array then?
byte readingHold[46];   //this would hold the response of the AT+HTTPREAD
byte readingHold2[11];  //This contains only the data which is the path e.g[5,4,3,2,1]
byte readingHold3[11];  //This would hold the value of the path recieved from the api
byte final_path[11];

/* Variables for 2G connection */
SoftwareSerial client_2G(7, 8);            //2G network pin 7:Rx (CE), pin 8(CSN): Tx, pin 9: power Up/Down for SW reset

/* Variable for RF communication */
RF24 radio(2, 3);
const uint64_t pipe = 0xE8E8F0F0E1LL; //channel to recieve
byte addresses[][6] = {"1Node", "2Node"};

/* data structure for the tranmission */
//unsigned long msg;
const byte NodeID = 0;                      //Node ID for home station
float NodeData = 2;                   //Home_node.NodeData = time sleep for low power mode.
const int Max_Nodes = 20;

typedef struct {
  byte ID;                                  //Node ID number
  byte path [Max_Nodes];                    //The path to go down    //up to 256 differnt node names only a path of 31
  byte Place_In_Path;                       //Where in the array are we
  byte cmd;                                 //go to sleep, other odd commands
  bool return_flag;                         //Return to home node, go from ++ to --
  float sensor1;                            //set time for sleep (shared by HOME + OPNODE)
} MsgData;

MsgData My_Data;
MsgData Received_Data;


//Initializing the Data in Structs.
//These can be altered Later by using Struct_name.Struct_access

int TransAMOUNT = 5;
int DataTRANS = false;
int i;
unsigned long Timeout = 5000;
int count = 0;
unsigned long start_time;
bool old_Data = true;

/* variable for sleep state, flag*/
int8_t POST_done_flag = 0;


void setup() {
  /* Set up 2G network here*/
  client_2G.begin(19200);                   //the GPRS baud rate
  Serial.begin(19200);                      // Serial monitor baud rate
  delay(500);

  /* Set up RF radio reception here*/
  _clear_data_struct();
  My_Data.Place_In_Path = 1;             //CHANNGEE ----------------------------------??

  My_Data.sensor1 = NodeData;             //sleep time for low-power mode. Sent out to Op-Node to make sure compatibility
  Serial.begin(9600);
  radio.begin();
  radio.setAutoAck(false);
  radio.openReadingPipe(1, pipe);
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
     if 2G network available: wait for radio reception and perform duty
     else: power cycle by SW reset (power_on)
  */
  if ( client_2G.available() ) {
    while ( client_2G.available() ) {
      Serial.println("loop_Connected");
      delay(1000);
      Serial.println(" POST something ");

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
      //      for(i=1; i<6; i++) {
      //        reading[i-1] = i;
      //      }
      Submit_HTTP_request();
      Serial.println("read the website");
      Serial.println("check it out the path");
      for (i = 0; i < sizeof(reading); i++) {
        Serial.print(reading[i]); Serial.print(" : ");
        delay(100);
      }
      Serial.println("Get shit done. please run IF condition");


      /* Need to check the condition, cannot exit when reading =0 */
      if (reading[0] == 0 && reading[1] == 0) {
        Serial.println("what the fuck??? How come you get in here");
        delay(1000);
        continue;         //never
      }
      else {
        serial_logger();
      }

      /*POST data to website*/
      Serial.println("POST data to web");
      while ( POST_done_flag == 0 ) {
        Post_HTTP_request();
        Submit_HTTP_request();            // confirmation flag that data is sent successfully to web. ASK WEBTEAM to design a flag-return
        POST_done_flag = 1;               // ---------------------- dummy value until Webteam get that done.
      }
      //power_on();
      My_Data.cmd = 1;                    // sleep command
      My_Data.sensor1 = NodeData;         // sleep time. Should be reasonable

      serial_logger();                    //passing sleep command to all Sensor node.
      delay(1000);

      /* sleep mode */
      Serial.println("Enter sleep mode zzzzzzzzzzzzzzzzz");
      delay(500);
      if ( My_Data.cmd = 1 ) {
        for (i = 0; i < NodeData; i++) {
          LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF,
                        SPI_OFF, USART0_OFF, TWI_OFF);
        }
      }

      /* Sleep time is over, wake up*/
      My_Data.cmd = 0;                    //zero out sleep cmd

    }
    return;
  } else {  //perform power_on
    //If no power at the beginning, works fine. But if mannually turn off, never back on???
    Serial.println(" Loop_no_Connected - power reset");
    power_on();
    delay(1000);
    //connect_GPRS();
    Serial.println("No need enable GPRS");                //Run once
  }
}

/**
   Software reset: re-initilize 2G module (solder JP for pin9)
   This will be executed if there 2G cellular link lost connectivity
*/
void power_on() {
  Serial.println(" Software PowerUp ");
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  delay(1000);

  digitalWrite(9, HIGH);
  delay(2000);

  digitalWrite(9, LOW);                                   //This turn off the 2G module
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
  //ShowSerialData();
  Reading_Path();
  //_convert_Str_to_IntArray(readingHold[2]);

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

  /*
    client_2G.println("AT+HTTPTERM");        //terminate the HTTP, cause POWER RESET ----------------- CAUTION
      delay(1000);
      ShowSerialData(); */
}

void ShowSerialData() {

  while ( client_2G.available() != 0)
    Serial.write( client_2G.read() );
  delay(100);
}

void _CastString_to_Int_Array() {

  return;
}


void serial_logger() {
  Serial.println("enter serial_logger");
  //clear path                      // ------------------------------------------------- Do we accidentally clear data path here?
  _clear_data_struct();
  My_Data.Place_In_Path = 1;       //CHANGE ME ---------------------??

  //reading[] = {5,3,2,4,1}; populate My_data.path[] by the array
  for ( i = 0; i < sizeof(reading); i++) {
    My_Data.path[i] = int( reading[i] );
  }

  //CHANGE ME
  //    int c = int(Serial.read());
  //    if ( c == '1'){
  //       //path to 1
  //        My_Data.path[0] = 0;
  //        My_Data.path[1] = 1;
  //      Serial.println(F("Calling node 1"));
  //   }

  //     else{
  //    return;
  //   }

  Serial.print("print mydata struct: ");
  for ( int8_t w = 0; w < sizeof(reading); w++) {
    Serial.print(My_Data.path[w]);
  }
  delay(100);
  Serial.print("print sleep cmd flag: "); Serial.println(My_Data.cmd);
  Serial.println(" ");
  Serial.println("-------------");

  transmit(My_Data);
  //RECIEVE
  delay(20);
  start_time = millis();
  Serial.println("---Listening For Response---");
  while (start_time + Timeout > millis()) {
    receive();
  }
  Serial.println("Complete Transceiver. Return loop");

}


void receive() {                                                            //Recieve Data from another node
  radio.openReadingPipe(1, addresses[0]);
  radio.startListening();
  if (radio.available()) {
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

void transmit(MsgData Transmit_Msg) {                                       //Transmit Data to Another Node
  radio.openWritingPipe(addresses[0]);
  radio.stopListening();

  for (i = 0; i < TransAMOUNT; i++) {
    radio.write(&Transmit_Msg, sizeof(MsgData));
    delay(5);
  }
  Serial.println("\nTransmitted Data");

  Serial.print("ID: ");
  Serial.println(Transmit_Msg.ID);

  Serial.print("Place_In_Path: ");
  Serial.println(Transmit_Msg.Place_In_Path);

  Serial.print("Path: ");
  for (i = 0; i < Max_Nodes; i++) {
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
   Helper method to zero out the Msg_Data struct
*/
void _clear_data_struct() {
  for ( i = 0; i < Max_Nodes; i++) {
    My_Data.path[i] = 0;
  }
  My_Data.return_flag = 0;
}




/* --------------------------------------- TEMP ----------------------- */
void Reading_Path() {
  i = 0;
  while ( client_2G.available() ) {
    readingHold[i] = byte(client_2G.read());
    delay(100);
    i++;
  }

  /* Terver's idea: a temp solution for this problem. Not a generic to be used everywhere.*/
  int m = 29;
  for (i = 0; i < 11; i++)
  {
    readingHold2[i] = readingHold[m]; //readingHold2 now contains the path which is stored as bytes
    m += 1;
  }
  //_convert_Str_to_IntArray(readingHold2);    //no idea why we cannot parse byte array
  Serial.println("final path: ");
  for ( i = 0; i < sizeof(readingHold2); i++) {
    if (isDigit(readingHold2[i])) {
      //      final_path[i] = readingHold2[i] - '0';      //Should use Int() to cast?
      //      Serial.println(final_path[i]);

      reading[i] = readingHold2[i] - '0';
      Serial.println(reading[i]);
      delay(100);
    }
  }


  /* generic capture the right path, but contain some extra '1' from somewhere */
  //_convert_Str_to_IntArray();
}


void _convert_Str_to_IntArray(byte path[]) {
  // works fine, but some extra '1' from somewhere
  Serial.println("final path: ");
  for ( i = 0; i < sizeof(path); i++) {
    if (isDigit(path[i])) {
      final_path[i] = path[i] - '0';      //Should use Int() to cast?
      //received_path[k] = byte(readingHold[j]);
      Serial.println(final_path[i]);
      delay(100);
    }
  }
}

