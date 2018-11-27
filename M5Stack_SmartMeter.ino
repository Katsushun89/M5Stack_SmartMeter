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
  bp35a1->initComm();
  bp35a1->activeScan();
  
}

void loop()
{
  uint32_t power;
#if 1
  if(bp35a1->getInstantaneousPower(&power) == true){
    Serial.println("getInstantaneousPower success");
  }
  delay(30 * 1000);
#endif  
}

