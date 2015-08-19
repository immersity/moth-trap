char incomingByte = 0;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.println("Hello World");
}

void loop() {
  // put your main code here, to run repeatedly:
    if (Serial.available() > 0) 
    {
      // read the incoming byte:
      incomingByte = Serial.read();

      // say what you got:
      Serial.print("I received: " + incomingByte);
      Serial.println(incomingByte);
    }
}
