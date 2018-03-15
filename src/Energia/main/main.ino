#include <Energia.h>

char c;
char on = 0;

void setup() {
  Serial.begin(9600);
  delay(100);
  Serial2.begin(9600);
  delay(100);
}

void loop() {
  while (Serial2.available()) {
    c = Serial2.read();
    Serial.print(c);
    //Serial.print(c, DEC);
    //Serial.print(" ");
    on = 1;
  }
  if (on) {
    on = 0;
    Serial.println();
    Serial.println("##################################");
    Serial.println();
  }
}
