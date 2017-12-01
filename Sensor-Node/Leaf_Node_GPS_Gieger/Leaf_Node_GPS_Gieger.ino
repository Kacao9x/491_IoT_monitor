#include <SPI.h>
#include "RF24.h"

const byte NodeID = 1;
float NodeData = 1.11;

const int Max_Nodes = 20;
byte Received_ID_Tags[Max_Nodes]; //write max number of Nodes. 
byte TransAMOUNT = 1;

RF24 radio(7,8);
const uint64_t pipe = 0xE8E8F0F0E1LL; //channel to receive
byte addresses[][6] = {"1Node","2Node"};
int delay_PASS = 100;
int delay_END = 500;





//Define Node Information
typedef struct {
  byte ID; //Node ID number
  byte path [Max_Nodes]; //Up to 256 Node names but a MAXIMUM PATH of 30
  byte Place_In_Path; //Where in the array are we
  byte cmd; //go to sleep, other odd commands
  bool return_flag;//Return to home node, go from ++ to --
  float sensor1;
}MsgData;

MsgData My_Data;
MsgData Received_Data;

int i;

void setup() {
    Serial.begin(9600);
    radio.begin();
    radio.setAutoAck(false);
    radio.openReadingPipe(1,pipe);
    radio.startListening();
    My_Data.ID = NodeID;
    My_Data.sensor1 = NodeData;
    pinMode(A0,OUTPUT);
}

void loop() {  
  receive(); //wait until we get something
  


  //If next number is -1 (end of path) AND we are the next in line
  //BACK TO HOME NODE
  if(0 == Received_Data.path[Received_Data.Place_In_Path + 1] && Received_Data.path[Received_Data.Place_In_Path] == My_Data.ID){
    Received_Data.return_flag = 1; //Return to the home node
    //put data into the send back variable
    Received_Data.sensor1 = My_Data.sensor1;
    Received_Data.ID = My_Data.ID; //sends the last node's ID number
    
    Received_Data.Place_In_Path --;     //go back a step, in the path (where is the next step)
    Serial.println("*******END NODE***** Passed Back Data");
    delay(20);//wait for Home Node to get into recieve mode.
    //if you are called, wait a WHILE before sending out information. 
    delay(delay_END);
    transmit(Received_Data);
  }
  
  //Not End Node so Pass Data
  else if(Received_Data.path[Received_Data.Place_In_Path] == My_Data.ID){
    //check flag
    Serial.println("Entered Read Area");
    if(Received_Data.return_flag == 1){ //if going back to home node
      Received_Data.Place_In_Path --;   //de-increment
      Serial.println("Passed Backwards Data");

    }
    else{
      Received_Data.Place_In_Path ++;   //otherwise, increment (follow up along path)
      Serial.println("Passed Fowards Data");
    }
    //if you are called, wait a moment before sending out information. 
    delay(delay_PASS);
    transmit(Received_Data);
  }
  else{
    //do nothing otherwise
   
  }
  

}


void receive(){                                                             //Recieve Data from another node
    radio.openReadingPipe(1,addresses[0]);
    radio.startListening();  
       //if(radio.available()){ 
          while(radio.available()){ 
            radio.read(&Received_Data, sizeof(MsgData));  //byte value
            digitalWrite(A0, HIGH);
            Serial.println("\nRecieved Data");
            
            Serial.print("ID: ");
            Serial.println(Received_Data.ID);

            Serial.print("Place_In_Path: ");
            Serial.println(Received_Data.Place_In_Path);

            Serial.print("Path: ");
            for (i=0;i<Max_Nodes;i++){
              Serial.print(Received_Data.path[i]);
              Serial.print(", ");
            }
            Serial.println("");

            Serial.print("Return_Flag: ");
            Serial.println(Received_Data.return_flag);
            
            delay(5);
         // }
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
  Serial.println("Transmitted Data");
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
}
