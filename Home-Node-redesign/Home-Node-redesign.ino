#include<SoftwareSerial.h>
#include <SPI.h>
#include "RF24.h"

String reading = "";
/* Variables for 2G connection */
SoftwareSerial client_2G(7,8);             //2G network pin 7:Rx, pin 8: Tx, pin 9: power Up/Down for SW reset

/* Variable for RF communication */
void setup() {
  power_cycle();
  delay(5000);
  /* Set up 2G network here*/
  client_2G.begin(19200);                  //the GPRS baud rate
  Serial.begin(19200);                    // Serial monitor baud rate
  delay(500);
  connect_GPRS();
  connectHTTP();
  
  if( client_2G.available() )
    Serial.print("Connected_setup");
  else
    Serial.print("NotConnected_setup");
  /* Set up RF radio reception here*/

  /* Run once to go end-to-end */
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

      connectHTTP();
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
  check_signal();
  client_2G.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
    delay(1000);
    ShowSerialData();

  client_2G.println("AT+SAPBR=3,1,\"APN\",\"TRACFONE-WFM\"");//APN
    delay(1000);
    ShowSerialData();

  client_2G.println("AT+SAPBR=1,1");
    delay(1000);
    ShowSerialData();
  
  client_2G.println("AT+SAPBR=2,1");
    delay(1000);
    ShowSerialData();
}

void Submit_HTTP_request() {
  check_signal();
//  mySerial.println("AT+CGATT?");    //attach GPRS 
//    delay(100);
//    ShowSerialData();
    
  client_2G.println("AT+HTTPINIT");
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

  client_2G.println("AT+HTTPREAD");  //Send a command to read the HTTP server response, run AT+HTTPACTION in prior
    delay(300);
    ShowSerialData();              //this code is in progress
}


void Post_Http_request()
{
  /* this function specifically extract only the path and not the other response retrieved from
  HTTPREAD response
  Reading and extracting the desired path from the api end here*/

  /* make a post to the server with sensor value */
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

  client_2G.println("AT+HTTPREAD");        //Send a command to read the HTTP server response, run AT+HTTPACTION in prior
    delay(1000);
    ShowSerialData();

  client_2G.println("AT+HTTPTERM");        //terminate the HTTP
    delay(1000);
    ShowSerialData();
}

void check_signal()
{
  client_2G.println("AT+CSQ");
  delay(100);
  ShowSerialData();
}
