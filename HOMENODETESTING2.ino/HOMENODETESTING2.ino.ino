#include<SoftwareSerial.h>

SoftwareSerial client(12,13);


String reading="";

void setup()
{
  Serial.begin(9600);
  client.begin(9600);
  delay(500);

  if(client.available())
  {
    Serial.print("Connected");
  }
  else
  {
    Serial.print("NotConnected");
  }

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
  reading = "";

  client.println("AT+HTTPPARA=\"CID\",1");
  delay(1000);
  ShowSerialData();
  reading = "";

  client.println("AT+HTTPPARA=\"URL\",\"http://sensorweb.ece.iastate.edu/api/4/path\"");//Public server IP address
  delay(1000);
  ShowSerialData();
  reading = "";
//modification starts here
  delay(1000);
  client.println("AT+HTTPACTION=0");
  delay(1000);
  ShowSerialData();
  reading = "";
  
  client.println("AT+HTTPREAD");
  delay(1000);
  ShowSerialData();
//modification ends here 
  client.println("AT+HTTPTERM");
  delay(1000);
  ShowSerialData;
}

void ShowSerialData()
{
  while(client.available()!=0)
  {
    Serial.write(client.read());
    reading += Serial.write(client.read());
    delay(100);
  }
}
