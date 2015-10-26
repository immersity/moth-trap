/* SM5100B-GPRS
 * this Arduino program sends an HTTP request over GPRS using the SM5100B GSM shield
 * author: http://toby.is
 * more info: http://github.com/tobek/SM5100B-GPRS
*/

// include the SoftwareSerial library to send serial commands to the cellular module:
#include <SoftwareSerial.h>

SoftwareSerial cell(2,3);  // Create a 'fake' serial port. Pin 2 is the Rx pin, pin 3 is the Tx pin. connect this to the GSM module

const String apn = "imovil.entelpcs.cl"; // access-point name for GPRS
const String user = "entelpcs";
const String password = "entelpcs";

const String ip = "54.94.157.69"; // IP address of server we're connecting to
const String port = "8082";
const String host = "54.94.157.69"; // required in HTTP 1.1 - what's the name of the host at this IP address?
const String request = "POST /api/users HTTP/1.1";
const String from = "mt0001@immersity.cl";
const String useragent = "ImmMothTrap/1.0"; // for our purposes the user agent doesn't matter - if I understand correctly it's helpful to use something generic the server will recognize
const String contentType = "application/json";
const String contentLength = "5";

/* this will send the following packet:
 * 
 * GET /m/testpage.php?data=testing HTTP/1.1
 * Host: avantari.co.uk
 * User-Agent: Mozilla/5.0
 * 
 * this is the equivalent of visiting http://avantari.co.uk/m/testpage.php?data=testing
*/

void setup()
{
  //Initialize serial ports for communication with computer
  Serial.begin(9600);
  
  Serial.println("Starting SM5100B Communication...");
  cell.begin(9600);
  
  waitTil("+SIND: 4"); // keep printing cell output til we get "+SIND: 4"
  Serial.println("Module ready");
}

void loop()
{
  Serial.println("Attaching GPRS...");
  cell.println("AT+CGATT=1");
  waitFor("OK");
  
  Serial.println("Setting up PDP Context...");
  cell.println("AT+CGDCONT=1,\"IP\",\""+apn+"\"");
  waitFor("OK");
  cell.println("AT+CGPCO=0,\""+user+"\",\""+password+"\", 1");
  waitFor("OK");

  Serial.println("Activating PDP Context...");
  cell.println("AT+CGACT=1,1");
  waitFor("OK");
  
  Serial.println("Configuring TCP connection to TCP Server...");
  cell.println("AT+SDATACONF=1,\"TCP\",\""+ip+"\","+port);
  waitFor("OK");
  
  Serial.println("Starting TCP Connection...");
  cell.println("AT+SDATASTART=1,1");
  waitFor("OK");
  
  delay(5000); // wait for the socket to connect
  
  // now we'll loop forever, checking the socket status and only breaking when we connect
  while (1) {
    Serial.println("Checking socket status:");
    cell.println("AT+SDATASTATUS=1"); // we'll get back SOCKSTATUS and then OK

    //waitFor("+STCPD:1"); // This is because the server return data when a connectio is established

    String sockstat = getMessage();
    waitFor("OK");
    if (sockstat=="+SOCKSTATUS:  1,0,0104,0,0,0") {
      Serial.println("Not connected yet. Waiting 1 second and trying again.");
      delay(1000);
    }
    else if (sockstat=="+SOCKSTATUS:  1,1,0102,0,0,0") {
      Serial.println("Socket connected");
      break;
    }
    else {
      Serial.println("We didn't expect that.");
      cellOutputForever();
    }
  }
  
  // we're now connected and can send HTTP packets!
  
  String image = returnImage();
  String data = "{\"image\": \"" + image + "\"}";

  int packetLength = 68+data.length()+host.length()+request.length()+useragent.length()+from.length()+String(data.length()).length()+contentType.length(); // 26 is size of the non-variable parts of the packet, see SIZE comments below
  
  Serial.println("Sending HTTP packet..." + packetLength);
  cell.print("AT+SDATATSEND=1,"+String(packetLength)+"\r");
  waitFor('>'); // wait for GSM module to tell us it's ready to recieve the packet. NOTE: some have needed to remove this line

  cell.print(request+"\r\n"); // SIZE: 2
  cell.print("From: "+from+"\r\n"); //SIZE: 8
  cell.print("Host: "+host+"\r\n"); // SIZE: 8
  cell.print("User-Agent: "+useragent+"\r\n"); // SIZE: 14
  cell.print("Content-Type: "+contentType+"\r\n"); //SIZE: 16
  cell.print("Content-Length: "+String(data.length())+"\r\n\r\n"); //SIZE: 20

//  cell.print("{\"image\": \"data:image/jpeg;base64,/9j/4AAQSkZJRgABAQAAAQABAAD/2wBDAAYEBQYFBAYGBQYHBwYIChAKCgkJChQODwwQFxQYGBcUFhYaHSUfGhsjHBYWICwgIyYnKSopGR8tMC0oMCUoKSj/2wBDAQcHBwoIChMKChMoGhYaKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCgoKCj/wAARCADwAUADASIAAhEBAxEB/8QAGwAAAwEBAQEBAAAAAAAAAAAAAQIDAAQFBgf/xAA3EAABBAECBAQEBQMEAwEAAAABAAIRITEDQQQSUWEicZGhBRMygUKxwdHwFFLxFSMz4QZikqL/xAAYAQEBAQEBAAAAAAAAAAAAAAABAAIDBP/EACARAQEBAAIDAQEBAQEAAAAAAAABEQIhEjFBUWEDE3H/2gAMAwEAAhEDEQA/APzf/wAL+CM47UdxmsZ0dDUDQ2J5nZnywvvSA1pAMKPw/g9H4fwWnw/DjwN3NyeqsTkArx8uW1pN09TKk8WYJidyqOxKmY9UZgc2tpte0hzQdlyaPw3T/qGvaYK73RB6rCiDOE6VGy2RJrCbaOaFnEObI3paahQNzGM+6IdDcpJzO5WJo/qo6pz5v3WLv/ZSHugSjIFOas1KAJszXmk37It3VCdpd1Tcxg+JTJiVMvuZ3TAvzHqc9VJzzH1e6AcTI+6Rxyqwg52TPukc89SgSkJtWAeYwfEkLv8A2QJ7pScqOi4mDZ9UpNC6QJQJ9lIZNwTlKXGDaAOeqU4tURuY45li6vqKnKwKkpJg3ZUHE3JKr1UtUSCVIA414li4xkqd42RJrKsWnmslS1CQRacms0pvwfVX/qAurJrCRxMm/wBFiazlAnHdOJjYyVN4zdQmOEhzEqUpHHv7qbyb8Rgp3CoJU34TFX6EdzNhTO63MISE0ei54AcaxSm4i05NQpmOm6pCR0WlMWmdgqfMAEp0aThBad8ICbtSDsEKjzLQ4YOVfxMTugSFgaSqkB5GVhiZSiPutzKw6cx9kpeAJhTL5BCnzV3RiU5y4TMSlmo7pOby9UOYHsE4FQ4QZTSCO3mozAJ/VEajYifZXiRdvalNFO6MzWftKk4jrjdXaY7oE2lLhYSkg7zF5ViEnKUlDmEG7Sm/t3VhGRf5pCc5RIoD7G+3+UjgY6TYrZWISVppI6YG6wPmrApP8lAkXO6SRC26s+orsx+iBNE/qi64KScyPdOJjgomIlLXXKEjurEQ1uhNUUXxMhITakFXHVBxgoHfqgceSvFAd0rwDITOIiZOyU2DtfmmRPuZEH9kpItfM6n/AJM54I0uGAcRRc/9F5ut8V+J69f1TNESf+LT/dM4VSPtS4TlI40SV8V874jqGXfE9URjlY1v+Vj8x4LdTiuKd0/3SEz/ACq6fZFw6j1U3ua0eI0vl9PUOk0NYXQBVysXueBJspn+X9T6B3GcOwf8jaoX+SDfimi0ObJK+X4t+ozhy7R5Xak0CO6bRLzpzqxzwCYwtf8AKRPov9VYBTHTkz+imfjGQNP/APX5rxvDyiklcuN1eEhe3qfGHVy6Yxklc5+K65/tgxtuvO2Ej2S3H3jKvGJ6n+p6v9rJGPDupHjdcyefPZcbbCJgNyrxjcdbuJ4gmS90KY4rXDZ+Y+qz+ajzkjl5zywlMcpvdWH0u/i9Zxt7/WEG62oMvfdZUB9Uz3RGAZukWRLHX1ZI53+q3z9YR4n9MqJiTfZB0QLR0VzxGrH1vyN044rWB+silzVBE2tVWqHFxxWrUuMDsq6fHaunJ5Wkd2yuLbK1SZOUZA9dnxDSe8h/DaIb+GJrytdPxnW0NPjGHgmNbwupo6Wq0DxcvNptLm+vMvniSN/5KtzTpMuhXurxjOO93EBzSIs9kkri0HgakTldZPlKzmdM8hJsnZMp1JWBb90SBSQdqKm+vsiSIzaD/pF4Si9lsg9kpjbCWQrBouIM1hJlpTOIghTJEHrCTvTOxKXY1uiSI8lMkQIwkCTUikrrvosI+2yWQrE8/SIrOF0sNBcukcWrsNDxLvrVdDcGAnJs+SkDmzlO2TI3lac9PUwSiIkeSUYKbsSr2foHyRBz+yV2G3v1QJ8Thfos2JU7WkqDfZNUijSWDJoi0NaeRdmUlcsTcphNVe6V02J7q02CCJFFExyGkoPiA6dk2xErLU9BXRYkcp81nbWl2Im1E4u491iRE90tCLR/CbOY+6yYJmfJEk7DokJAnKJIg52QTDKEztSFd8rEisoRoo0gd6QrqVjP7K+gHfSPNPpkf09CwSpOxvlU0iPkGzlKo6cDUldn32XDpkfNAkrrOMWs3dY5HJqEpIgpSRv5JZ6FDKsiO6wN5SB3cozWVRA+jG6QGj1lMbHspHJqLTUcOqIwkf5St+XnCByD/wBql+BidklEWFpEYQKpUV1SNkCRmUT0CUuG+FrVjztMxFLpYZDaC45sWuvRgxnK78WuS7YxVpwRGUjcxCo2YW3MwiCtvhYg3fkmP1Tzf5RfRgC2438kNzicwmBHLmyUhgF1LPs0cx4ljEG/dCbHhzaOZMIsXESRA6pSBBzKczVhKcOk+yMbLIkQN0xiHGElz3TC2u8SMHETE4W3NbpelnCLcmzaGjyYFQEATDuq0irWq7WdbacYWJMVCxiRe6xgTZUmGRhb8PktI6HK1RImJ6qibt+iPS0tTCJiPus4gg8u3ojo/wDE6xskdgUYT6P0PB7bLTNEfULFLoJndco+oDeSrk1hZs7ZsMTWUpNIfdDt6owHB3RkQpAppwr6jA3ZvySHzRJjf3Wfg2rASRZQeREzhAkQUDF9FRASIyhI+6EiMpZsx+ScBiQaSOiSeiwMZNLPcK+ycUeYXWNl2cOZY215WjqbQvW4NhDBS9HBr/RdkQLVY8G8pGAwKVRQ9lpzYQZELD6hWyDtRrObmMV091mva8S10gfbZFmwmuBe9JHAwZKIcHaYIsElBzmuc9oEuDR/hG418Ifw3v8AonEcpvspargwA8u4W0zqkEkNEQi7jM9rmKtIYgiEwcSGuAspNdzm6Ty0CR+yNrrPQGKolM0gB1UhLiATHooazi0wLkhDKwIJFKjPxCFznUcOVjBLif5+ibxNdbpmjSq1FDqNaACQCcIh3MSB+S5tVzTrsbyg94TaDidfVDCflgAA97pGdGV0DAO5RMyQuZridZwOw6KzqBPQeyPTRhsaohHZ37Lg0NTVfot1H31raV2ab+dgIb5yi9I5mfIJelrbgxKI+kCMFG9IrsR39k+jADwdkjsGuibSPid4dkyjGqRJVSRPLupXVKhmTVot7ZvpvzSk0bpFxuISOyVndYw9Tmks5Qnt+iE5xlJUmYMeaJPhSDMQhJ+5RoAmjRlCaRcbmKKQ+W6VgFYQSY/JB+yQk9KSDEn7JScjdBxMTFJXEifsmVSPO4XhS/W5j9LRa9nS0wAOsKuiwQIZsq8plpAXonHIzy5eVRMN05G2FtVoJ0xE2D7I8QXt05YA53MIE5vCzfm8zRqcoANR2CsDnAY7i9UOaPA0ET13j3VtYgaUgCA3yUmlg4niXarmNa0ANJrZHjCNTQaGEw4iKRnbUW0xy6LBWAtoCDqULIJOZpAP03N5WvDi0WAbFKHDcTpEOaHEuD3CBcDmMSs2a0Gu3m4nSaCLMneun5KuOaTtClrw3idJ5ENsHzRJ+ZqFrRQMn9le4xmVZoAY2xNFT4mDw+oAb5SR6K18sRv1Wdg1mh6ZWMdZ6cruI026WkYMuAJGdkhLX8Q/+1oAUxw2p/S65cA7W5j8uDkZAVtHTexkahl5HiPUreRmtwwBkx3VTDtZoG1n+dVzsJY93egI2XTpMc0EuI5nWVixri8/VDzx+q5uGgS3cr0dPkayNMN5Yr91HR0Dp6mrq8888DypWa1rZI3+6OV1qRPRHj1TIsjtsn1zOjq2PpIPoos4YB2q/wCbqeN0wDt/ArP0mO0X6cvhzSJnBtCzpz6OsDwmk3T5XucKAK6dLT+XpBs3upcHw2nwnDaek0Tyt5SYzSvXQ4Rf4f6Iy25C1RZwtsBGyxxELJCvFe62jHMZK3X9ltOfmeQWgNRG6Y9Y80Dmx/hF/wDPNZFLIgoV91nTyxPmlJ2RjmaeyBiEpKxKThhACIIj/tTB/hKYGgoGJFCFPbEpqrySOqfZH1YDsBIYJnunP0kdFM5Wg0+vmlOcWEZuZ8kDt1pIelpRyjxFOIgZzCGgD4elJ4PbK9PbCWvphzWxsQT9jhMWt5pi9lQT1qUhH1WEWUxJ2m3nLuQTGUNRgcWczBAsSqkXE+6Qj6fFv1hZsbhWN5WFrQABgJrnZYcsG/dAgcwJQUtRofp8rojoQg0coowI6J3AcrjukdHPhWUUzcRKaBzC8qQpsRU7qn4vss2VvjeiH6e6V0c59FQ4NDKR+Qf1RJ2eXpNoHLO+6qc4SAZvKoMtMhNlHBvwmlhlCBZlE5BlYytgZh2JTCaMiUsC7RrYqywwTj7o3zZhLAutwiReN1WJulo9byUIoUmdN4WcRah32S6UfMTR0St/5BfutSUG60tqSI6LO3/eENb8Pks3jaORZPdCbyl3MHyQqxKpxrBzlA4lLVhY4Fqyod0Q7I2CnGURn+FVlR5NnZZ9mZ80n3wtU+avGgJkROEvW5Q3H+FjRlawaAxP6oHr1ytUdvNAxInalZU9bSFAXCq4ATAObUtOYyrkGKNyvVrkmcY7IEUays6QCZ9kXYzmkWksGfpQIMCsFEx4bSEgADm6FGtQIIJrb2TEOonrCUxJs90JFWZnzRrTPH1Kbh4gZ3RcR4sk0gY8NGFai3BE7phBiSkP4vCnafpEItPESB1SOAgWUZMGoWJOT+azL9bvpIASatOI6GjKU04nGERMgnCbdZ49VSBB6+aEdt1jk2t+EXSzroLhbgAsJ8PmEOto5I67I1DFErEGb9Up3RuqyrS1QiRRzCANCltyNkWgetVKRsfMbW6p3nf2Umx8xt7plBz+Ku2Uurhpz90xIkx0pJq/S3qi3scvSe3khHSYW/hRbHfKpWBiohCysZ6rcykzgIPbuskLvb9kpdapUY79ViQAk5q7rA13UD7T+qR1HsjPgMpHEA4T6Gig4EyP1Sg49liZ6p36ntMAg1atUEQpMwfFlM7fxLtjmz8kgeqQ+Wyd29pDEDxIuNRj9IrCUkwa7o2Wm0jhnxbLLUMJk0OiUkwB36LT4s2KQHSUNMQbNQUhP03QTuiTnClXQ5RUzpk3nsgDQkovAk1aTYVvKr6UPubqFpETPRY+SFwa/VZb+EdHNIx5rAt5RnPRF8hyS4A6Fa9xj1VhnC20Qg3sVhEZWNjpOzA3CwmP+0KnK2wz0QTGxt6rGKk3KTc9UwyIHqqESWrGLqaCBFErE99kahHVTw5icXlTf9bTFymWCqkm8Sk1XCG43TGJrp1UtcRptPmi5WaUuE/dIH79+qnzdVicHdUjChdeSErnCjtKQmblB5wtI7nQZmZ2STXZAGSbWnqgDInsiCLzKQn1WBv91dJUO9Ak1N6MA+aEhDVwj6mDphabU2mgmkEYv1Wg91luxsqnuApaeBmwrQC0ZGy9GfHMjs436pGzAJCd/mZSWW53Rhnpvw4pK6Q4ivVaRcz6rPgnfKzWpSjm8Jj3Wsg49UtQbRMTk2jGozp6KcHllOIMEhTIA6owiZzIhJcG6TfavJI8RzZyrALTYuKTVyuElI2eie7ELPjNbl0jo5oU27qrrIxKndiRBTgp2RcBETBACDM53RwD+izY1L0bfCAoLVKwGaNQrCbeygNhJlEg1VrbQjE0ACPRE7/ZAZWqZlGHWAMBT1Mt81UisKWoLaPSlqQKHJKlxX0s63gqu8KXFx8rT6glGdi+nG4+fqgT6LOi78kpj8kzi5nwfdK4iCUCd8JSRewVna0/2SuSyZytNZwrxWmnEoTWe2UkwUA68qxKgiB+6DzSnz9Sg5xg3lM4jTA0CmBwuPS1QRcTKvpuyT1TifTM8/uqtB5ZXOJg1YIV2/STHdd76ck3iPUpLggKmoM0p7FDU9MM4QcCQPPqtg3hB2BlFahBMnFFMcgUEhyaKNSjMaG+UGUrm+IrSOTCXD8LOEHTA3hAiyJW3wc9EDM/Tt5KzoYU/hVKgicqc1SdkycYRjXFiBWZUjHMaVSDyqRnmN9FRcjMrZPua91MGs0E485lFh4m32QiW5yg2JHZEEyUfGjfdaBaHS0p1Gii8TlFRxNwEbroonX02yeaT5d0juM0xMNPknFvTo7JH5FVK5/607NCk7itQnP8hXiz5Y9IgXRx5Ll476dOqJ6rn/qtSYm8YUn6r3/USUTiLdO7EwkOfv1WJPXH2SONStYzRLpd/wBIEgjskJ7+yUuN9k50NO6ClkTjdA2EvZSMXD7JZJMYWJEHr2SlwAkxCcSjQCBzJXuogdFB+q51Mnzwtw2k7iHBrHy2JLhYynOuwg/k5pDiDMqnzhPeV6Orwmh8lrCLFg91y/6XzNhr3AntKJYq+qaWiRNqrNRkGXrjJ6FIcLP/AEo8HfqOacOBCmQP7guMusWiXGvNPnVjpI8QtAmjBH5rm5o3qVprPks+VakWOCZ26oDax2UJ2lDm713T5F0nESIKQ5yoh1G0ebpsi8qlDFglIXNnKkTv1SkwJCtSvM2IndEajMlc5JiOiAyrV3HSdRsGAkdqM6HplRJjdIXHqpW6sdYNFN9Uv9U64Hsud53Sgn+UpTp0niNTE0pu1tSSCYPn2youdWUpPczsrBtVe4kzmEkwcfolDq9kC68rX8BxHRA4mEhJO6a7KyW3wt/hYCjdoEAZdupNtG4WJMTGL6IAjM15oFwnPukYoDsBdbobGknOAMpXa23dRwzvQJSYKjr6zWOHzNRmnUjmdC4eI+KaDAfll+q70C1ONvpY9Nz4/kKbiAOZwAb1JheHq/FOIef9rl0weglc5+drGdR73n/2dK1P8qnsa/xDh9Nx8ReQMMH5ri1fiWq9wGlptYO9lRZwbnOAkkuqgvovhfwdmhy6uveoIPKb5fNV8eMHbk4D4Zr8YRq8a5w0shgqf5S+hZw+npsazTDWMaIACVxMQDA3MpHNM/UuVtqdGnpaYPVU8ImIXE1rrPMUOR53MrKeiY6KfWfuqTEhI7sVraQ7dUED50sSIKdTE+azsJa+4KBz5q7Ai1j7oCJ80HH1R/To0gT5wlBERARJEFXa0wwUjhRpMTX3SjBq1doqwIgIuIjupkwqHWJnf3Sz37whIjCUmSkA6QJtJIAlMSDn8khifyTNAkihv5rEeIGaCTbFLNfZuoT3CqGtJA5kztHoVzud4HH+21P+sa1osIy1OktLYoz5ps0uQceIBePDic/dc+p8Y0APqaDhOWp6PLXbzU3GJE79V4ur8aqNNjney5n/ABHi9SOSGX+Fsrc4clr33PAbNBvWY+64tb4hw+nIdqhzujPF/wBLxjpa+uZ1XOce5VdLgiSC4+y1OEnuh06vxYX8nRLuhef0XLqcbxesOXnLBOGDlpdmlwjAcTauzSaJoR5K2T0tryGcJqPMnfqrs4GpcV6XK2gMx0RIuvyTedTkbw2m2azur6PDu1tQM028xJ2XVwnCanEOhoAAyYwvd4fh9PhdKNNsuOT1XLlzsTn4HgGcKOZ3i1oEnp5Locxzosx0THUcCaQ+YfRctu6SjTjuZhHkEojUr7pgRPkraqUAQFqBk7J662tyg5jKpoUNDHRK5H5jJhzmj7qRe2T42+qct+IXERiu6WowgXtP42eqQ6jYjmHqnL+IxMlaUvM2CedvqtztMeNsYynL+LRmyb7LbpC5sW5vqiXN/ub6oz+JgmBBCmXNs87f/pYvbH1iPNVl/FP04ixcITHklOoLHM2fNKXs/uB+6sueiZ0KbiINe6Lnt5acPVIXtg21WX8GscXhKYgoFzYyI80hc2COYYTl/EJPVKSLnZBzhnmGUvM3lI5h6+yfG/i0Djt2QJuUHPbdi7SFwL2t3xlay/Ih4vWGlwuo49F84/jOI1By85ABqAur4txJ4jWGjo/8baz9XdbR0GhjRFnK6cZk9DXI3h9TUILiT52r6fAiPFOF3aTAI7KkbbZVbynxObT4TSafpsFWbpsa3FbUqtaI22TENqT7rPd+LpMMaAaThoDjSYBsCx0/numDmitzaO/xaBbR6I8ovpOyBcIInfql5xZnfqqS/i2KEN6Lr4Lg3a553gt08+fkq8FwDa1OJcyQAQzm/Neo5zAKeyMU7ZY5cr8hnZGNZpafKxkAdEQ4HaoQL2E/W3/6SyyKez1XPL+E5AjHZYsbGLKn8xo/E3bdOzWZ/e3/AOlWX8QFgipSlnYqvPpkD/cZ1+oYTfMZs9mequ/xJBtCuyZ5bpsLngADKL9TTazmc9kC/qkrweJ4nU+KcWNDRdy6IPiM5jotcZeXwZnt/9k=\"}");

cell.print(data);
  cell.write(26); // ctrl+z character: send the packet

  waitFor("OK");
  
  
  // now we're going to keep checking the socket status forever
  // break when the server tells us it acknowledged data
  while (1) {
    cell.println("AT+SDATASTATUS=1"); // we'll get back SOCKSTATUS and then OK
    String s = getMessage(); // we want s to contain the SOCKSTATUS message
    if (s == "+STCPD:1") // this means server sent data. cool, but we want to see SOCKSTATUS, so let's get next message
      s = getMessage();
    if (s == "+STCPC:1") // this means socket closed. cool, but we want to see SOCKSTATUS, so let's get next message
      s = getMessage();
    waitFor("OK");
    
    if (!s.startsWith("+SOCKSTATUS")) {
      Serial.println("Wait, this isn't the SOCKSTATUS message!");
      cellOutputForever(); // something went wrong
    }
    if (checkSocketString(s) == packetLength) // checks that packetLength bytes have been acknowledged by server
      break; // we're good!
    else {
      Serial.println("Sent data not yet acknowledged by server, waiting 1 second and checking again.");
      delay(1000);
    }
  }
  Serial.println("Yes! Sent data acknowledged by server!");

  // we could skip the checking of SOCKSTATUS in the above while-loop
  // instead we could just wait for one or both of these:
  //waitTil("+STCPD:1"); // this means data is received
  //waitTil("+STCPC:1"); // this means socket is closed
  
  // TODO: actually check if we received data, don't just do this blindly
  Serial.println("Reading data from server...");
  cell.println("AT+SDATAREAD=1"); // how we read data server has sent
  // WARNING: this might not work - software serial can be too slow for receiving data
    
  //cellHexForever(); // useful for debugging
  cellOutputForever(); // just keep printing whatever GSM module is telling us
}

/* NOTES
 * 
 * what is +STIN:1 ?
 * 
 * to disconnect after transmission: AT+CGACT=0,1 breaks socket. AT+CGATT=0 seems to work more authoritatively?
 * AT+SDATASTART=1,0 // close TCP connection
 * AT+SDATASTATUS=1 // clear sent/ack bytes from SOCKSTATUS
 * 
*/

// ====== HELPER FUNCTIONS ====== //

// keep reading the serial messages we receive from the module
// loop forever until we get a nonzero string ending in \r\n - print and return that.
// TODO: implement a timeout that returns 0?
String getMessage() {
  String s="";
  while(1) {
    if(cell.available()>0) {
      s = s+(char)cell.read();
      if (s.length()>1 && s[s.length()-2]=='\r' && s[s.length()-1]=='\n') { // if last 2 chars are \r\n
        if (s==" \r\n" || s=="\r\n") { // skip these, move on
          s="";
        }
        else { // we have a message!
          Serial.println(s.substring(0,s.length()-2));
          return s.substring(0,s.length()-2);
        }
      }
    }
  }
}

// for eating a single message we expect from the module
// prints out the next message from the module. if it's not the expected value, die
void waitFor(String s) {
  String message=getMessage();
  if (message != s) {
    Serial.println("Wait, that's not what we were expecting. We wanted \""+s+"\" and we get "+message);
    cellOutputForever();
  }
  delay(100); // wait for a tiny bit before sending the next command
}

// keep spitting out messages from the module til we get the one we expect
void waitTil(String s) {
  String message;
  while (1) {
    message = getMessage();
    if (message == s){
      delay(100); // cause we're probably about to send another command
      return;
    }
  }
}

// keep reading characters until we get char c
void waitFor(char c) {
  while(1) {
    if(cell.available()>0) {
      if ((char)cell.read() == c) {
        delay(100);
        return;
      }
    }
  }
}

// if something goes wrong, abort and just display cell module output so we can see error messages
// this will loop forever
void cellOutputForever() {
  Serial.println("Looping forever displaying cell module output...");
  while(1) {
    if(cell.available()>0) {
      Serial.print((char)cell.read());
    }
  }
}

// like above, but in hex, useful for debugging
void cellHexForever() {
  while(1) {
    if(cell.available()>0) {
      char c = (char)cell.read();
//      Serial.print("a char: ");
      Serial.print(c, HEX);
      Serial.print(" ");
      Serial.println(c);
    }
  }
}


// receive string such as "SOCKSTATUS: 1,1,0102,10,10,0"
// 0 is connection id. 1 is whether connected or not. 2 is status (0104 is connecting, 0102 is connected, others)
// 3 is sent bytes. 4 is acknowledged bytes. 5 is "received data counter"
// THIS FUNCTION WILL check that sent bytes == ack bytes, and return that value
// return 0 if they don't match or if amount of data is 0
int checkSocketString(String s) {
  if (socketStringSlice(3,s) == 0)
    return 0;
  else if (socketStringSlice(3,s) == socketStringSlice(4,s))
    return socketStringSlice(3,s);
  else
    return 0;
}

// returns the index of the nth instance of char c in String s
int nthIndexOf(int n, char c, String s) {
  int index=0;
  for (int i=0; i<=n; i++) {
    index = s.indexOf(c,index+1);
  }
  return index;
}

// expects string such as "SOCKSTATUS: 1,1,0102,10,10,0"
// returns nth chunk of data, delimited by commas
int socketStringSlice(int n, String s) {
  String slice = s.substring(nthIndexOf(n-1,',',s)+1,nthIndexOf(n,',',s));
  char cArray[slice.length()+1];
  slice.toCharArray(cArray, sizeof(cArray));
  return atoi(cArray);
}

String returnImage(){
return "1234";
}
