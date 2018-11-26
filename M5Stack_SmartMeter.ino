#include <M5Stack.h>
#include "BP35A1.h"

BP35A1 *bp35a1;

void setup()
{
  M5.begin();

  delay(1000);
  bp35a1 = new BP35A1();
  
  // text print
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);

  M5.Lcd.setCursor(10, 10); M5.Lcd.print("BP35A1");
  Serial.begin(115200);
  Serial2.begin(115200);
  delay(100);

  bp35a1->testComm();
}

void resetWI_SUN() {
  digitalWrite(5, LOW); delay(5); digitalWrite(5, HIGH);
  Serial.setTimeout(300);
  String str;
  while(1) {
    str = Serial2.readStringUntil('\0');
    if(str.length() == 0) {
      Serial2.println("SKVER");
    }
    delay(10);
  }

  return;
}

void loop()
{


  
}

