#include<SoftwareSerial.h>
#include <SPI.h>
#include "RF24.h"
#include "LowPower.h"

/* data structure for the tranmission */
//unsigned long msg;
const byte NodeID = 0;                      //Node ID for home station
float NodeData = 5;                         //Home_node.NodeData = time sleep for low power mode.
const int Max_Nodes = 20;
//byte reading_temp[Max_Nodes];
//byte reading[Max_Nodes];                    //change to dynamic array then?

/* temp value to encapsulate the byte value from Op-Nodes */
byte readingHold[46];                       //this would hold the response of the AT+HTTPREAD
byte readingHold2[Max_Nodes];               //This contains only the data which is the path e.g[5,4,3,2,1]
byte readingHold3[Max_Nodes];               //This contains the final path to copy to My_Data.path[ ]    

String sensor_value;

/* Json template to populate the sensor value */
String readx0 = "[{";
String readx1 = " \"id\": ";
String readx2 = " \"value\": ";
String readx3 = "}]";
String jsonString ="";
//String failedNode = readx0 + readx1 +  nodeID + "\":" + "ERROR" + "\"" + "603" + "\"}";



/* Variables for 2G connection */
SoftwareSerial client_2G(7, 8);             //2G network pin 7:Rx (CE), pin 8(CSN): Tx, pin 9: power Up/Down for SW reset

/* Variable for RF communication */
RF24 radio(2, 3);
const uint64_t pipe = 0xE8E8F0F0E1LL;       //channel to recieve
byte addresses[][6] = {"1Node", "2Node"};

typedef struct {
  byte ID;                                  //Node ID number
  byte path [Max_Nodes];                    //The path to go down    //up to 256 differnt node names only a path of 31
  byte Place_In_Path;                       //Where in the array are we
  byte cmd;                                 //go to sleep, other odd commands
  bool return_flag;                         //Return to home node, go from ++ to --
  float sensor1;                            //sleep time(sent from HomeNode) or sensor value(sent by Op-Node)
} MsgData;

MsgData My_Data;
MsgData Received_Data;


//Initializing the Data in Structs.
//These can be altered Later by using Struct_name.Struct_access

int TransAMOUNT = 5;
//int DataTRANS = false;
int i;
unsigned long Timeout = 5000;
//int count = 0;
unsigned long start_time;
//bool old_Data = true;

/* variable for sleep state, flag*/
int8_t POST_done_flag = 0;


void setup() {
  /* Set up 2G network here*/
  client_2G.begin(19200);                   //the GPRS baud rate
  Serial.begin(19200);                      // Serial monitor baud rate
  delay(500);

  /* Set up RF radio reception here*/
  Serial.begin(9600);
    radio.begin();
    radio.setAutoAck(false);
    radio.openReadingPipe(1, pipe);
    radio.startListening();

  /* init variable */
    _clear_data_struct();
    memset(readingHold2, 0, sizeof(readingHold2));            //zero-out the array before being populated
    memset(readingHold, 0, sizeof(readingHold));              //zero-out the array before being populated
    My_Data.sensor1 = NodeData;             //sleep time for low-power mode. Sent out to Op-Node to make sure compatibility

  /* Run once to go end-to-end */
  //connect_GPRS();
  //connect_HTTP();
  //Submit_HTTP_request();
  Serial.println(" \n\n--------- Setup complete ---------- \n\n");
}

void loop() {
  /**
     if 2G network available: wait for radio reception and perform duty
     else: power cycle by SW reset (power_on)
  */
  My_Data.cmd = 0;                    //zero out sleep cmd
  if ( client_2G.available() ) {
    while ( client_2G.available() ) {
      Serial.println("loop_Connected");
      delay(1000);
      Serial.println(" POST something ");

      _clear_data_struct();
      memset(readingHold2, 0, sizeof(readingHold2));            //zero-out the array before being populated
      memset(readingHold, 0, sizeof(readingHold));              //zero-out the array before being populated
      connect_GPRS();

      /* Read the path from website */
      Serial.println("read the website");
      Submit_HTTP_request();                                    //Read path from the website
        //sensor_value = String(Received_Data.sensor1);
      
      //Debugging 
      Serial.println("MAIN LOOP_check it out the path");

      /* RF transceier enabled to parse the path */
      serial_logger();
      //value = Received_Data.sensor1;
        if( (Received_Data.path[Received_Data.Place_In_Path] == 0) ) {
            Serial.print("Data received from Serial log");Serial.print(Received_Data.ID);Serial.print(", ");Serial.println(Received_Data.sensor1);
        }
      /* Need to check the condition, cannot exit when reading =0 
      if (reading[0] == 0 && reading[1] == 0) {
        Serial.println("what the fuck??? How come you get in here");
        delay(1000);
        continue;         //never
      }
      else {
        serial_logger();
      } */
      
      /*POST data to website*/
      Serial.println("POST data to web");
      while ( POST_done_flag == 0 ) {
        Post_HTTP_request();              //relay the sensor datas to webserver
        delay(500);
        Submit_HTTP_request();            // NEW LINK: reading the confirm flag indicating that data is sent successfully to web. ASK WEBTEAM to design a flag-return
        POST_done_flag = 1;               // ---------------------- dummy value until Webteam get that done.
      }


      /* Sleep command to the same array path */
      //
      My_Data.cmd = 1;                    // sleep command
      My_Data.sensor1 = NodeData;         // sleep time. Should be reasonable

      serial_logger();                    //passing sleep command to all Sensor node.
      delay(500);
      
      /* sleep mode */ 
      Serial.println("Enter sleep mode zzzzzzzzzzzzzzzzz");
      delay(5000);
      if ( My_Data.cmd = 1 ) {
        power_cycle();                      //turn off 2G module
        for (i = 0; i < NodeData; i++) {
          LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF,
                        SPI_OFF, USART0_OFF, TWI_OFF);
        }
        My_Data.cmd = 0;                    //zero out sleep cmd
      }

      /* Sleep time is over, wake up*/
      //My_Data.cmd = 0;                    //zero out sleep cmd
    }
    return;
  } else {  //perform power_on
    //If no power at the beginning, works fine. But if mannually turn off, never back on???
    Serial.println( client_2G.available() );
    Serial.println(" \nLoop_no_Connected - power reset");
    power_cycle();                      //If module is off, turn ON
    delay(500);
    //connect_GPRS();                     //No need GPRS setup here?
    Serial.println("---- complete power cycle - go back to loop -----\n\n\n");                //Run once
  }
}

/**
   Software reset: re-initilize 2G module (solder JP for pin9)
   This will be executed if there 2G cellular link lost connectivity
*/
void power_cycle() {
  Serial.println(" Software PowerCycle ");
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  delay(1000);

  digitalWrite(9, HIGH);
  delay(2000);

  digitalWrite(9, LOW);                                   //This turn off the 2G module
  delay(3000);
}


void connect_GPRS() {
  client_2G.println("AT+CSQ");                                 //Check signal quality
  delay(500);
  //ShowSerialData();

  client_2G.println("AT+CGATT?");                             //Attach or Detach from GPRS Support
  delay(500);
  //ShowSerialData();

  client_2G.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");     //setting the SAPBR, the connection type is using gprs
  delay(1000);
  ShowSerialData();

  client_2G.println("AT+SAPBR=3,1,\"APN\",\"TRACFONE-WFM\""); //setting the APN, Access point name string
  delay(500);
  //ShowSerialData();

  client_2G.println("AT+SAPBR=1,1");                          //setting the SAPBR
  delay(500);
  //ShowSerialData();

  //  client_2G.println("AT+SAPBR=2,1");
  //    delay(1000);
  //    ShowSerialData();
}
void connect_HTTP() {
  client_2G.println("AT+HTTPINIT");           //init the HTTP request
    delay(2000);
    ShowSerialData();

  client_2G.println("AT+HTTPPARA=\"CID\",1");
    delay(500);
    ShowSerialData();

}
void Submit_HTTP_request() {
  client_2G.println("AT+HTTPINIT");           //init the HTTP request
  delay(500);
  ShowSerialData();

  client_2G.println("AT+HTTPPARA=\"CID\",1");      //HTTP://sensorweb.ece.iastate.edu/api/4/path
  delay(500);
  ShowSerialData();

  //this is a modification to obtain path from api
  client_2G.println("AT+HTTPPARA=\"URL\",\"http://sensorweb.ece.iastate.edu/api/2/path\"");//Public server IP address
  delay(1000);
  ShowSerialData();

//  client_2G.println("AT+HTTPPARA=\"CID\",1");      //HTTP://sensorweb.ece.iastate.edu/api/4/path
//  delay(1000);
//  ShowSerialData();

  client_2G.println("AT+HTTPACTION=0");    //=0 (READ), =1 (POST)
  delay(6000);
  ShowSerialData();

  client_2G.println("AT+HTTPREAD");       //Send a command to read the HTTP server response, run AT+HTTPACTION in prior
  delay(1000);
  Reading_Path();

  client_2G.println("");
  delay(100);
}
void Post_HTTP_request() {
  /* this function specifically extract only the path and not the other response retrieved from
    HTTPREAD response
    Reading and extracting the desired path from the api end here*/

  /* make a post to the server with sensor value */
  jsonString = readx0 + readx1 + String(Received_Data.ID) + "," + readx2 + String(Received_Data.sensor1) + readx3;
  
//  client_2G.println("AT+HTTPINIT");           //init the HTTP request
//      delay(1000);
//      ShowSerialData();
  connect_HTTP();

  client_2G.println("AT+HTTPPARA=\"URL\",\"http://sensorweb.ece.iastate.edu/api/homenodes/2\"");//Public server IP address where the data would be posted
  client_2G.println("AT+HTTPPARA=\"URL\",\"http://sensorweb.ece.iastate.edu/api/homenodes/2\"");//Public server IP address where the data would be posted
  //client_2G.println("AT+HTTPPARA=\"URL\",\"HTTP://posttestserver.com/post.php?dir=Homenodetestiastate1\"");//Public server IP address
      delay(1000);
      ShowSerialData();

    //add ID number here
  client_2G.println("AT+HTTPPARA=\"CONTENT\",\"application/json"); //create the data structure in json format
      delay(500);
      ShowSerialData();


  client_2G.println("AT+HTTPDATA=" + String(jsonString.length()) + ",100000"); //insert data into the json format
      delay(100);
      ShowSerialData();

  client_2G.println(jsonString);
      delay(100);
      ShowSerialData;

  client_2G.println("AT+HTTPACTION=1");    //=0 (READ), =1 (POST)
      delay(1000);
      ShowSerialData();

  client_2G.println("AT+HTTPREAD");
      delay(100);
      ShowSerialData();

  client_2G.println("AT+HTTPTERM");        //terminate the HTTP, cause POWER RESET ----------------- CAUTION
      delay(500);
      ShowSerialData();
  Serial.println(jsonString);
}

void ShowSerialData() {

  while ( client_2G.available() != 0) {
    Serial.write( client_2G.read() );
    delay(100);
  }
}


void serial_logger() {
  Serial.println("enter serial_logger");
  //clear path
  _clear_data_struct();

//-------------------- TEMP test
//    reading_temp[0] = 0;
//    reading_temp[1] = 1;
//    reading_temp[2] = 2;
//    reading_temp[3] = 0;
//    for( i=0; i<Max_Nodes; i++) {
//        My_Data.path[i] = reading[i];
//    }

    pathToSend();
    
  Serial.print("Serial_logger_print mydata struct: ");
  for ( int8_t i = 0; i < sizeof(My_Data.path); i++) {
    Serial.print(My_Data.path[i]);
  }
  delay(100);
  Serial.print("\nprint sleep cmd flag: "); Serial.println(My_Data.cmd);
  Serial.println("\n");
  Serial.println("-------------");

  transmit(My_Data);
  //RECIEVE
  delay(20);
  start_time = millis();
  Serial.println("---Listening For Response---");
  while (start_time + Timeout > millis()) {
    receive();
  }

    /* save the value to populate JSON format*/
  
  Serial.println("Complete Transceiver. Return loop");

  
  /* Display data received*/
  Serial.println(" $$$$$$$$  Display data received $$$$$$$$ ");
//  Serial.print("ID: "); Serial.println(Received_Data.sensor1);
//  Serial.print("value: "); Serial.println(Received_Data.sensor1);
  Serial.print("ID: "); Serial.println("2.0");
  Serial.print("value: "); Serial.println("2.22");

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
  My_Data.Place_In_Path = 1;

  memset(readingHold3, 0, sizeof(readingHold3));            //zero-out the array before being populated
}




/* --------------------------------------- TEMP ----------------------- */
void Reading_Path() {
  Serial.println(" value from readingHold");

  i = 0;
  while ( client_2G.available() ) {
    readingHold[i] = byte(client_2G.read());
    i++;
  }

  /* Terver's idea: a temp solution for this problem. Not a generic to be used everywhere.*/
  //memset(readingHold2, 0, sizeof(readingHold2));
  int m = 29;
  Serial.println(" ");
  Serial.println(" ---------- element after 29th----------------------");

    for (i = 0; i<sizeof(readingHold2); i++) {
        readingHold2[i] = readingHold[m];
        m++;
    }
    //pathToSend();         // No idea why this function doesn't work here.

  /* generic capture the right path, but contain some extra '1' from somewhere */
  //_convert_Str_to_IntArray();
}


/**
 * this function converts path from bytes to int and then modifies the struct with path.//////////
 */
void pathToSend() {
     //the loop below just converts the recieved from the api into a byte array which is then used to modify path in the struct to be transmitted. The string Read is just for debugging purposes
  
  int8_t b = 0;
  int8_t a=0;
  
  for( i=0; i<sizeof(readingHold2); i++ )
  {
    
    char var = readingHold2[i];
    switch( var ) {
      case 49:
        readingHold3[a] = 1;
        a++;
        break;
      case 50:
        readingHold3[a] = 2;
        a++;
        break;
      case 51:
        readingHold3[a] = 3;
        a++;
        break;
      case 52:
        readingHold3[a] = 4;
        a++;
        break;
      case 53:
        readingHold3[a] = 5;
        a++;
        break;
      case 54:
        readingHold3[a] = 6;
        a++;
        break;
      case 55:
        readingHold3[a] = 7;
        a++;
        break;
      case 56:
        readingHold3[a] = 8;
        a++;
        break;
      case 57:
        readingHold3[a] = 9;
        a++;
        break;
      case 48:
        if( a < 1) {
            readingHold3[a] = 0;
            a++;
            break;
        }
        else {}
    }
    /*
    if(readingHold2[i] == 49)
    {
       readingHold3[a] = 1;
       a+=1;
    }
    if(readingHold2[i] == 50)
    {
       readingHold3[a]=2;
       a+=1;
    }
     if(readingHold2[i] == 51)
    {
       readingHold3[a] = 3;
       a+=1;
    }
     if(readingHold2[i] == 52)
    {
       readingHold3[a] = 4;
       a+=1;
    }
     if(readingHold2[i] == 53)
    {
       readingHold3[a]=5;
       a+=1;
    } 
     if(readingHold2[i] == 48 && b<1)
    {
       readingHold3[a]=0;
       a+=1;
       b +=1;                           //ensures that 0 is written only ones in the path
    }
  } */
  }
  //b = 0;
  //nodeID = String(readingHold3[4]);     //this holds the value of the requested node
   //The data type path in my_Data below is modified so it contains the desired path to be sent
  
  for(i=0; i<sizeof(readingHold3); i++)  //The reason for using 5 is that the api test path is 3
  {
      My_Data.path[i] = readingHold3[i];
  }
}

/*
void _convert_Str_to_IntArray(byte path[]) {
  // works fine, but some extra '1' from somewhere
  Serial.println("Final path read from casting: ");
  for ( i = 0; i < sizeof(path); i++) {
    if (isDigit(path[i])) {
      reading[i] = path[i] - '0';      //Should use Int() to cast?
      //reading[i] = byte(path[i]);
      Serial.println(reading[i]);
      delay(100);
    }
  }
}
*/
