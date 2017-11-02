#include<SoftwareSerial.h>

SoftwareSerial client(2,3);


String reading ="";
byte readingHold[46]; //this would hold the response of the AT+HTTPREAD  
byte readingHold2[11]; //This contains only the data which is the path
int a =0; //this would hold the value for byte
//String reading="[5,3,2,4,1,6,8,0]";
int parse_int[8]; //consider using dynamic array for optimal runtime


void setup()
{
  Serial.begin(9600);
  client.begin(9600);
  delay(500);

  Serial.print("\nlength of strign: ");
  Serial.println(reading.length());

  
 if(client.available())
  {
    Serial.println("NotConnected");
  }
  else
  {
    Serial.println("Connected");
  }
  //initSIM();
  connectGPRS();
  connectHTTP();
   // extractArray();
  Serial.print("\n\n Int arr: ");
  for(int8_t w =0; w<8; w++) {
    Serial.print(parse_int[w]);
  }
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

  client.println("AT+HTTPPARA=\"CID\",1");      //http://sensorweb.ece.iastate.edu/api/4/path
  delay(1000);
  ShowSerialData();
  //this is a modification
  
  client.println("AT+HTTPPARA=\"URL\",\"http://sensorweb.ece.iastate.edu/api/4/path\"");//Public server IP address
  delay(1000);
  ShowSerialData();

  client.println("AT+HTTPACTION=0");
  delay(1000);
  ShowSerialData();
 
  client.println("AT+HTTPREAD");  //in this line of code we are trying to store the sensor data
  delay(1000);
  ShowSerialData1(); //this code is in progress
  //the test code starts here
  Serial.println("");
  int l=0;
  for(l=0;l<11;l++)
  {
    Serial.write(readingHold2[l]);
  }
  //end of the test code
  
  client.println("AT+HTTPPARA=\"URL\",\"http://posttestserver.com/post.php?dir=Homenodetestiastate\"");//Public server IP address
  ShowSerialData();

  client.println("AT+HTTPPARA=\"CONTENT\",\"application/jason");
  delay(1000);
  ShowSerialData();


  client.println("AT+HTTPDATA=" + String(reading.length()) + ",100000");
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

void ShowSerialData()
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
void ShowSerialData1() //this function would be used to store the character read as a string
{
  int8_t i=0;
  while(client.available()!=0)
  {
      readingHold[i]= byte(client.read());
      i+=1;
  }
  //parse the path value and store into an array
  //WARNING: do this function simutanuously with client.read may cause missing character
    extractArray(readingHold);
  
  //inorder to understand this line of code, we have to understand the exact response of the AT+command Httpread
  /*
   int n = 0;
   int m = 29;
   for(n = 0;n<11;n++)
   {
      readingHold2[n] = readingHold[m];
      m+=1;
   }*/   
}

void extractArray(String reading) {
  int8_t k = 0;
  for (int8_t j=0; j<reading.length(); j++) {
    if (isDigit(reading.charAt(j))) {
      parse_int[k] = reading[j] - '0';                  //value in string need to convert into int. Should use Int() to cast?
      //Serial.println(reading[j]);
      Serial.println(parse_int[k]);
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

