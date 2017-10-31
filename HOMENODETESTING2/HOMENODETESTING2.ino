#include<SoftwareSerial.h>

SoftwareSerial client(2,3);


String reading="[5,3,2,4,1,6,8,0]";
int parse_int[8];

void setup()
{
  Serial.begin(9600);
  //client.begin(9600);
  delay(500);
  if(client.available()) {
    while (client.available()){
      Serial.print("Connected");
    }
  }

  Serial.print("\nlength of strign: ");
  Serial.println(reading.length());

  extractArray();
  Serial.print("\n\n Int arr: ");
  for(int8_t w =0; w<8; w++) {
    Serial.print(parse_int[w]);
  }
  /*
  if(client.available())
    Serial.print("Connected");
  else
    Serial.print("NotConnected");*/
  
  //initSIM();
  //connectGPRS();    //don't need GPRS connection for now
  //connectHTTP();
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
  ShowSerialData();
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

void extractArray() {
  int8_t k = 0;
  for (int8_t j=0; j<reading.length(); j++) {
    if (isDigit(reading.charAt(j))) {
      parse_int[k] = reading[j];
      Serial.println(reading[j]);
      k++;
    }
  }
}

boolean isFloat(String tString) {
  String tBuf;
  boolean decPt = false;
  
  if(tString.charAt(0) == '+' || tString.charAt(0) == '-') tBuf = &tString[1];
  else tBuf = tString;  

  for(int x=0;x<tBuf.length();x++)
  {
    if(tBuf.charAt(x) == '.') {
      if(decPt) return false;
      else decPt = true;  
    }    
    else if(tBuf.charAt(x) < '0' || tBuf.charAt(x) > '9') return false;
  }
  return true;
}

/* store dynamic array
void savePath() {
  char *A = malloc(size(*A) * NUM_ELEMENTS);
  for(int8_t i=0; i < NUM_ELEMENTS; i++) {
    A[i] = ...
  }
  }
}
*/
