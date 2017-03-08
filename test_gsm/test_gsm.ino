#include <SoftwareSerial.h>

SoftwareSerial GPRS(7,8);
char buffer[64];
int count = 0;

void setup() {
  // put your setup code here, to run once:
  GPRS.begin(9600);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  //check if therer is a signal from the output port
  if(GPRS.isListening()) {
    Serial.println("working !!!");
    delay(3000);
  }
  else {
    Serial.println("no signal");
    delay(3000);
  }

  
  while(GPRS.available())
  {
    buffer[count++] = GPRS.read();
    if(count == 64) break;
  }
    Serial.write(buffer,count);
    clearBufferArray();
    count = 0;

    if (Serial.available())
    {
      byte b = Serial.read();
      if(b == '*')
        GPRS.write(0x1a);
      else
      GPRS.write(b);
    }
}
void clearBufferArray()
{
  int i;
  for(i=0;i<count;i++)
  {
    buffer[i]=0;
  }
}

