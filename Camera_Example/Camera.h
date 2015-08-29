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
#include <Arduino.h>
#include <SoftwareSerial.h>


class Camera
{
  public:
    SoftwareSerial mySerial;
    int a=0x0000;//j=0,k=0,count=0;                    //Read Starting address       
    uint8_t MH,ML;
    boolean EndFlag=0;
    Camera(int RX, int TX)
      : mySerial(RX, TX)
    {
    };

    void Setup()
    {
      mySerial.begin(38400);
    };
    bool Available()
    {
      return mySerial.available();
    }

    
    //Send Reset command
    void SendResetCmd()
    {
      mySerial.write(0x56);
      mySerial.write((byte)0);
      mySerial.write(0x26);
      mySerial.write((byte)0);
    };

    //Send take picture command
    void SendTakePhotoCmd()
    {
      mySerial.write(0x56);
      mySerial.write((byte)0);
      mySerial.write(0x36);
      mySerial.write(0x01);
      mySerial.write((byte)0);
    };

    //Read data
    void SendReadDataCmd()
    {
      MH = a / 0x100;
      ML = a % 0x100;
      mySerial.write(0x56);
      mySerial.write((byte)0);
      mySerial.write(0x32);
      mySerial.write(0x0c);
      mySerial.write((byte)0);
      mySerial.write(0x0a);
      mySerial.write((byte)0);
      mySerial.write((byte)0);
      mySerial.write(MH);
      mySerial.write(ML);
      mySerial.write((byte)0);
      mySerial.write((byte)0);
      mySerial.write((byte)0);
      mySerial.write(0x20);
      mySerial.write((byte)0);
      mySerial.write(0x0a);
      a += 0x20;                          //address increases 32£¬set according to buffer size
    };

    void StopTakePhotoCmd()
    {
      mySerial.write(0x56);
      mySerial.write((byte)0);
      mySerial.write(0x36);
      mySerial.write(0x01);
      mySerial.write(0x03);
    };
};





