#include <SoftwareSerial.h>

SoftwareSerial ESPserial(5, 4); // RX | TX
byte data[16];
void setup() {
  Serial.begin(115200);
  ESPserial.begin(9600);
}

void loop() {
  delay(1000);
  ESPserial.write(65);
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
    Serial.println("\n");
}
