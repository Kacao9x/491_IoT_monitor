#include <SPI.h>
#include "RF24.h"

RF24 radio(2,3);
const uint64_t pipe = 0xE8E8F0F0E1LL; //channel to recieve
byte addresses[][6] = {"1Node","2Node"};
//unsigned long msg;
const byte NodeID = 1;
float NodeData = 0;
const int Max_Nodes = 20;

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

   //byte path[Max_Nodes];
//Initializing the Data in Structs.
//These can be altered Later by using Struct_name.Struct_access 

int TransAMOUNT=5;
int DataTRANS=false;
int i;
unsigned long Timeout=5000;
int count=0;
unsigned long start_time;
bool old_Data=true;


void setup(void){
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
    pinMode(4,INPUT_PULLUP);  
    pinMode(5,INPUT_PULLUP); 
    pinMode(6,INPUT_PULLUP);
}


void loop() {
  // put your main code here, to run repeatedly:
  serial_logger();  //decide what to transmit

    if((Received_Data.return_flag == 1)&&(Received_Data.path[Received_Data.Place_In_Path]==0)){
      Serial.print("Data recieved: "); Serial.print(Received_Data.ID);Serial.print(", ");Serial.println(Received_Data.sensor1);
      old_Data=true;
    }
 }

void serial_logger(){

    //clear path
    for(i=0;i<Max_Nodes;i++){
      My_Data.path[i]=0;
    }
     My_Data.return_flag=0;
     My_Data.Place_In_Path=1;

    int c = int(Serial.read());
    if ( c == '1'){ 
       //path to 1
        My_Data.path[0] = 0;
        My_Data.path[1] = 1;     
      Serial.println(F("Calling node 1"));
   }

    else if ( c == '2'){ 
        //path to 2
        My_Data.path[0] = 0;
        My_Data.path[1] = 1;
        My_Data.path[2] = 2;   
      Serial.println(F("Calling node 2"));
   }

       else if ( c == '3'){ 
        //path to 3
        My_Data.path[0] = 0;
        My_Data.path[1] = 1;
        My_Data.path[2] = 2;
        My_Data.path[3] = 3;
      Serial.println(F("Calling node 3"));
   }
        else if ( c == '4'){ 
        //path to 4
        My_Data.path[0] = 0;
        My_Data.path[1] = 1;
        My_Data.path[2] = 2;
        My_Data.path[3] = 3;
        My_Data.path[4] = 4;
      Serial.println(F("Calling node 4"));
   }
     else{
    return;
   }
   transmit(My_Data);
    //RECIEVE
    delay(20);
    start_time=millis();
     Serial.println("---Listening For Response---");
     while(start_time+Timeout>millis()){
      receive();
     }
  

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

