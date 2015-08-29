/******************************************************************************
LinkSprite_Cam
Ryan Owens @ SparkFun Electronics>
Revised by Joel Bartlett on 03/25/15 for compilation on Arduino 1.6+

This code allows you to control the LinkSprite IR Camera (SEN-11610) with an Arduino microcontroller

Development environment specifics:
Arduino 1.6.0

This code is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
*********************************************************************************/
#include <SoftwareSerial.h>
#include "Camera.h"

Camera test(4, 5);
/* Linksprite */
byte incomingbyte;
//SoftwareSerial mySerial(4, 5); // RX, TX         //Configure pin 4 and 5 as soft serial port

int a=0x0000,j=0,k=0,count=0;                    //Read Starting address
//uint8_t MH,ML;
//boolean EndFlag=0;

void SendResetCmd();
void SendTakePhotoCmd();
void SendReadDataCmd();
void StopTakePhotoCmd();

void setup()
{
  Serial.begin(19200);
  //mySerial.begin(38400);
  test.Setup();
}

void loop()
{
  test.SendResetCmd();
  delay(4000);                               //After reset, wait 2-3 second to send take picture command

  test.SendTakePhotoCmd();

  while (test.Available() > 0)
  {
    incomingbyte = test.mySerial.read();
  }
  byte a[32];

  while (!test.EndFlag)
  {
    j = 0;
    k = 0;
    count = 0;
    test.SendReadDataCmd();

    delay(25);
    while (test.Available() > 0)
    {
      incomingbyte = test.mySerial.read();
      k++;
      if ((k > 5) && (j < 32) && (!test.EndFlag))
      {
        a[j] = incomingbyte;
        if ((a[j - 1] == 0xFF) && (a[j] == 0xD9)) //Check if the picture is over
          test.EndFlag = 1;
        j++;
        count++;
      }
    }

    for (j = 0; j < count; j++)
    {
      if (a[j] < 0x10)
        Serial.print("0");
      Serial.print(a[j], DEC);
      Serial.print(" ");
    }                                       //Send jpeg picture over the serial port
    //Serial.println();
  }
  while (1);
}










