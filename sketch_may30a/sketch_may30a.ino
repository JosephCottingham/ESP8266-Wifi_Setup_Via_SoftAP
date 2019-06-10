#include <SoftwareSerial.h>

SoftwareSerial ESPserial(5, 14); // RX | TX
SoftwareSerial PICserial(16, 4);
byte data[16];
void setup() {
  Serial.begin(115200);
  ESPserial.begin(9600);
  delay(10000); 
}

void loop() {
  delay(10000);
  PICserial.write(65);
  Serial.println("\nWrite");
  byte bytes_read = 0;
    while(ESPserial.available() > 0){
      data[bytes_read] = ESPserial.read();
      bytes_read++;
    }


  for(int asdf = 0; asdf < 16; asdf++){
      Serial.print(data[asdf]);
      Serial.print("~");
  }
  for(int asdfk = 0; asdfk < 16; asdfk++){
      data[asdfk] = 0;
  }
    Serial.println("\n");
}
